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
 * @file P2PMatch.cpp
 *       @see MUST::P2PMatch.
 *
 *  @date 26.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "P2PMatch.h"

#include <assert.h>
#include <sstream>
#include <fstream>

using namespace must;

mGET_INSTANCE_FUNCTION(P2PMatch)
mFREE_INSTANCE_FUNCTION(P2PMatch)
mPNMPI_REGISTRATIONPOINT_FUNCTION(P2PMatch)

//=============================
// Constructor
//=============================
P2PMatch::P2PMatch (const char* instanceName)
    : gti::ModuleBase<P2PMatch, I_P2PMatch> (instanceName),
      myQs (),
      myCheckpointQs (),
      mySuspendedForEntry (NULL),
      myCheckpointSuspendedForEntry (NULL),
      myDecidedIrecvs (),
      myDecidedRecvs (),
      myCheckpointDecidedIrecvs (),
      myCheckpointDecidedRecvs (),
      suspendedWcRecvs (),
      myCheckpointSuspendedWcRecvs (),
      myOpsToWatchForMatch (),
      myFinCompletion (NULL),
      myCheckpointFinCompletion (NULL),
      myListeners (),
      myPrintLostMessages (true),
      myCheckpointPrintLostMessages (true)
#ifdef MUST_DEBUG
      ,myMatches (),
      myCheckpointMatches ()
#endif
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 9
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
    myRTrack = (I_RequestTrack*) subModInstances[4];
    myDTrack = (I_DatatypeTrack*) subModInstances[5];
    myOrder = (I_OperationReordering*) subModInstances[6];
    myFloodControl = (I_FloodControl*) subModInstances[7];
    myLIdMod = (I_LocationAnalysis*) subModInstances[8];

    //Initialize module data
    /*nothing to do*/
}

//=============================
// Destructor
//=============================
P2PMatch::~P2PMatch ()
{
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

	if (myRTrack)
		destroySubModuleInstance ((I_Module*) myRTrack);
	myRTrack = NULL;

	if (myDTrack)
	    destroySubModuleInstance ((I_Module*) myDTrack);
	myDTrack = NULL;

	if (myOrder)
	    destroySubModuleInstance ((I_Module*) myOrder);
	myOrder = NULL;

	if (myFloodControl)
	    destroySubModuleInstance ((I_Module*) myFloodControl);
	myFloodControl = NULL;

	//==Free other data
	//Matching structure
	//We do the graceful free in printLostMessages, we should not destruct anything here as destruction hell is already going on.
	myQs.clear ();
	myCheckpointQs.clear();

	//Completion
	if (myFinCompletion) delete (myFinCompletion);
	if (myCheckpointFinCompletion) delete (myCheckpointFinCompletion);

	//Other checkpointing stuff
	myCheckpointSuspendedForEntry = NULL;
	myCheckpointSuspendedWcRecvs.clear();

	//Enforced matching decisions
	myDecidedIrecvs.clear();
	myDecidedRecvs.clear();
	myCheckpointDecidedIrecvs.clear();
	myCheckpointDecidedRecvs.clear();
}

//=============================
// send
//=============================
GTI_ANALYSIS_RETURN P2PMatch::send (
		MustParallelId pId,
		MustLocationId lId,
		int destIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
        int mode)
{
#ifdef MUST_MSG_MATCH_REDUCTION
    if (!canOpBeProcessed (pId, comm, destIn))
        return GTI_ANALYSIS_SUCCESS;
#endif

    //== 1) Prepare the operation for this send
    //MPI_PROC_NULL ops are no-ops!
	if (myConsts->isProcNull(destIn))
		return GTI_ANALYSIS_SUCCESS;

	//Get infos and translation
	int dest;
	I_CommPersistent* cInfo;
	I_DatatypePersistent *dInfo;
	if (!getCommTranslationAndType (pId, comm, destIn, type, &cInfo, &dest, &dInfo))
	    return GTI_ANALYSIS_SUCCESS;

	//Create op
	P2POp* newOp = new P2POp (
	        this,
	        true,
	        tag,
	        dest /*TRANSLATED dest*/,
	        cInfo,
	        dInfo,
	        count,
	        pId,
	        lId,
	        (MustSendMode) mode);

	//== 2) Process or Queue ?
	int rank = myPIdMod->getInfoForId(pId).rank;
	handleNewOp (rank, newOp);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isend
//=============================
GTI_ANALYSIS_RETURN P2PMatch::isend (
		MustParallelId pId,
		MustLocationId lId,
		int destIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
        int mode,
		MustRequestType request)
{
#ifdef MUST_MSG_MATCH_REDUCTION
    if (!canOpBeProcessed (pId, comm, destIn))
        return GTI_ANALYSIS_SUCCESS;
#endif

    //== 1) Prepare the operation for this send
    //MPI_PROC_NULL ops are no-ops!
    if (myConsts->isProcNull(destIn))
        return GTI_ANALYSIS_SUCCESS;

    //Get infos and translation
    int dest;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (pId, comm, destIn, type, &cInfo, &dest, &dInfo))
        return GTI_ANALYSIS_SUCCESS;

    //Create op
    P2POp* newOp = new P2POp (
            this,
            true,
            tag,
            dest /*TRANSLATED dest*/,
            request,
            cInfo,
            dInfo,
            count,
            pId,
            lId,
            (MustSendMode) mode);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// recv
//=============================
GTI_ANALYSIS_RETURN P2PMatch::recv (
		MustParallelId pId,
		MustLocationId lId,
		int sourceIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count)
{
#ifdef MUST_MSG_MATCH_REDUCTION
    if (!canOpBeProcessed (pId, comm, sourceIn))
        return GTI_ANALYSIS_SUCCESS;
#endif

    //== 1) Prepare the operation for this receive
    //MPI_PROC_NULL ops are no-ops!
    if (myConsts->isProcNull(sourceIn))
        return GTI_ANALYSIS_SUCCESS;

    //Get infos and translation
    int source;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (pId, comm, sourceIn, type, &cInfo, &source, &dInfo))
        return GTI_ANALYSIS_SUCCESS;

    //Create op
    P2POp* newOp = new P2POp (
            this,
            false,
            tag,
            source /*TRANSLATED source*/,
            cInfo,
            dInfo,
            count,
            pId,
            lId);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_ANALYSIS_RETURN P2PMatch::irecv (
		MustParallelId pId,
		MustLocationId lId,
		int sourceIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
		MustRequestType request)
{
#ifdef MUST_MSG_MATCH_REDUCTION
    if (!canOpBeProcessed (pId, comm, sourceIn))
        return GTI_ANALYSIS_SUCCESS;
#endif

    //== 1) Prepare the operation for this receive
    //MPI_PROC_NULL ops are no-ops!
    if (myConsts->isProcNull(sourceIn))
        return GTI_ANALYSIS_SUCCESS;

    //Get infos and translation
    int source;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (pId, comm, sourceIn, type, &cInfo, &source, &dInfo))
        return GTI_ANALYSIS_SUCCESS;

    //Create op
    P2POp* newOp = new P2POp (
            this,
            false,
            tag,
            source /*TRANSLATED source*/,
            request,
            cInfo,
            dInfo,
            count,
            pId,
            lId);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getCommTranslationAndType
//=============================
bool P2PMatch::getCommTranslationAndType (
        MustParallelId pId,
        MustCommType comm,
        int rankIn,
        MustDatatypeType type,
        I_CommPersistent**pOutComm,
        int *pOutTranslatedRank,
        I_DatatypePersistent**pOutType)
{
    //==1) Comm
    I_CommPersistent* cInfo = myCTrack->getPersistentComm(pId,comm);
    if (cInfo == NULL)
        return false;//Unknown comm
    if (cInfo->isNull())
    {
        cInfo->erase();
        return false;//MPI_COMM_NULL
    }

    if (pOutComm)
        *pOutComm = cInfo;

    //==2) Translation
    if (pOutTranslatedRank)
        *pOutTranslatedRank = translateDestSource (cInfo, rankIn);


    //==3) Datatype
    I_DatatypePersistent *dInfo = myDTrack->getPersistentDatatype (pId, type);
    if (!dInfo) return false; //Unknown datatype

    if (pOutType) *pOutType = dInfo;

    return true;
}

//=============================
// translateDestSource
//=============================
int P2PMatch::translateDestSource (I_Comm* comm, int destSource)
{
    int ret;
    if (destSource != myConsts->getAnySource())
    {
        if (!comm->isIntercomm())
            comm->getGroup()->translate(destSource, &ret);
        else
            comm->getRemoteGroup()->translate(destSource, &ret);
    }
    else
    {
        ret = destSource;
    }

    return ret;
}

//=============================
// invTranslateIssuingRank
//=============================
int P2PMatch::invTranslateIssuingRank (I_Comm* comm, int rank)
{
    int ret;
    if (rank != myConsts->getAnySource())
    {
        comm->getGroup()->containsWorldRank(rank, &ret);
    }
    else
    {
        ret = rank;
    }

    return ret;
}

//=============================
// invTranslateIssuingRank
//=============================
int P2PMatch::invTranslateDestSource (I_Comm* comm, int rank)
{
    int ret;
    if (rank != myConsts->getAnySource())
    {
        if (!comm->isIntercomm())
            comm->getGroup()->containsWorldRank(rank, &ret);
        else
            comm->getRemoteGroup()->containsWorldRank(rank, &ret);
    }
    else
    {
        ret = rank;
    }

    return ret;
}

//=============================
// handleNewOp
//=============================
void P2PMatch::handleNewOp (int rank, P2POp* op)
{
    if (myOrder->isRankOpen(rank))
    {
        //Process
        PROCESSING_RETURN ret = op->process(rank);

        //If processing caused a suspension and a re-processing we have to queue
        if (ret == PROCESSING_REEXECUTE)
        {
            myOrder->enqueueOp(rank, op);
            op->addToSuspendedWCOpQueue ();//add to queue of operations that are queued and have a wilcard if applicable to this op
        }
    }
    else
    {
        //Queue
        myOrder->enqueueOp(rank, op);
        op->addToSuspendedWCOpQueue ();//add to queue of operations that are queued and have a wilcard if applicable to this op
    }
}

//=============================
// recvUpdate
//=============================
GTI_ANALYSIS_RETURN P2PMatch::recvUpdate (
		MustParallelId pId,
		MustLocationId lId,
		int source)
{
	int from = myPIdMod->getInfoForId(pId).rank;

	//==Is this an update for a recv for which we enforced a matching decision
	std::map<int, int>::iterator pos = myDecidedRecvs.find (from);
	if (pos != myDecidedRecvs.end() && pos->second > 0)
	{
	    pos->second = pos->second - 1;
#ifdef MUST_MATCH_DEBUG
    std::cout << "P2PMatch: IGNORED (due to match enforcement) WcRecvUpdate newSource=" << source << " from rank " << myPIdMod->getInfoForId(pId).rank << std::endl;
#endif
        return GTI_ANALYSIS_SUCCESS;
	}

	//==Perform the update
	bool unsuspend = false;
	findRecvForUpdate (from, source, false, 0, &unsuspend);

#ifdef MUST_MATCH_DEBUG
	std::cout << "P2PMatch: WcRecvUpdate newSource=" << source << " from rank " << myPIdMod->getInfoForId(pId).rank << std::endl;
	printQs ();
#endif

	//Remove suspension if necessary
	if (unsuspend)
	    myOrder->removeSuspension();

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// irecvUpdate
//=============================
GTI_ANALYSIS_RETURN P2PMatch::irecvUpdate (
		MustParallelId pId,
		MustLocationId lId,
		int source,
		MustRequestType request)
{
	int from = myPIdMod->getInfoForId(pId).rank;

	//==Is this an update for an irecv for which we enforced a matching decision
	std::map<int, std::list<MustRequestType> >::iterator pos = myDecidedIrecvs.find (from);
	if (pos != myDecidedIrecvs.end())
	{
	    std::list<MustRequestType>::iterator iter;
	    for (iter = pos->second.begin(); iter != pos->second.end(); iter++)
	    {
	        if (*iter != request) continue;

	        pos->second.erase(iter);
#ifdef MUST_MATCH_DEBUG
	        std::cout << "LostMessage: IGNORED (due to enforced matching) WcIrecvUpdate newSource=" << source << " request=" << request << " from rank " << myPIdMod->getInfoForId(pId).rank << std::endl;
#endif
	        return GTI_ANALYSIS_SUCCESS;
	    }
	}

	//==Perform the update
	bool unsuspend = false;
	findRecvForUpdate (from, source, true, request, &unsuspend);

#ifdef MUST_MATCH_DEBUG
	std::cout << "LostMessage: WcIrecvUpdate newSource=" << source << " request=" << request << " from rank " << myPIdMod->getInfoForId(pId).rank << std::endl;
	printQs ();
#endif

	//Remove suspension if necessary
	if (unsuspend)
	    myOrder->removeSuspension();

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// startPersistent
//=============================
GTI_ANALYSIS_RETURN P2PMatch::startPersistent (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	I_Request* info = myRTrack->getRequest(pId, request);

	if (info == NULL || !info->isPersistent())
		return GTI_ANALYSIS_SUCCESS;

	/*We are mapped before RequestTrack, so we check here whether the request is still active!*/
	if (info->isActive())
	    return GTI_ANALYSIS_SUCCESS;

	int destSource;
	if (info->isSend())
	    destSource = info->getDest();
	else
	    destSource = info->getSource();

	//Don't do anything for ops with MPI_PROC_NULL
	if (destSource == myConsts->getProcNull())
	    return GTI_ANALYSIS_SUCCESS;

	I_CommPersistent* comm = info->getCommCopy();
	I_DatatypePersistent* type = info->getDatatypeCopy();

	int destSourceTranslate = translateDestSource (comm, destSource);

	P2POp* newOp = new P2POp (
            this,
            info->isSend(),
            info->getTag(),
            destSourceTranslate,
            request,
            comm,
            type,
            info->getCount(),
            pId,
            lId,
            info->getSendMode()
            );

	handleNewOp (newOp->getIssuerRank(), newOp);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// cancel
//=============================
GTI_ANALYSIS_RETURN P2PMatch::cancel (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	static bool warned = false;
	if (!warned)
		std::cerr << "P2PMatch: detected a cancel, not supported, outputs may be wrong!" << std::endl;
	warned = true;

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// printLostMessages
//=============================
GTI_ANALYSIS_RETURN P2PMatch::printLostMessages (
		I_ChannelId *thisChannel)
{
	//Is completely reduced without a channel id ? (Reduced on this place)
	if (thisChannel)
	{
		//Initialize completion tree
		if (!myFinCompletion)
			myFinCompletion = new CompletionTree (
					thisChannel->getNumUsedSubIds()-1, thisChannel->getSubIdNumChannels(thisChannel->getNumUsedSubIds()-1));

		myFinCompletion->addCompletion (thisChannel);
	}

	if (!thisChannel || myFinCompletion->isCompleted())
	{
		//Log the lost messages
		printLostMessages ();
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// printLostMessages
//=============================
GTI_ANALYSIS_RETURN P2PMatch::printLostMessages ()
{
	QT::iterator qIter;

	if (myPrintLostMessages)
	{
        for (qIter = myQs.begin(); qIter != myQs.end(); qIter++)
        {
            ProcessQueues::iterator cIter;
            for (cIter = qIter->second.begin(); cIter != qIter->second.end(); cIter++)
            {
                ProcessTable *t = &(cIter->second);
                RankOutstandings::iterator oIter;

                //==== SENDS
                for (oIter = t->sends.begin(); oIter != t->sends.end(); oIter++)
                {
                    std::list<P2POp*>::iterator lIter;
                    for (lIter = oIter->second.begin(); lIter != oIter->second.end(); lIter++)
                    {
                        if (*lIter) (*lIter)->logAsLost(qIter->first);
                    }
                }

                //==== RECVS
                for (oIter = t->recvs.begin(); oIter != t->recvs.end(); oIter++)
                {
                    std::list<P2POp*>::iterator lIter;
                    for (lIter = oIter->second.begin(); lIter != oIter->second.end(); lIter++)
                    {
                        if (*lIter) (*lIter)->logAsLost(qIter->first);
                    }
                }

                //==== WC-RECVS
                std::list<P2POp*>::iterator lIter;
                for (lIter = t->wcRecvs.begin(); lIter != t->wcRecvs.end(); lIter++)
                {
                    if (*lIter) (*lIter)->logAsLost(qIter->first);
                }
            }//for comms
        }//for processes
	}

#ifdef MUST_DEBUG
	//Print the match histories
	printMatchHistories();
	clearMatchHistory(&myMatches);
	clearMatchHistory(&myCheckpointMatches);
	std::cout << "P2PMatch: printed match histories for each comm in files named \"must_match_history_comm_%d.dot\" use DOT to visualize them." << std::endl;
#endif

	//Do a graceful free of all remaining data
	//==Free other data
	//Matching structure
	clearQ (&myQs);
	clearQ (&myCheckpointQs);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// findMatchingRecv
//=============================
bool P2PMatch::findMatchingRecv (
		/*int from,
		int dest,
		int tag,
		I_Comm* comm,*/
        P2POp* op,
		bool *pOutNeedsToSuspend)
{
	if (pOutNeedsToSuspend)
		*pOutNeedsToSuspend = false;

	//==1) Get the process that would need to call the receive
	QT::iterator q = myQs.find (op->getToRank());
	if (q == myQs.end()) return false;

	//==2) Get the table
	/*
	 * Comm handles may be different on each process,
	 * so we have to ask the comm tracker whether any
	 * of the comms present for the destination are equal
	 * to the given comm.
	 */
	ProcessQueues::iterator t;
	for (t = q->second.begin(); t != q->second.end(); t++)
	{
		if (op->getComm()->compareComms(t->first))
			break;
	}
	if (t == q->second.end()) return false;

	//==3) First search the recvs (they have priority over the wcrecvs)
	RankOutstandings::iterator r = t->second.recvs.find(op->getIssuerRank());

	if (r != t->second.recvs.end())
	{
		std::list<P2POp*>::iterator recv;
		for (recv = r->second.begin(); recv != r->second.end(); recv++)
		{
			if (op->matchTags(*recv))
			{
				//!!! HIT, remove the recv
#ifdef MUST_DEBUG
			    addMatchToHistory (op, *recv);
#endif
			    op->matchTypes (*recv);
			    P2POp *other = *recv;
			    r->second.erase(recv);
			    notifyMatch(op, other);
			    delete (other);
				return true;
			}
		}
	}

	//==4) Search the wcRecvs
	std::list<P2POp*>::iterator recv;
	for (recv = t->second.wcRecvs.begin(); recv != t->second.wcRecvs.end(); recv++)
	{
	    P2POp* other = *recv;
		if (other->getToRank() == op->getToRank() || other->getToRank() == myConsts->getAnySource())
		{
			if (op->matchTags(other))
			{
				if (other->getToRank() != myConsts->getAnySource())
				{
					//!!! HIT, remove the recv
#ifdef MUST_DEBUG
                addMatchToHistory (op, other);
#endif
				    op->matchTypes (*recv);
				    r->second.erase(recv);
				    notifyMatch(op, other);
				    delete (other);
					return true;
				}
				else
				{
					//We need to start suspension
					mySuspendedForEntry = other;
					mySuspendedForEntry->setFirstWorldRankWithValidMatch (op->getIssuerRank());

					if (pOutNeedsToSuspend)
						*pOutNeedsToSuspend = true;
					return false;
				}
			}
		}
	}

	return false;
}

//=============================
// addOutstandingSend
//=============================
void P2PMatch::addOutstandingSend (
		/*int from,
		int dest,
		int tag,
		I_Comm* comm,
		bool hasRequest,
		MustRequestType request,
		MustParallelId pId,
		MustLocationId lId*/
        P2POp* op)
{
    //==0) Report as bad op
    myFloodControl->markCurrentRecordBad();

	//==1) Get the process for the sending process
	QT::iterator q = myQs.find (op->getIssuerRank());
	if (q == myQs.end())
	{
		myQs.insert(std::make_pair(op->getIssuerRank(), ProcessQueues ()));
		q = myQs.find (op->getIssuerRank());
	}

	//==2) Get the table
	ProcessQueues::iterator t = q->second.find(op->getPersistentComm());
	if (t == q->second.end())
	{
		q->second.insert (std::make_pair (op->getCommCopy(), ProcessTable()));
		t = q->second.find(op->getPersistentComm());
	}

	//==3) Get position in sends map
	RankOutstandings::iterator r = t->second.sends.find(op->getToRank());
	if (r == t->second.sends.end())
	{
		t->second.sends.insert (std::make_pair (op->getToRank(), std::list<P2POp*>()));
		r = t->second.sends.find(op->getToRank());
	}

	//==4) Add to queue for the position in sends map
	r->second.push_back(op);
}

//=============================
// findMatchingSend
//=============================
bool P2PMatch::findMatchingSend (
		/*int from,
		int source,
		int tag,
		I_Comm* comm,*/
        P2POp* op,
		bool *pOutNeedsToSuspend)
{
	if (pOutNeedsToSuspend)
		*pOutNeedsToSuspend = false;

	//==1) Get the process(es) that would need to call the send
	bool iterate = false;
	QT::iterator q;

	if (op->getToRank() != myConsts->getAnySource())
	{
		q = myQs.find (op->getToRank());
	}
	else
	{
		q = myQs.begin();
		iterate = true;
	}

	for (;q != myQs.end(); q++)
	{
		//==2) Get the table
		/*
		 * Comm handles may be different on each process,
		 * so we have to ask the comm tracker whether any
		 * of the comms present for the destination is equal
		 * to the given comm.
		 */
		ProcessQueues::iterator t;
		for (t = q->second.begin(); t != q->second.end(); t++)
		{
			if (op->getComm()->compareComms(t->first))
				break;
		}
		if (t == q->second.end())
		{
		    if (iterate)
		        continue;
		    else
		        break;
		}

		//==3) Search the sends
		RankOutstandings::iterator s = t->second.sends.find(op->getIssuerRank());

		if (s != t->second.sends.end())
		{
			std::list<P2POp*>::iterator send;
			for (send = s->second.begin(); send != s->second.end(); send++)
			{
			    P2POp* other = *send;
				if (op->matchTags(other))
				{
					if (op->getToRank() != myConsts->getAnySource())
					{
						//!!! HIT, remove the send
#ifdef MUST_DEBUG
					    addMatchToHistory (other, op);
#endif
					    op->matchTypes (other);
					    s->second.erase(send);
					    notifyMatch(other, op);
                        delete (other);

						return true;
					}
					else
					{
		                op->setFirstWorldRankWithValidMatch (other->getIssuerRank());

						//!!! Potential hit, we go into suspension now
						if (pOutNeedsToSuspend)
							*pOutNeedsToSuspend = true;
						return false;
					}
				}
			}
		}

		//Force abortion of this loop if we don't need to look into all processes
		if (!iterate)
			break;
	}//For processes to look into

	return false;
}

//=============================
// addOutstandingRecv
//=============================
void P2PMatch::addOutstandingRecv (
		/*int from,
		int source,
		int tag,
		I_Comm* comm,
		bool hasRequest,
		MustRequestType request,
		MustParallelId pId,
		MustLocationId lId*/
        P2POp* op)
{
    //==0) Report as bad op
    myFloodControl->markCurrentRecordBad();

	//==1) Get the process for the receiving process
	QT::iterator q = myQs.find (op->getIssuerRank());
	if (q == myQs.end())
	{
		myQs.insert(std::make_pair(op->getIssuerRank(), ProcessQueues ()));
		q = myQs.find (op->getIssuerRank());
	}

	//==2) Get the table
	ProcessQueues::iterator t = q->second.find(op->getPersistentComm());
	if (t == q->second.end())
	{
		q->second.insert (std::make_pair (op->getCommCopy(), ProcessTable()));
		t = q->second.find(op->getPersistentComm());
	}

	//==3) Get position in recvs or wcRecvs map
	std::list<P2POp*> *list;

	if (op->getToRank() != myConsts->getAnySource() && t->second.wcRecvs.empty())
	{
		RankOutstandings::iterator r;
		r = t->second.recvs.find(op->getToRank());
		if (r == t->second.recvs.end())
		{
			t->second.recvs.insert (std::make_pair (op->getToRank(), std::list<P2POp*>()));
			r = t->second.recvs.find(op->getToRank());
		}

		list = &(r->second);
	}
	else
	{
		list = &(t->second.wcRecvs);
	}

	//==4) Add to the list
	list->push_back(op);
}

//=============================
// findRecvForUpdate
//=============================
void P2PMatch::findRecvForUpdate (
		int from,
		int newSource,
		bool hasRequest,
		MustRequestType request,
		bool *pOutUnsuspend)
{
    if (pOutUnsuspend) *pOutUnsuspend = false;

	//==1) Search in the tables
	QT::iterator qIter = myQs.find(from);
	if (qIter != myQs.end())
	{
		ProcessQueues::iterator cIter;
		for (cIter = qIter->second.begin(); cIter != qIter->second.end(); cIter++)
		{
			ProcessTable *table = &(cIter->second);

			std::list<P2POp*>::iterator rIter;
			for (rIter = table->wcRecvs.begin(); rIter != table->wcRecvs.end(); rIter++)
			{
				P2POp* entry = *rIter;

				//We are only looking for wc recvs (there may also be other receives in the queue)
				if (entry->getToRank() != myConsts->getAnySource())
					continue;

				//Either both are blocking or both receives are non-blocking
				if (hasRequest != entry->hasRequest())
					continue;

				//If we have a request it must match with the request of the wildcard receive
				if ((hasRequest && entry->getRequest() != request))
					continue;

				//This is the right request:
				//For blocking receives there can always only be one blocking receive
				//in any table or queue, As after that receive it is guaranteed that the
				//update comes before any further blocking wc receive!
				//For non-blocking receives the request is sufficient as identifier.
				//=>Translate new source with given communicator into comm world
				int newSourceTranslated = translateDestSource (entry->getComm(), newSource);
				entry->updateToSource(newSourceTranslated);

				//Determine whether this was the suspension reason
				if ( mySuspendedForEntry && mySuspendedForEntry->hasRequest() == hasRequest &&
						(!hasRequest || mySuspendedForEntry->getRequest() == request) &&
						mySuspendedForEntry->getIssuerRank() == from)
				{
					if (pOutUnsuspend) *pOutUnsuspend = true;
				}

				//Add to the right recv queue
				RankOutstandings::iterator toRecv = table->recvs.find(newSourceTranslated);
				if (toRecv == table->recvs.end())
				{
					table->recvs.insert(std::make_pair(newSourceTranslated, std::list<P2POp*>()));
					toRecv = table->recvs.find(newSourceTranslated);
				}

				toRecv->second.push_back(entry);
				table->wcRecvs.erase(rIter);

				//Look at the wcRecvs and move all non wc receives into their respective
				//receive queues until it is empty or a wc receive is at the front
				while (!table->wcRecvs.empty())
				{
					rIter = table->wcRecvs.begin();

					if ((*rIter)->getToRank() == myConsts->getAnySource())
						break;

					//Add to the right recv queue
					RankOutstandings::iterator toRecv = table->recvs.find((*rIter)->getToRank());
					if (toRecv == table->recvs.end())
					{
						table->recvs.insert(std::make_pair((*rIter)->getToRank(), std::list<P2POp*>()));
						toRecv = table->recvs.find((*rIter)->getToRank());
					}

					toRecv->second.push_back(*rIter);
					table->wcRecvs.erase(rIter);
				}

				//Done
				return;
			}
		}
	}

	//==2) Search in the suspended queue
	std::map<int, std::list<P2POp* > >::iterator wcRQueue = suspendedWcRecvs.find(from);
	if (wcRQueue == suspendedWcRecvs.end())
		return; // TODO should warn that there was an update without a recv to update

	std::list<P2POp*>::iterator susIter;

	for (susIter = wcRQueue->second.begin(); susIter != wcRQueue->second.end(); susIter++)
	{
		P2POp* wc = *susIter;

		//Request property must match
		if (wc->hasRequest() != hasRequest)
			continue;

		//If there is an associated request it must match
		if (hasRequest && wc->getRequest() != request)
			continue;

		//Hit! Update ! (No translation needed, it was not yet processed and will be translated at that time)
		wc->updateToSource(translateDestSource (wc->getComm(), newSource));

		wcRQueue->second.erase(susIter);

		//Determine whether this was the suspension reason
		if (	mySuspendedForEntry && mySuspendedForEntry->hasRequest() == hasRequest &&
				(!hasRequest || mySuspendedForEntry->getRequest() == request) &&
				mySuspendedForEntry->getIssuerRank() == from)
		{
			if (pOutUnsuspend) *pOutUnsuspend = true;
		}

		break;
	}
}

//=============================
// printQs
//=============================
void P2PMatch::printQs (void)
{
	std::cout << "====== Qs ======" << std::endl;

	QT::iterator qIter;
	for (qIter = myQs.begin(); qIter != myQs.end(); qIter++)
	{
		std::cout << qIter->first << ":" << std::endl;

		ProcessQueues::iterator cIter;
		for (cIter = qIter->second.begin(); cIter != qIter->second.end(); cIter++)
		{
			std::cout << "    " << cIter->first << ":" << std::endl;

			ProcessTable *t = &(cIter->second);

			RankOutstandings::iterator oIter;

			//==== SENDS
			std::cout << "        Sends:" << std::endl;
			for (oIter = t->sends.begin(); oIter != t->sends.end(); oIter++)
			{
				std::cout << "            To " << oIter->first << ": " << std::endl;

				std::list<P2POp*>::iterator lIter;
				for (lIter = oIter->second.begin(); lIter != oIter->second.end(); lIter++)
				{
				    std::cout << "                ";
					if(*lIter) (*lIter)->print(std::cout);
					std::cout << std::endl;
				}

				std::cout << std::endl;
			}

			//==== RECVS
			std::cout << "        Recvs:" << std::endl;
			for (oIter = t->recvs.begin(); oIter != t->recvs.end(); oIter++)
			{
				std::cout << "            From " << oIter->first << ": " << std::endl;

				std::list<P2POp*>::iterator lIter;
				for (lIter = oIter->second.begin(); lIter != oIter->second.end(); lIter++)
				{
				    std::cout << "                ";
				    if(*lIter) (*lIter)->print(std::cout);
				    std::cout << std::endl;
				}

				std::cout << std::endl;
			}

			//==== WC-RECVS
			std::cout << "        wcRecvs: " << std::endl;
			std::list<P2POp*>::iterator lIter;
			for (lIter = t->wcRecvs.begin(); lIter != t->wcRecvs.end(); lIter++)
			{
			    std::cout << "                ";
			    if(*lIter) (*lIter)->print(std::cout);
			    std::cout << std::endl;
			}
			std::cout << std::endl;

		}//for comms
	}//for processes

	std::cout << "====== Suspended WC-Receives ======" << std::endl;
	std::map<int, std::list<P2POp*> >::iterator rankWcIter;
	for (rankWcIter = suspendedWcRecvs.begin(); rankWcIter != suspendedWcRecvs.end(); rankWcIter++)
	{
	    std::cout << "RANK " << rankWcIter->first << ":" << std::endl;

	    std::list<P2POp*>::iterator sus;
		for (sus = rankWcIter->second.begin(); sus != rankWcIter->second.end(); sus++)
		{
		    P2POp* entry = *sus;

			if (entry)
				std::cout << "    ->";

			entry->print(std::cout);
			std::cout << std::endl;
		}
	}
}

//=============================
// addMatchToHistory
//=============================
#ifdef MUST_DEBUG
void P2PMatch::addMatchToHistory (P2POp* send, P2POp* recv)
{
    //Add to matches for graphical representation
    AllMatchHistories::iterator historyIter;
    for (historyIter = myMatches.begin(); historyIter != myMatches.end(); historyIter++)
        if (historyIter->first->compareComms(send->getComm()))
            break;
    if (historyIter == myMatches.end())
    {
        std::pair<AllMatchHistories::iterator, bool> ret = myMatches.insert (std::make_pair(send->getCommCopy(), MatchHistory ()));
        historyIter = ret.first;
    }

    historyIter->second[send->getIssuerRank()][recv->getIssuerRank()].push_back (send->getTag());
}
#endif

//=============================
// printMatchHistories
//=============================
#ifdef MUST_DEBUG
void P2PMatch::printMatchHistories (void)
{
    /**
     * @todo with intercommunicators the comm translation will be very wacky, think about that.
     */

    AllMatchHistories::iterator cIter;
    int d = 0;
    for (cIter = myMatches.begin(); cIter != myMatches.end(); cIter++, d++)
    {
        I_Comm *comm = cIter->first;

        MatchHistory::iterator sendIter;

        std::stringstream stream, infostr;
        stream << "must_match_history_comm_" << d << ".dot";
        std::ofstream out (stream.str().c_str());
        std::list<std::pair <MustParallelId, MustLocationId> > references;

        out << "digraph MsgMatchHistory {" << std::endl;
        out << "commInfo [label=\"";
        comm->printInfo (infostr, &references);
        out << infostr.str() << "\"];"<<std::endl;

        for (sendIter = cIter->second.begin(); sendIter != cIter->second.end(); sendIter++)
        {
            int send = sendIter->first;
            int sendTranslated = invTranslateDestSource (comm, send);

            std::map<int, std::list<int> >::iterator receiveIter;
            for (receiveIter = sendIter->second.begin(); receiveIter != sendIter->second.end(); receiveIter++)
            {
                int receive = receiveIter->first;
                int receiveTranslated = invTranslateDestSource (comm, receive);

                out << sendTranslated << "->" << receiveTranslated << " [label=\"";

                std::list<int>::iterator tagIter;
                for (tagIter = receiveIter->second.begin(); tagIter != receiveIter->second.end(); tagIter++)
                {
                    int tag = *tagIter;

                    if (tagIter != receiveIter->second.begin())
                        out << ", ";

                    out << tag;
                }

                out << "\"];" << std::endl;
            }
        }

        out << "}" << std::endl;
    }
}
#endif

//=============================
// clearMatchHistories
//=============================
#ifdef MUST_DEBUG
void P2PMatch::clearMatchHistory(AllMatchHistories *history)
{
    AllMatchHistories::iterator cIter;
    for (cIter = history->begin(); cIter != history->end(); cIter++)
    {
        cIter->first->erase();
    }
    history->clear ();
}
#endif

//=============================
// copyMatchHistory
//=============================
#ifdef MUST_DEBUG
void P2PMatch::copyMatchHistory(AllMatchHistories *from, AllMatchHistories *to)
{
    AllMatchHistories::iterator iter;

    for (iter = from->begin(); iter != from->end(); iter++)
    {
        //Add
        iter->first->copy();
        to->insert (std::make_pair(iter->first, iter->second));
    }
}
#endif

//=============================
// registerListener
//=============================
GTI_RETURN P2PMatch::registerListener (I_P2PMatchListener *listener)
{
    myListeners.push_back (listener);
    return GTI_SUCCESS;
}

//=============================
// notifyMatch
//=============================
void P2PMatch::notifyMatch (P2POp* send, P2POp *recv)
{
    std::list<P2POp*>::iterator watchIter;
    for (watchIter = myOpsToWatchForMatch.begin(); watchIter != myOpsToWatchForMatch.end(); watchIter++)
    {
        if (send == *watchIter || recv == *watchIter)
        {
            myOpsToWatchForMatch.erase(watchIter);
            watchIter = myOpsToWatchForMatch.begin();
        }
    }

    std::list<I_P2PMatchListener*>::iterator lIter;
    for (lIter = myListeners.begin(); lIter != myListeners.end(); lIter++)
    {
        I_P2PMatchListener* listener = *lIter;
        listener->newMatch(
                send->getIssuerRank(),
                recv->getIssuerRank(),
                send->hasRequest(),
                send->getRequest(),
                recv->hasRequest(),
                recv->getRequest(),
                send->getSendMode());
    }
}

//=============================
// getP2PInfo
//=============================
bool P2PMatch::getP2PInfo (int rank, MustRequestType request, P2PInfo* outInfo)
{
    QT::iterator pos = myQs.find(rank);

    if (pos == myQs.end()) return false;

    //We don't know the comm, lets try all of them!
    ProcessQueues::iterator cIter;
    for (cIter = pos->second.begin(); cIter != pos->second.end(); cIter++)
    {
        ProcessTable *table = &(cIter->second);

        //Sends & Receives
        RankOutstandings::iterator toIter;
        for (int i = 0; i < 2; i++)
        {
            RankOutstandings *outstandings = &(table->sends);
            if (i == 1)
                outstandings = &(table->recvs);

            for (toIter = outstandings->begin(); toIter != outstandings->end(); toIter++)
            {
                std::list<P2POp*>::iterator opIter;
                for (opIter = toIter->second.begin(); opIter != toIter->second.end(); opIter++)
                {
                    P2POp* op = *opIter;

                    if (!op->hasRequest()) continue;

                    if (op->getRequest() != request) continue;

                    fillInfo (op, outInfo);
                    return true;
                }
            }
        }

        //Wc receives
        std::list<P2POp*>::iterator opIter;
        for (opIter = table->wcRecvs.begin(); opIter != table->wcRecvs.end(); opIter++)
        {
            P2POp* op = *opIter;

            if (!op->hasRequest()) continue;

            if (op->getRequest() != request) continue;

            fillInfo (op, outInfo);
            return true;
        }
    }//For communicators

    return false;
}

//=============================
// getP2PInfo
//=============================
bool P2PMatch::getP2PInfo (int rank, bool isSend, P2PInfo* outInfo)
{
    QT::iterator pos = myQs.find(rank);

    if (pos == myQs.end()) return false;

    //We don't know the comm, lets try all of them!
    ProcessQueues::iterator cIter;
    for (cIter = pos->second.begin(); cIter != pos->second.end(); cIter++)
    {
        ProcessTable *table = &(cIter->second);

        //Sends & Receives
        RankOutstandings::iterator toIter;
        RankOutstandings *outstandings = &(table->sends);
        if (!isSend)
            outstandings = &(table->recvs);

        for (toIter = outstandings->begin(); toIter != outstandings->end(); toIter++)
        {
            std::list<P2POp*>::iterator opIter;
            for (opIter = toIter->second.begin(); opIter != toIter->second.end(); opIter++)
            {
                P2POp* op = *opIter;

                if (op->hasRequest()) continue;

                //IMPORTANT: Ignore "blocking" bsends
                if (op->isSend() && op->getSendMode() == MUST_BUFFERED_SEND) continue;

                fillInfo (op, outInfo);
                return true;
            }
        }

        //Wc receives
        if (!isSend)
        {
            std::list<P2POp*>::iterator opIter;
            for (opIter = table->wcRecvs.begin(); opIter != table->wcRecvs.end(); opIter++)
            {
                P2POp* op = *opIter;

                if (op->hasRequest()) continue;

                //IMPORTANT: Ignore "blocking" bsends
                if (op->isSend() && op->getSendMode() == MUST_BUFFERED_SEND) continue;

                fillInfo (op, outInfo);
                return true;
            }
        }//If a receive is being looked for
    }//For communicators

    return false;
}

//=============================
// getP2PInfos
//=============================
std::list<P2PInfo> P2PMatch::getP2PInfos (int fromRank, int toRank, bool sends, bool receives)
{
    std::list<P2PInfo> ret;

    QT::iterator pos = myQs.find(fromRank);

    if (pos == myQs.end()) return ret;

    //We don't know the comm, lets try all of them!
    ProcessQueues::iterator cIter;
    for (cIter = pos->second.begin(); cIter != pos->second.end(); cIter++)
    {
        ProcessTable *table = &(cIter->second);

        //Sends & Receives
        RankOutstandings::iterator toIter;
        std::list<RankOutstandings*> outstandings;
        std::list<RankOutstandings*>::iterator checkIter;
        std::list<P2POp*>::iterator opIter;

        if (sends) outstandings.push_back(&(table->sends));
        if (receives) outstandings.push_back(&(table->recvs));

        for (checkIter = outstandings.begin(); checkIter != outstandings.end(); checkIter++)
        {
            RankOutstandings* outstanding = *checkIter;

            for (opIter = (*outstanding)[toRank].begin(); opIter != (*outstanding)[toRank].end(); opIter++)
            {
                P2POp* op = *opIter;
                P2PInfo info;
                fillInfo (op, &info);
                ret.push_back(info);
            }
        }

        //Wc receives
        for (opIter = table->wcRecvs.begin(); opIter != table->wcRecvs.end(); opIter++)
        {
            P2POp* op = *opIter;

            if (op->wasIssuedAsWcReceive() || op->getToRank() == toRank)
            {
                P2PInfo info;
                fillInfo (op, &info);
                ret.push_back(info);
            }
        }
    }//For communicators

    return ret;
}

//=============================
// fillInfo
//=============================
void P2PMatch::fillInfo (P2POp* op, P2PInfo *info)
{
    if (!info || !op)
        return;

    info->comm = op->getComm();
    info->isSend = op->isSend ();
    info->isWc = op->wasIssuedAsWcReceive();
    info->lId = op->getLId();
    info->pId = op->getPId();
    info->mode = op->getSendMode();
    info->target = op->getToRank();
    info->tag = op->getTag();
}

//=============================
// notifyDeadlock
//=============================
void P2PMatch::notifyDeadlock (void)
{
    myPrintLostMessages = false;
}

//=============================
// clearQ
//=============================
void P2PMatch::clearQ (QT* queue)
{
    QT::iterator rankIter;
    for (rankIter = queue->begin(); rankIter != queue->end();rankIter++)
    {
        ProcessQueues::iterator commIter;
        for (commIter = rankIter->second.begin(); commIter != rankIter->second.end(); commIter++)
        {
            //Free the comm
            commIter->first->erase();

            //Free any ops in the table
            std::list<P2POp*>::iterator opIter;

            //WC Ops
            for (opIter = commIter->second.wcRecvs.begin(); opIter != commIter->second.wcRecvs.end(); opIter++)
            {
                if (*opIter) delete (*opIter);
            }

            //Sends
            RankOutstandings::iterator toIter;
            for (toIter = commIter->second.sends.begin(); toIter != commIter->second.sends.end(); toIter++)
            {
                for (opIter = toIter->second.begin(); opIter != toIter->second.end(); opIter++)
                {
                    if (*opIter) delete (*opIter);
                }
            }

            //Receives
            for (toIter = commIter->second.recvs.begin(); toIter != commIter->second.recvs.end(); toIter++)
            {
                for (opIter = toIter->second.begin(); opIter != toIter->second.end(); opIter++)
                {
                    if (*opIter) delete (*opIter);
                }
            }
        }
    }
    queue->clear ();
}

//=============================
// checkpoint
//=============================
void P2PMatch::checkpoint (void)
{
    //==0) Child modules do not change

    //==1) Matching queues
    clearQ (&myCheckpointQs);

    QT::iterator iter;
    for (iter = myQs.begin(); iter != myQs.end(); iter++)
    {
        //Copy process queues
        ProcessQueues pQueues;
        ProcessQueues::iterator pqIter;

        for (pqIter = iter->second.begin(); pqIter != iter->second.end(); pqIter++)
        {
            //Copy ProcessTable
            ProcessTable table;
            ProcessTable *other = &pqIter->second;
            std::list<P2POp*>::iterator opIter;
            RankOutstandings::iterator ranksIter;

            //sends
            for (ranksIter = other->sends.begin(); ranksIter != other->sends.end(); ranksIter++)
            {
                //Copy Ops
                std::list<P2POp*> temp;
                for (opIter = ranksIter->second.begin(); opIter != ranksIter->second.end(); opIter++)
                {
                    temp.push_back((*opIter)->copy());
                }

                //Add
                table.sends.insert(std::make_pair(ranksIter->first, temp));
            }

            //recvs
            for (ranksIter = other->recvs.begin(); ranksIter != other->recvs.end(); ranksIter++)
            {
                //Copy Ops
                std::list<P2POp*> temp;
                for (opIter = ranksIter->second.begin(); opIter != ranksIter->second.end(); opIter++)
                {
                    temp.push_back((*opIter)->copy());
                }

                //Add
                table.recvs.insert(std::make_pair(ranksIter->first, temp));
            }

            //wcRecvs
            for (opIter = other->wcRecvs.begin(); opIter != other->wcRecvs.end(); opIter++)
            {
                P2POp* newOp = (*opIter)->copy();
                table.wcRecvs.push_back(newOp);

                //Was this a suspension reason?
                if (*opIter == mySuspendedForEntry)
                    myCheckpointSuspendedForEntry = newOp;
            }

            //Add
            if (pqIter->first) pqIter->first->copy();
            pQueues.insert(std::make_pair (pqIter->first, table));
        }

        //Add
        myCheckpointQs.insert(std::make_pair(iter->first, pQueues));
    }

    //==3) suspendedWcRecvs
    /*
     * We empty the suspended wc recv lists.
     * The individual ops that are in the current list will add themselves
     * to this queue during their copy action
     */
    myCheckpointSuspendedWcRecvs.clear();

    //==4) myFinCompletion
    if (myCheckpointFinCompletion) delete (myCheckpointFinCompletion);
    if (myFinCompletion)
        myCheckpointFinCompletion = myFinCompletion->copy();
    else
        myCheckpointFinCompletion = NULL;

    //==5) myMatches (DEBUG)
#ifdef MUST_DEBUG
    clearMatchHistory (&myCheckpointMatches);
    copyMatchHistory (&myMatches, &myCheckpointMatches);
#endif

    //==6) myPrintLostMessages
    myCheckpointPrintLostMessages = myPrintLostMessages;

    //==7) Clear myOpsToWatchForMatch
    myOpsToWatchForMatch.clear(); //This is the way we use the list

    //==8) Store lists of enforced matches (to avoid wrong wild-card updates)
    myCheckpointDecidedIrecvs.clear();
    myCheckpointDecidedRecvs.clear();
    myCheckpointDecidedIrecvs = myDecidedIrecvs;
    myCheckpointDecidedRecvs = myDecidedRecvs;
}

//=============================
// rollback
//=============================
void P2PMatch::rollback (void)
{
    //==0) Child modules do not change

    //==1) Matching queues
    clearQ (&myQs); //This must be an actual delete
    myQs = myCheckpointQs;
    myCheckpointQs.clear(); //This must be a simple clear

    mySuspendedForEntry = myCheckpointSuspendedForEntry;
    myCheckpointSuspendedForEntry = NULL;

    //==3) suspendedWcRecvs
    suspendedWcRecvs.clear();
    suspendedWcRecvs = myCheckpointSuspendedWcRecvs;
    myCheckpointSuspendedWcRecvs.clear();

    //==4) myFinCompletion
    if (myFinCompletion) delete (myFinCompletion);
    myFinCompletion = myCheckpointFinCompletion;
    myCheckpointFinCompletion = NULL;

    //==5) myMatches (DEBUG)
#ifdef MUST_DEBUG
    clearMatchHistory (&myMatches);
    myMatches = myCheckpointMatches;
    myCheckpointMatches.clear();
#endif

    //==6) myPrintLostMessages
    myPrintLostMessages = myCheckpointPrintLostMessages;

    //==7) Clear myOpsToWatchForMatch
    myOpsToWatchForMatch.clear(); //This is the way we use the list

    //==8) Restore lists of enforced matches (to avoid wrong wild-card updates)
    myDecidedIrecvs.clear();
    myDecidedRecvs.clear();
    myDecidedIrecvs = myCheckpointDecidedIrecvs;
    myDecidedRecvs = myCheckpointDecidedRecvs;
}

//=============================
// decideSuspensionReason
//=============================
bool P2PMatch::decideSuspensionReason (
                        int decissionIndex,
                        int *outNumAlternatives)
{
    //Are we suspended at all?
    if (!mySuspendedForEntry)
        return false;

    //How many alternatives do we have? (Test the index and return it to the caller)
    int numAlternatives = mySuspendedForEntry->getComm()->getGroup()->getSize();

    if (decissionIndex >= numAlternatives)
        return false;

    if (outNumAlternatives)
        *outNumAlternatives = numAlternatives;

    //Lets do an update
    /*
     * If the decission index is 0 we use the first match we saw
     * otherwise we use any other decission than the first match.
     */
    int rankToDecideFor;
    int firstMatchLocalRank;
    assert (mySuspendedForEntry->getFirstWorldRankWithValidMatch() >= 0);
    mySuspendedForEntry->getComm()->getGroup()->containsWorldRank(mySuspendedForEntry->getFirstWorldRankWithValidMatch(), &firstMatchLocalRank);

    if (decissionIndex == 0)
    {
        rankToDecideFor = firstMatchLocalRank;
    }
    else
    {
        rankToDecideFor = decissionIndex - 1;
        if (rankToDecideFor >= firstMatchLocalRank)
            rankToDecideFor++;
    }


    bool unsuspend = false;
    int opRank = myPIdMod->getInfoForId(mySuspendedForEntry->getPId()).rank;
    findRecvForUpdate (
            opRank,
            rankToDecideFor,
            mySuspendedForEntry->hasRequest(),
            mySuspendedForEntry->getRequest(),
            &unsuspend);

    //Store the enforced match in our lists! (to drop a source update that might arrive at a later point in time)
    if (mySuspendedForEntry->hasRequest())
    {
        myDecidedIrecvs[opRank].push_back(mySuspendedForEntry->getRequest());
    }
    else
    {
        if (myDecidedRecvs.find(opRank) == myDecidedRecvs.end())
            myDecidedRecvs[opRank] = 1;
        else
            myDecidedRecvs[opRank] = myDecidedRecvs[opRank] + 1;
    }

    //Sanity
    if (!unsuspend)
    {
        std::cerr << "Internal Error: tried to enforce a wc-receive source update but failed to remove the suspension with that." << std::endl;
        return false;
    }

    //Prepare watching for our fixed op
    myOpsToWatchForMatch.push_back(mySuspendedForEntry);

    //Remove suspension and continue
    if (unsuspend)
        myOrder->removeSuspension();

    //Was this alright?
    /**
     * @todo Right now this is horrible in terms of search space!
     * We may have to fix multiple wc recvs even before we recognize
     * that the first fix was invalid. To make this right we need
     * to change the suspension mechanism as follows:
     * - P2P tables never store a wc recv for which we suspend
     * - If a wc recv is in the tables and a first send shows up we need to remove the recv and add the send instead
     *  (Beware we must retain order in that case, so likely also remove anything that came after the wc receive)
     *  - Instead of suspending the whole operation execution we just suspend the wc receives rank and continue as long as possible
     *  - With that we basically know all (or hopefully many) of the matching sends for a susended receive
     *  => This information allows us to make the right decissions in the first place!
     */

    //It is right if we ended up in a new suspension (irrespective if we found a match for out proposed fix)
    if (myOrder->isSuspended())
        return true;

    //If we are not suspended then the whole list of ops to watch for must be empty,
    //otherwise we did an infullfilable fix!
    if (!myOpsToWatchForMatch.empty())
        return false;

    return true;
}

//=============================
// canOpBeProcessed
//=============================
bool P2PMatch::canOpBeProcessed (
                        MustParallelId pId,
                        MustCommType comm,
                        int sourceDest)
{
    //MPI_PROC_NULL is always a no-op and can thus be processed
    if (myConsts->isProcNull(sourceDest))
        return true;

    if (sourceDest == myConsts->getAnySource())
    {
        /*
         * TODO
         * @todo
         * This is currently not supported!
         */
        //std::cout << "Lost message match reduction not supported for wildcards!" << std::endl;
        //assert (0);
        return true;
    }

    I_Comm *info = myCTrack->getComm(pId,comm);
    if (!info) return true; //???

    return info->isRankReachable(sourceDest);
}

//=============================
// canOpBeProcessed
//=============================
bool P2PMatch::canOpBeProcessed (
                        MustParallelId pId,
                        I_Comm* comm,
                        int sourceDest)
{
    //MPI_PROC_NULL is always a no-op and can thus be processed
    if (myConsts->isProcNull(sourceDest))
        return true;

    if (sourceDest == myConsts->getAnySource())
    {
        /*
         * TODO
         * @todo
         * This is currently not supported!
         */
        //std::cout << "Lost message match reduction not supported for wildcards!" << std::endl;
        //assert (0);
        return true;
    }

    return comm->isRankReachable(sourceDest);
}

/*EOF*/
