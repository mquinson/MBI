/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file CollectiveMatch.cpp
 *       @see MUST::CollectiveMatch.
 *
 *  @date 30.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "CollectiveMatch.h"

#include <assert.h>
#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(CollectiveMatch)
mFREE_INSTANCE_FUNCTION(CollectiveMatch)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CollectiveMatch)

//=============================
// Constructor
//=============================
CollectiveParticipantInfo::CollectiveParticipantInfo (void)
: currentMatchId(0), /*IMPORTANT: this must be smaller then currentMatchId of CollectiveMatchInfo!*/
  isComplete (false),
  sendTransfer (NULL),
  recvTransfer (NULL),
  queuedTransfers ()
{
    //Nothing to do
}

//=============================
// Constructor
//=============================
CollectiveMatchInfo::CollectiveMatchInfo (I_Comm* comm, int worldSize)
: commSize (0),
  numParticipants (0),
  numCompletedParticipants (0),
  currentMatchId (1),
  collId (MUST_COLL_UNKNOWN),
  rootRank (0),
  firstParticipant (0),
  participants (),
  partIndices ()
{
    //For intra comms its the group size, for intercomms the sum of the group sizes
    commSize = comm->getGroup()->getSize();
    if (comm->isIntercomm()) commSize+=comm->getRemoteGroup()->getSize();

    //We must reserve size of MPI_COMM_WORLD here!
    //=>As we use world ranks as indices
    participants.resize(worldSize);
    partIndices.resize(worldSize);
}

//=============================
// Constructor
//=============================
CollectiveMatch::CollectiveMatch (const char* instanceName)
: gti::ModuleBase<CollectiveMatch, I_CollectiveMatch> (instanceName),
  myMatching (),
  myCheckpointMatching (),
  myActive (true),
  myCheckpointActive (true),
  myListeners ()
#ifdef MUST_DEBUG
  ,myHistory (),
  myCheckpointHistory ()
#endif
  {
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 7
    if (subModInstances.size() < NUM_MODS_REQUIRED)
    {
        std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
        assert (0);
    }
    if (subModInstances.size() > NUM_MODS_REQUIRED)
    {
        for (std::vector<I_Module*>::size_type i = NUM_MODS_REQUIRED; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myConsts = (I_BaseConstants*) subModInstances[1];
    myLogger = (I_CreateMessage*) subModInstances[2];
    myCTrack = (I_CommTrack*) subModInstances[3];
    myDTrack = (I_DatatypeTrack*) subModInstances[4];
    myOTrack = (I_OpTrack*) subModInstances[5];
    myOrder = (I_OperationReordering*) subModInstances[6];

    //Initialize module data
    /*nothing to do*/
  }

//=============================
// Destructor
//=============================
CollectiveMatch::~CollectiveMatch ()
{
    //==Free other data
    if (clearMatching (&myMatching))
    {
#ifdef MUST_DEBUG
        std::cout << "CollectiveMatch had collective ops in its matching structure during its destruction!" << std::endl;
#endif
    }
    clearMatching (&myCheckpointMatching);

#ifdef MUST_DEBUG
    std::map<I_CommPersistent*, std::list<MustCollCommType> >::iterator histIter;
    for (histIter = myHistory.begin(); histIter != myHistory.end(); histIter++)
    {
        I_CommPersistent *comm = histIter->first;
        std::cout << "Match history for comm (first match first): ";
        std::stringstream stream;
        std::list<std::pair<MustParallelId, MustLocationId> > refs;
        comm->printInfo (stream, &refs);
        std::cout << stream.str() << std::endl;

        std::list<MustCollCommType>::iterator lIter;
        for (lIter = histIter->second.begin(); lIter != histIter->second.end(); lIter++)
        {
            std::cout << collIdToString (*lIter) << std::endl;
        }
        std::cout << std::endl;
    }

    clearHistory (&myHistory);
    clearHistory (&myCheckpointHistory);
#endif

    //Free sub modules
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myConsts)
        destroySubModuleInstance ((I_Module*) myConsts);
    myConsts = NULL;

    if (myLogger)
        destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myCTrack)
        destroySubModuleInstance ((I_Module*) myCTrack);
    myCTrack = NULL;

    if (myDTrack)
        destroySubModuleInstance ((I_Module*) myDTrack);
    myDTrack = NULL;

    if (myOTrack)
        destroySubModuleInstance ((I_Module*) myOTrack);
    myOTrack = NULL;

    if (myOrder)
        destroySubModuleInstance ((I_Module*) myOrder);
    myOrder = NULL;
}

//=============================
// CollNoTransfer
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollNoTransfer (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        MustCommType comm,
        int numTasks, // counter for event aggregation
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
        return GTI_ANALYSIS_SUCCESS;

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollSend
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollSend (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks, // counter for event aggregation
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return GTI_ANALYSIS_SUCCESS;
        }
    }

	//Check whether given root is reasonable
	if (dest < 0)
	{
	     commInfo->erase();
	     typeInfo->erase();
	     if (opInfo)
	     	opInfo->erase();
	     return GTI_ANALYSIS_SUCCESS;
	}
	
    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            true,
            count,
            typeInfo,
            opInfo,
            dest);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollSendN
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollSendN (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            true,
            count,
            typeInfo,
            opInfo);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollSendCounts
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollSendCounts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            true,
            ownCounts,
            typeInfo,
            opInfo);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollSendTypes
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollSendTypes (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent** typeInfos = new I_DatatypePersistent* [commsize];
    for (int i = 0; i < commsize; i++)
    {
        if (!getTypeInfo(pId, types[i], &(typeInfos[i])))
        {
            for (int j = 0; j < i; j++)
                typeInfos[j]->erase();
            commInfo->erase();
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            true,
            ownCounts,
            typeInfos,
            NULL);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollRecv
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollRecv (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int src, /*Root process to receive from as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }
    
    	//Check whether given root is reasonable
	if (src < 0)
	{
	     commInfo->erase();
	     typeInfo->erase();
	     return GTI_ANALYSIS_SUCCESS;
	}

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            false,
            count,
            typeInfo,
            NULL,
            src);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollRecvN
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollRecvN (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            false,
            count,
            typeInfo,
            opInfo);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollRecvCounts
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollRecvCounts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            false,
            ownCounts,
            typeInfo,
            NULL);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollRecvTypes
//=============================
GTI_ANALYSIS_RETURN CollectiveMatch::CollRecvTypes (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest
)
{
    //== 0) Report and disable for nonblocking operations
    if (hasRequest) reportNonblockingCollectiveUnsopported (pId, lId);

    if (!myActive)
        return GTI_ANALYSIS_SUCCESS;

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return GTI_ANALYSIS_SUCCESS;
    }

    I_DatatypePersistent** typeInfos = new I_DatatypePersistent* [commsize];
    for (int i = 0; i < commsize; i++)
    {
        if (!getTypeInfo(pId, types[i], &(typeInfos[i])))
        {
            for (int j = 0; j < i; j++)
                typeInfos[j]->erase();
            commInfo->erase();
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    //Create op
    CollectiveOp* newOp = new CollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            false,
            ownCounts,
            typeInfos,
            NULL);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getCommInfo
//=============================
bool CollectiveMatch::getCommInfo (
                MustParallelId pId,
                MustCommType comm,
                I_CommPersistent **outComm)
{
    I_CommPersistent *commInfo = myCTrack->getPersistentComm(pId, comm);

    //Handle unknown
    if (!commInfo) return false;

    //Handle MPI_COMM_NULL
    if (commInfo->isNull())
    {
        commInfo->erase();
        return false;
    }

    if (outComm) *outComm = commInfo;

    return true;
}

//=============================
// getTypeInfo
//=============================
bool CollectiveMatch::getTypeInfo (
        MustParallelId pId,
        MustDatatypeType type,
        I_DatatypePersistent **outType)
{
    I_DatatypePersistent *typeInfo = myDTrack->getPersistentDatatype (pId, type);

    if (!typeInfo) return false;

    if (typeInfo->isNull())
    {
        typeInfo->erase();
        return false;
    }

    if (outType) *outType = typeInfo;
    return true;
}

//=============================
// getOpInfo
//=============================
bool CollectiveMatch::getOpInfo (
        MustParallelId pId,
        MustOpType op,
        I_OpPersistent** outOp)
{
    I_OpPersistent *opInfo = myOTrack->getPersistentOp(pId, op);

    if (!opInfo) return false;

    if (opInfo->isNull())
    {
        opInfo->erase();
        return false;
    }

    if (outOp) *outOp = opInfo;
    return true;
}

//=============================
// handleNewOp
//=============================
bool CollectiveMatch::handleNewOp (int rank, CollectiveOp* newOp)
{
    if (myOrder->isRankOpen(rank))
    {
        //Process
        PROCESSING_RETURN ret = newOp->process(rank);

        //reprocessing should never be necessary ....
        if (ret == PROCESSING_REEXECUTE)
        {
            std::cerr << "Internal error in CollectiveMatch, a operation returned PROCESSING_REEXECUTE, which should not happen!" << std::endl;
            assert (0);
        }
    }
    else
    {
        //Queue
        myOrder->enqueueOp(rank, newOp);
    }

    return true;
}

//=============================
// addCollectiveTransfer
//=============================
bool CollectiveMatch::addCollectiveTransfer (CollectiveOp* op)
{
    CollectiveMatchInfo *info;
    I_Comm *comm;

    //Is it a known comm ?
    MatchStructure::iterator commIter;
    for (commIter = myMatching.begin(); commIter != myMatching.end(); commIter++)
    {
        if (!commIter->first)
            continue;

        //Works for both intra and inter comms
        if (!commIter->first->compareComms(op->getComm()))
            continue;

        info = &(commIter->second);
        comm = commIter->first;
        break;
    }

    //Add the new comm if necessary
    if (commIter == myMatching.end())
    {
        std::pair <MatchStructure::iterator, bool> ret =
                myMatching.insert(
                        std::make_pair (
                                op->getCommCopy(),
                                CollectiveMatchInfo(op->getComm(), myCTrack->getComm(op->getPId(), myCTrack->getWorldHandle())->getGroup()->getSize())
                                        ));
        comm = op->getComm();
        info = &(ret.first->second);
    }

    return processTransferForInfo (info, op);
}

//=============================
// processTransferForInfo
//=============================
bool CollectiveMatch::processTransferForInfo (CollectiveMatchInfo* info, CollectiveOp* op)
{
    //===Process for given CollectiveMatchInfo
    //1) Does the collective Id fit ?
    if (info->numParticipants == 0)
    {
        //==We start the match here

        //A) Prepare the match info
        info->collId = op->getCollId();
        info->currentMatchId = info->currentMatchId + 1;
        info->commSize = op->getCommSize();
        info->firstParticipant = op->getIssuerRank();

        info->numParticipants = 0;
        info->numCompletedParticipants = 0;

        if (op->hasRoot())
            info->rootRank = op->getRoot();
        else
            info->rootRank = -1;

        //B) Prepare the first participants info
        CollectiveParticipantInfo *part = &(info->participants[op->getIssuerRank()]);
        return applyOpToParticipant (info, part, op);
    }
    else
    {
        CollectiveParticipantInfo *part = &(info->participants[op->getIssuerRank()]);
        if (part->currentMatchId < info->currentMatchId || !part->isComplete)
        {
            //Does the collective id matches ?
            if (info->collId != op->getCollId ())
            {
                //Get first participant to retrieve mismatch partner
                CollectiveParticipantInfo *part = &(info->participants[info->firstParticipant]);

                CollectiveOp* first;
                if (part->recvTransfer)
                    first = part->recvTransfer;
                if (part->sendTransfer)
                    first = part->sendTransfer;

                if (!first) return false; /*This should never happen in any case!*/

                first->printCollectiveMismatch (op);
                myActive = false; //Deactivate!
                return true;
            }

            return applyOpToParticipant (info, part, op);
        }
        else
        {
            //Queue, this is from a future match
            part->queuedTransfers.push_back(op);
            return true;
        }
    }

    return true;
}

//=============================
// applyOpToParticipant
//=============================
bool CollectiveMatch::applyOpToParticipant (
		CollectiveMatchInfo* info,
		CollectiveParticipantInfo *part,
		CollectiveOp* op)
{
    //==> Prepare the participant (if not already done)
    if (part->currentMatchId < info->currentMatchId)
    {
        part->sendTransfer = NULL;
        part->recvTransfer = NULL;
        part->currentMatchId = info->currentMatchId;
    }

    //==> Store the op in the participant
    if (op->isSendTransfer() || op->isNoTransfer ())
        part->sendTransfer = op;
    else
        part->recvTransfer = op;

    part->isComplete = !op->requiresSecondOp();

    //==> Validate the op
    //A) Op must match
    if (op->hasOp() && info->numParticipants > 0)
    {
        CollectiveOp *other = info->participants[info->firstParticipant].recvTransfer;
        if (!other)
            other = info->participants[info->firstParticipant].sendTransfer;
        assert (other);
        if (!other) return false;

        I_Op *a = op->getOp(),
                *b = other->getOp();

        if (*a != *b)
        {
        	op->printOpMismatch(other);
        	myActive = false;
        	return false;
        }
    }

    //B) Root must match
    //@todo What about intercomms here, do they have two roots???
    if (op->hasRoot())
    {
        if (info->rootRank == -1)
        {
            info->rootRank = op->getRoot();
            info->firstParticipant = op->getIssuerRank(); /*Update first participant here, we need to have the task providing the first root here!*/
        }
        else if (info->rootRank != op->getRoot())
        {
            CollectiveOp *other = info->participants[info->firstParticipant].recvTransfer;
            if (!other || !other->hasRoot())
                other = info->participants[info->firstParticipant].sendTransfer;

            assert (other); //Internal error, should not happen
            if (!other) return false;

            op->printRootMismatch(other);
            myActive = false;
            return false;
        }
    }

    //==> Store that we have one more participant in the info
    /*
     * We do this before type mismatching in order to also validate send/recvs from
     * a rank to itself.
     */
    info->numParticipants = info->numParticipants + 1;

    if (part->isComplete)
    {
        info->partIndices[info->numCompletedParticipants] = op->getIssuerRank();
        info->numCompletedParticipants = info->numCompletedParticipants + 1;
    }

    //C) Types must match
    //@todo What about intercomms here, the matching rules are probably different for them???
    if (part->isComplete) //Only check if we both got the send and the receive part of this collective
    {
        CollectiveOp *transfers[2] = {part->sendTransfer, part->recvTransfer};
        CollectiveOp *send, *receive;

        for (int i = 0; i < 2; i++)
        {
            CollectiveOp *transfer = transfers[i];
            if (!transfer || transfer->isNoTransfer())
                continue;

            if(transfer->isSendTransfer())
                send = transfer;
            else
                receive = transfer;

            if (transfer->isToOne())
            {
                //to-1 transfer, check whether information from the root arrived
                //(If not the root will check against this once its complete information arrives)
                if (info->participants[info->rootRank].isComplete)
                {
                    if (transfer->isSendTransfer())
                        receive = info->participants[info->rootRank].recvTransfer;
                    else
                        send = info->participants[info->rootRank].sendTransfer;

                    if (!send || !receive) //This may happen for MPI_Bcast, the root does not sends to itself there
                        continue;

                    send->validateTypeMatch(receive); //We do not switch of for type mismatches, if the app doen't crashs we should be able to continue
                }
            }
            else
            {
                //to-N transfer
                if (!transfer->hasRoot() && !transfer->collectiveUsesMultipleCounts())
                {
                    /*
                     * A: If all transfers are to-N transfers with a single type sig, we can use type equality transitivity
                     * I.e. we only compare to one type that was already compared instead of to all types
                     */
                    if (info->numCompletedParticipants > 1) //This is operation itself is a completed participant, wo we need one other guy at least ...
                    {
                        int otherRank = info->partIndices[info->numCompletedParticipants-2];

                        assert (info->participants[otherRank].isComplete); //This is the criterion for ranks to be in partIndices!
                        if (!info->participants[otherRank].isComplete) break;

                        if (transfer->isSendTransfer())
                            receive = info->participants[otherRank].recvTransfer;
                        else
                            send = info->participants[otherRank].sendTransfer;

                        if (!send || !receive) //This may happen for MPI_Bcast, the root does not sends to itself there
                            continue;

                        send->validateTypeMatch(receive); //We do not switch of for type mismatches, if the app doen't crashs we should be able to continue
                    }
                }
                else
                {
                    /*
                     * B: If we are the send/receive-N needed for a rooted operation we have to compare to all
                     *    or if we use multiple types
                     */
                    for (int otherRankIndex = 0; otherRankIndex < info->numCompletedParticipants; otherRankIndex++)
                    {
                        int otherRank = info->partIndices[otherRankIndex];

                        assert (info->participants[otherRank].isComplete); //This is the criterion for ranks to be in partIndices!
                        if (!info->participants[otherRank].isComplete) break;

                        if (transfer->isSendTransfer())
                            receive = info->participants[otherRank].recvTransfer;
                        else
                            send = info->participants[otherRank].sendTransfer;

                        if (!send || !receive) //This may happen for MPI_Bcast, the root does not sends to itself there
                            continue;

                        send->validateTypeMatch(receive); //We do not switch of for type mismatches, if the app doen't crashs we should be able to continue
                    }
                }
            }
        }
    }

    //==> Did we complete this match ?
    if (info->numCompletedParticipants == info->commSize)
    {
#ifdef MUST_DEBUG
        CollectiveOp *temp = info->participants[info->firstParticipant].sendTransfer;
        if (!temp) temp = info->participants[info->firstParticipant].recvTransfer;

        std::map<I_CommPersistent*, std::list<MustCollCommType> >::iterator histIter;
        for (histIter = myHistory.begin(); histIter != myHistory.end(); histIter++)
        {
            I_CommPersistent *comm = histIter->first;

            if (comm->compareComms(temp->getComm()))
            {
                histIter->second.push_back (info->collId);
                break;
            }
        }
        if (histIter == myHistory.end())
        {
            std::list<MustCollCommType> list;
            list.push_back (info->collId);
            myHistory.insert (std::make_pair(temp->getCommCopy(), list));
        }
#endif

        int oldMatchId = info->currentMatchId;

        //Reset basic info state
        info->numCompletedParticipants = 0;
        info->numParticipants = 0;
        info->collId = MUST_COLL_UNKNOWN;

        //Create a copy of this ops Comm for notification
        MustCollCommType collId = op->getCollId();
        I_CommPersistent *commCopy = op->getCommCopy();

        //Loop over all ranks, reset their information, activate ops in their queues
        /** @todo This is somewhat inefficent for small comms (smaller than MPI_COMM_WORLD) as we loop over COMM_WORLD size here, one might optimize by creating a copy of partIndices here */
        for (std::vector<CollectiveParticipantInfo>::size_type i = 0; i < info->participants.size(); i++)
        {
            CollectiveParticipantInfo *curr = &(info->participants[i]);

            if (curr->currentMatchId != oldMatchId)
                continue;

            curr->isComplete = false;
            if (curr->recvTransfer) delete (curr->recvTransfer);
            curr->recvTransfer = NULL;
            if (curr->sendTransfer) delete (curr->sendTransfer);
            curr->sendTransfer = NULL;

            while (!curr->queuedTransfers.empty() && !curr->isComplete)
            {
                CollectiveOp *toProcessOp = curr->queuedTransfers.front();
                curr->queuedTransfers.pop_front();

                if (!addCollectiveTransfer (toProcessOp))
                    return false;
            }
        }/*For: reset rank*/

        //Notification of listeners
        std::list <I_CollMatchListener*>::iterator lIter;
        for (lIter = myListeners.begin(); lIter != myListeners.end(); lIter++)
        {
            I_CollMatchListener *listener = *lIter;
            listener->newMatch(collId, commCopy);
        }
        commCopy->erase();
    }

    return true;
}

//=============================
// collIdToString
//=============================
std::string CollectiveMatch::collIdToString (MustCollCommType id)
{
    switch (id)
    {
    case MUST_COLL_GATHER: return "MPI_Gather";
    case MUST_COLL_GATHERV: return "MPI_Gatherv";
    case MUST_COLL_REDUCE: return "MPI_Reduce";
    case MUST_COLL_BCAST: return "MPI_Bcast";
    case MUST_COLL_SCATTER: return "MPI_Scatter";
    case MUST_COLL_SCATTERV: return "MPI_Scatterv";
    case MUST_COLL_ALLGATHER: return "MPI_Allgather";
    case MUST_COLL_ALLGATHERV: return "MPI_Allgatherv";
    case MUST_COLL_ALLTOALL: return "MPI_Alltoall";
    case MUST_COLL_ALLTOALLV: return "MPI_Alltoallv";
    case MUST_COLL_ALLTOALLW: return "MPI_Alltoallw";
    case MUST_COLL_ALLREDUCE: return "MPI_Allreduce";
    case MUST_COLL_REDUCE_SCATTER: return "MPI_Reduce_scatter";
    case MUST_COLL_REDUCE_SCATTER_BLOCK: return "MPI_Reduce_scatter_block";
    case MUST_COLL_SCAN: return "MPI_Scan";
    case MUST_COLL_EXSCAN: return "MPI_Exscan";
    case MUST_COLL_BARRIER: return "MPI_Barrier";
    case MUST_COLL_CART_CREATE: return "MPI_Cart_create";
    case MUST_COLL_CART_SUB: return "MPI_Cart_sub";
    case MUST_COLL_COMM_CREATE: return "MPI_Comm_create";
    case MUST_COLL_COMM_DUP: return "MPI_Comm_dup";
    case MUST_COLL_COMM_FREE: return "MPI_Comm_free";
    case MUST_COLL_COMM_SPLIT: return "MPI_Comm_split";
    case MUST_COLL_FINALIZE: return "MPI_Finalize";
    case MUST_COLL_GRAPH_CREATE: return "MPI_Graph_create";
    case MUST_COLL_INTERCOMM_CREATE: return "MPI_Intercomm_create";
    case MUST_COLL_INTERCOMM_MERGE: return "MPI_Intercomm_merge";
    case MUST_COLL_UNKNOWN: return "UNKNOWN COLLECTIVE";
    }

    return "CHECK IMPLEMENTATION!";
}

//=============================
// registerListener
//=============================
GTI_RETURN CollectiveMatch::registerListener (I_CollMatchListener *listener)
{
    myListeners.push_back(listener);
    return GTI_SUCCESS;
}

//=============================
// checkpoint
//=============================
void CollectiveMatch::checkpoint (void)
{
    //==0) Module handles stay the same

    //==1) myMatching
    clearMatching (&myCheckpointMatching);

    MatchStructure::iterator iter;
    for (iter = myMatching.begin(); iter != myMatching.end(); iter++)
    {
        CollectiveMatchInfo collInfo (iter->first, myCTrack->getComm((int)0, myCTrack->getWorldHandle())->getGroup()->getSize());
        if (!iter->first) continue;

        //Copy data
        collInfo.commSize = iter->second.commSize;
        collInfo.numParticipants = iter->second.numParticipants;
        collInfo.numCompletedParticipants = iter->second.numCompletedParticipants;
        collInfo.currentMatchId = iter->second.currentMatchId;
        collInfo.collId = iter->second.collId;
        collInfo.rootRank = iter->second.rootRank;
        collInfo.firstParticipant = iter->second.firstParticipant;

        if (iter->second.participants.size())
            collInfo.participants.resize(iter->second.participants.size());

        for (std::vector<CollectiveParticipantInfo>::size_type i = 0; i < iter->second.participants.size(); i++)
        {
            CollectiveParticipantInfo *other = &(iter->second.participants[i]);

            collInfo.participants[i].currentMatchId = other->currentMatchId;
            collInfo.participants[i].isComplete = other->isComplete;
            collInfo.participants[i].sendTransfer = other->sendTransfer;
            if (collInfo.participants[i].sendTransfer) collInfo.participants[i].sendTransfer = collInfo.participants[i].sendTransfer->copy();
            collInfo.participants[i].recvTransfer = other->recvTransfer;
            if (collInfo.participants[i].recvTransfer) collInfo.participants[i].recvTransfer = collInfo.participants[i].recvTransfer->copy();

            std::list<CollectiveOp*>::iterator opIter;
            for (opIter = other->queuedTransfers.begin(); opIter != other->queuedTransfers.end(); opIter++)
            {
                CollectiveOp *add = *opIter;
                if (add) add = add->copy();

                collInfo.participants[i].queuedTransfers.push_back(add);
            }
        }

        collInfo.partIndices = iter->second.partIndices;

        //Add
        iter->first->copy();
        myCheckpointMatching.insert(std::make_pair(iter->first, collInfo));
    }

    //==2) myActive
    myCheckpointActive = myActive;

    //==3) myHistory (DEBUG)
#ifdef MUST_DEBUG
    clearHistory (&myCheckpointHistory);

    MatchHistory::iterator histIter;
    for (histIter = myHistory.begin(); histIter != myHistory.end(); histIter++)
    {
        if (!histIter->first) continue;
        histIter->first->copy();
        myCheckpointHistory.insert (std::make_pair (histIter->first, histIter->second));
    }
#endif
}

//=============================
// rollback
//=============================
void CollectiveMatch::rollback (void)
{
    //==0) Module handles stay the same

    //==1) myMatching
    clearMatching (&myMatching); //This must be the real delete and clear
    myMatching = myCheckpointMatching;
    myCheckpointMatching.clear(); //This must just be a clear

    //==2) myActive
    myActive = myCheckpointActive;

    //==3) myHistory (DEBUG)
#ifdef MUST_DEBUG
    clearHistory (&myHistory);  //This must be the real delete and clear
    myHistory = myCheckpointHistory;
    myCheckpointHistory.clear(); //This must just be a clear
#endif
}

//=============================
// clearMatching
//=============================
bool CollectiveMatch::clearMatching (MatchStructure *matching)
{
    MatchStructure::iterator commIter;
    bool hadOpenOp = false;
    for (commIter = matching->begin(); commIter != matching->end(); commIter++)
    {
        I_CommPersistent *comm = commIter->first;
        if (!comm)continue;

        for (std::vector<CollectiveParticipantInfo>::size_type i = 0; i < commIter->second.participants.size(); i++)
        {
            CollectiveParticipantInfo &part = commIter->second.participants[i];

            if (part.sendTransfer)
            {
                delete part.sendTransfer;
                hadOpenOp = true;
            }
            if (part.recvTransfer)
            {
                delete part.recvTransfer;
                hadOpenOp = true;
            }

            std::list<CollectiveOp*>::iterator qIter;
            for (qIter = part.queuedTransfers.begin(); qIter != part.queuedTransfers.end(); qIter++)
            {
                if (*qIter)
                {
                    delete *qIter;
                    hadOpenOp = true;
                }
            }
        }

        //Erase the comm
        comm->erase();
    }
    matching->clear();

    return hadOpenOp;
}

//=============================
// clearHistory
//=============================
#ifdef MUST_DEBUG
void CollectiveMatch::clearHistory (MatchHistory* history)
{
    std::map<I_CommPersistent*, std::list<MustCollCommType> >::iterator histIter;
    for (histIter = history->begin(); histIter != history->end(); histIter++)
    {
        I_CommPersistent *comm = histIter->first;
        comm->erase();
    }
    history->clear();
}
#endif

//=============================
// reportNonblockingCollectiveUnsopported
//=============================
bool CollectiveMatch::reportNonblockingCollectiveUnsopported (
        MustParallelId pId,
        MustLocationId lId)
{
    //Do not repeat any errors if we already shut down (for whatever reason)
    if (!myActive)
        return true;

    //Report the issue to the user
    std::stringstream stream;

    stream
        << "This configuration of MUST does not support nonblocking collective operations. "
        << "Please rerun with a distributed MUST configuration (\"mustrun --must:distributed\" "
        << "or \"mustrun --must:fanin X\"). This configuration will support nonblocking "
        << "collectives." << std::endl << std::endl
        << "MUST correctness analysis will largely halt now. Subsequent errors may result from this fact, e.g., deadlock errors.";

    myLogger->createMessage (
            MUST_ERROR_UNSUPPORTED,
            pId,
            lId,
            MustErrorMessage,
            stream.str());

    //Disable collective matching from here on
    myActive = false;
    return true;
}

/*EOF*/
