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
 * @file DP2PMatch.cpp
 *       @see MUST::DP2PMatch.
 *
 * @date 20.01.2012
 * @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "DP2PMatch.h"

#include <assert.h>
#include <sstream>
#include <fstream>

using namespace must;

mGET_INSTANCE_FUNCTION(DP2PMatch)
mFREE_INSTANCE_FUNCTION(DP2PMatch)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DP2PMatch)

//=============================
// SuspensionInfo
//=============================
SuspensionInfo::SuspensionInfo (void)
 : isSuspended (false),
   suspensionReason (NULL),
   furtherReasons (),
   queue ()
{
   //Nothing to do
}

//=============================
// addReason
//=============================
bool SuspensionInfo::addReason (DP2POp* reason)
{
    if (!suspensionReason)
    {
        suspensionReason = reason;
        return true;
    }

    //This is expensive, but should be rarely needed
    std::list<DP2POp*>::iterator iter;
    for (iter = furtherReasons.begin(); iter != furtherReasons.end(); iter++)
    {
        if (*iter == reason)
            return false;
    }

    furtherReasons.push_back(reason);
    return true;
}

//=============================
// removeReason
//=============================
bool SuspensionInfo::removeReason (DP2POp* reason)
{
    //Is it the first reason?
    if (suspensionReason == reason || !suspensionReason)
    {
        suspensionReason = NULL;

        //Do we need to "refill" the first reason?
        if (furtherReasons.empty())
            return true;

        suspensionReason = furtherReasons.front();
        furtherReasons.pop_front();
        return true;
    }

    //Check the other reasons
    std::list<DP2POp*>::iterator iter;
    for (iter = furtherReasons.begin(); iter != furtherReasons.end(); iter++)
    {
        if (*iter == reason)
        {
            furtherReasons.erase(iter);
            return true;
        }
    }

    //Was not a reason for us ...
    return false;
}

//=============================
// Constructor
//=============================
DP2PMatch::DP2PMatch (const char* instanceName)
    : gti::ModuleBase<DP2PMatch, I_DP2PMatch> (instanceName),
      myPlaceId (-1),
      myListener (NULL),
      myQs (),
      myQSize (0),
      myMaxQSize (0),
      mySendsToTransfer (),
      mySuspension (),
      myIsInProcessQueues (false)
#ifdef MUST_DEBUG
      ,myMatches ()
#endif
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 8
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
    myLIdMod = (I_LocationAnalysis*) subModInstances[1];
    myConsts = (I_BaseConstants*) subModInstances[2];
    myLogger = (I_CreateMessage*) subModInstances[3];
    myCTrack = (I_CommTrack*) subModInstances[4];
    myRTrack = (I_RequestTrack*) subModInstances[5];
    myDTrack = (I_DatatypeTrack*) subModInstances[6];
    myFloodControl = (I_FloodControl*) subModInstances[7];
    myProfiler = NULL;

    //Initialize module data
    getWrapAcrossFunction("passSendForMatching", (GTI_Fct_t*)&myPassSendFunction);
    getWrapAcrossFunction("passIsendForMatching", (GTI_Fct_t*)&myPassIsendFunction);
    getWrapAcrossFunction("passSendStartForMatching", (GTI_Fct_t*)&myPassSendStartFunction);

    //Assert correct mapping
    //TODO this is somewhat crude
    int numPlacesOnLevel = 0;
    int tempTarget, lastTarget = -1;
    int rank = 0;

    while (getLevelIdForApplicationRank (rank, &tempTarget) == GTI_SUCCESS)
    {
        rank++;
        if (lastTarget != tempTarget)
            numPlacesOnLevel++;
    }

    if (numPlacesOnLevel > 1 && (!myPassSendFunction || !myPassIsendFunction))
    {
        std::cerr << "ERROR: Distributed P2P Matching was mapped on a layer of size > 0 while no intra layer communication was present, as a result P2P matching will not be possible. Either add an intra layer communication, or map the P2P matching onto a layer with a single process." << std::endl;
        assert (0);
    }
}

//=============================
// Destructor
//=============================
DP2PMatch::~DP2PMatch ()
{
	if (myPIdMod)
		destroySubModuleInstance ((I_Module*) myPIdMod);
	myPIdMod = NULL;

	if (myLIdMod)
	    destroySubModuleInstance ((I_Module*) myLIdMod);
	myLIdMod = NULL;

	if (myConsts)
	    destroySubModuleInstance ((I_Module*) myConsts);
	myConsts = NULL;

	if (myLogger)
		destroySubModuleInstance ((I_Module*) myLogger);
	myLogger = NULL;

	if (myCTrack)
	{
	    myCTrack->notifyOfShutdown();
		destroySubModuleInstance ((I_Module*) myCTrack);
	}
	myCTrack = NULL;

	if (myRTrack)
	{
	    myRTrack->notifyOfShutdown();
		destroySubModuleInstance ((I_Module*) myRTrack);
	}
	myRTrack = NULL;

	if (myDTrack)
	{
	    myDTrack->notifyOfShutdown();
	    destroySubModuleInstance ((I_Module*) myDTrack);
	}
	myDTrack = NULL;

	if (myFloodControl)
	    destroySubModuleInstance ((I_Module*) myFloodControl);
	myFloodControl = NULL;

	if (myProfiler)
	{
	    myProfiler->reportWrapperAnalysisTime ("DP2PMatch", "maxEventQueue", 0, myMaxQSize);
	    myProfiler->reportWrapperAnalysisTime ("DP2PMatch", "finalQueueSize", 0, myQSize);
	    destroySubModuleInstance ((I_Module*) myProfiler);
	}
	myProfiler = NULL;

	//==Free other data
	//Matching structure
	//We do the graceful free in printLostMessages, we should not destruct anything here as destruction hell is already going on.
	myQs.clear ();
}

//=============================
// init
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::init (MustParallelId pId)
{
    if (myPlaceId < 0)
        getLevelIdForApplicationRank (myPIdMod->getInfoForId(pId).rank, &myPlaceId);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// send
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::send (
		MustParallelId pId,
		MustLocationId lId,
		int destIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
        int mode,
        MustLTimeStamp remoteTS)
{
    //== 1) Get rid of PROC_NULL ops
    if (myConsts->isProcNull(destIn))
        return GTI_ANALYSIS_SUCCESS;

    //== 2) Get infos and translation
    int rank = myPIdMod->getInfoForId(pId).rank;
    int targetPlace;
    int sourcePlace;
    getLevelIdForApplicationRank (rank, &sourcePlace);
    bool isFromRemotePlace = (myPlaceId != sourcePlace);
    int dest;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (rank, pId, comm, destIn, type, &cInfo, &dest, &dInfo, isFromRemotePlace))
        return GTI_ANALYSIS_SUCCESS;
    getLevelIdForApplicationRank (dest, &targetPlace);

    //== 3) Pass the send across if necessary
    //== 3alpha) Notify our P2P listener if necessary
    MustLTimeStamp eventLTime = 0;
    bool active=true;
    if (myListener && sourcePlace == myPlaceId)
    {
        cInfo->copy();
        eventLTime = myListener->newP2POp (
                pId,
                lId,
                cInfo,
                true,
                dest,
                false,
                (MustSendMode) mode,
                tag,
                false,
                0,
                &active
                );
    }
    else
    {
        //If this is a remote event, use the timestamp that is associated there
        eventLTime = remoteTS;
    }

    /**
     * We send if the target place for this op is different from our place id.
     * BUT, we do not send if our place id is not yet initialized, in that case
     * the op is definitiely targeted for this place, as otherwise we would have
     * inited the place id in the init function first!
     */
    if (myPlaceId != targetPlace && myPlaceId >= 0)
    {
        //Forward associated resources first
        myLIdMod->passLocationToPlace(pId, lId, targetPlace);

        //Pass across
        if (active)
        {
	    //Forward associated resources first
            MustRemoteIdType typeRId, commRId;
            myCTrack->passCommAcross(rank, cInfo, targetPlace, &commRId);
       	    myDTrack->passDatatypeAcross(rank, dInfo, targetPlace, &typeRId);

            if (myPassSendFunction)
                (*myPassSendFunction) (pId, lId, destIn, tag, commRId, typeRId, count, mode, eventLTime, targetPlace);

            //Free persistent handles
            if (cInfo)
                cInfo->erase();
            if (dInfo)
                dInfo->erase();
        }
        else
        {
            PassSendInfo sInfo;
            sInfo.pId = pId;
            sInfo.lId = lId;
            sInfo.destIn = destIn;
            sInfo.tag = tag;
            sInfo.comm = comm;
            sInfo.type = type;
            sInfo.count = count;
            sInfo.mode = mode;
            sInfo.targetPlace = targetPlace;
            sInfo.hasRequest = false;
            sInfo.isPersistent = false;
            sInfo.request = 0;
            sInfo.cInfo = cInfo;
            sInfo.dInfo = dInfo;
            mySendsToTransfer.insert(std::make_pair(
                    std::make_pair(rank, eventLTime),
                    sInfo));
        }

        return GTI_ANALYSIS_SUCCESS;
    }

    //== 3b) Set channel information in mySuspension
    GTI_STRATEGY_TYPE direction;
    unsigned int channel;
    myFloodControl->getCurrentRecordInfo(&direction, &channel);
    mySuspension[rank].direction = direction;
    mySuspension[rank].channel = channel;

	//== 4) Create op
	DP2POp* newOp = new DP2POp (
	        this,
	        true,
	        tag,
	        dest /*TRANSLATED dest*/,
	        cInfo,
	        dInfo,
	        count,
	        pId,
	        lId,
	        eventLTime,
	        (MustSendMode) mode);

	//== 5) Process
    handleNewOp (rank, newOp);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isend
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::isend (
		MustParallelId pId,
		MustLocationId lId,
		int destIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
        int mode,
		MustRequestType request,
        MustLTimeStamp remoteTS)
{
    //== 1) Get rid of PROC_NULL ops
    if (myConsts->isProcNull(destIn))
        return GTI_ANALYSIS_SUCCESS;

    //== 2) Get infos and translation
    int rank = myPIdMod->getInfoForId(pId).rank;
    int targetPlace;
    int sourcePlace;
    getLevelIdForApplicationRank (rank, &sourcePlace);
    bool isFromRemotePlace = (myPlaceId != sourcePlace);
    int dest;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (rank, pId, comm, destIn, type, &cInfo, &dest, &dInfo, isFromRemotePlace))
        return GTI_ANALYSIS_SUCCESS;
    getLevelIdForApplicationRank (dest, &targetPlace);

    //== 3) Pass the send across if necessary
    //== 3alpha) Notify our P2P listener if necessary
    MustLTimeStamp eventLTime = 0;
    bool active = true;
    if (myListener && sourcePlace == myPlaceId)
    {
        cInfo->copy();
        eventLTime = myListener->newP2POp (
                pId,
                lId,
                cInfo,
                true,
                dest,
                false,
                (MustSendMode) mode,
                tag,
                true,
                request,
                &active
        );
    }
    else
    {
        //If this is a remote event, use the timestamp that is associated there
        eventLTime = remoteTS;
    }

    /**
     * We send if the target place for this op is different from our place id.
     * BUT, we do not send if our place id is not yet initialized, in that case
     * the op is definitiely targeted for this place, as otherwise we would have
     * inited the place id in the init function first!
     */
    if (myPlaceId != targetPlace && myPlaceId >= 0)
    {
        //Forward associated resources first
        myLIdMod->passLocationToPlace(pId, lId, targetPlace);

        //Pass across
        if (active)
        {
            //Forward associated resources first
            MustRemoteIdType typeRId, commRId;
            myCTrack->passCommAcross(rank, cInfo, targetPlace, &commRId);
            myDTrack->passDatatypeAcross(rank, dInfo, targetPlace, &typeRId);

            //Forward the actual isend
            if (myPassIsendFunction)
                (*myPassIsendFunction) (pId, lId, destIn, tag, commRId, typeRId, count, mode, request, eventLTime, targetPlace);

            //Free persistent handles
            if (cInfo)
                cInfo->erase();
            if (dInfo)
                dInfo->erase();
        }
        else
        {
            //Delay the forwarding
            PassSendInfo sInfo;
            sInfo.pId = pId;
            sInfo.lId = lId;
            sInfo.destIn = destIn;
            sInfo.tag = tag;
            sInfo.comm = comm;
            sInfo.type = type;
            sInfo.count = count;
            sInfo.mode = mode;
            sInfo.targetPlace = targetPlace;
            sInfo.hasRequest = true;
            sInfo.isPersistent = false;
            sInfo.request = request;
            sInfo.cInfo = cInfo;
            sInfo.dInfo = dInfo;
            mySendsToTransfer.insert(std::make_pair(
                    std::make_pair(rank, eventLTime),
                    sInfo));
        }

        return GTI_ANALYSIS_SUCCESS;
    }

    //== 3b) Set channel information in mySuspension
    GTI_STRATEGY_TYPE direction;
    unsigned int channel;
    myFloodControl->getCurrentRecordInfo(&direction, &channel);
    mySuspension[rank].direction = direction;
    mySuspension[rank].channel = channel;

    //== 4) Create op
    DP2POp* newOp = new DP2POp (
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
            eventLTime,
            (MustSendMode) mode);

    //== 5) Process or Queue ?
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// recv
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::recv (
		MustParallelId pId,
		MustLocationId lId,
		int sourceIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count)
{
    //== 1) Prepare the operation for this receive
    //MPI_PROC_NULL ops are no-ops!
    if (myConsts->isProcNull(sourceIn))
        return GTI_ANALYSIS_SUCCESS;

    //Get infos and translation
    int rank = myPIdMod->getInfoForId(pId).rank;
    int source;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (rank, pId, comm, sourceIn, type, &cInfo, &source, &dInfo, false))
        return GTI_ANALYSIS_SUCCESS;

    //Get logical time stamp (if we have a listener)
    MustLTimeStamp eventLTime = 0;
    bool active = true;
    if (myListener)
    {
        cInfo->copy();
        eventLTime = myListener->newP2POp (
                pId,
                lId,
                cInfo,
                false,
                source,
                source == myConsts->getAnySource(),
                MUST_UNKNOWN_SEND,
                tag,
                false,
                0,
                &active
        );
    }

    //== 1b) Set channel information in mySuspension
    GTI_STRATEGY_TYPE direction;
    unsigned int channel;
    myFloodControl->getCurrentRecordInfo(&direction, &channel);
    mySuspension[rank].direction = direction;
    mySuspension[rank].channel = channel;

    //Create op
    DP2POp* newOp = new DP2POp (
            this,
            false,
            tag,
            source /*TRANSLATED source*/,
            cInfo,
            dInfo,
            count,
            pId,
            lId,
            eventLTime);

    //== 2) Process or Queue ?
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::irecv (
		MustParallelId pId,
		MustLocationId lId,
		int sourceIn,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
		MustRequestType request)
{
    //== 1) Prepare the operation for this receive
    //MPI_PROC_NULL ops are no-ops!
    if (myConsts->isProcNull(sourceIn))
        return GTI_ANALYSIS_SUCCESS;

    //Get infos and translation
    int rank = myPIdMod->getInfoForId(pId).rank;
    int source;
    I_CommPersistent* cInfo;
    I_DatatypePersistent *dInfo;
    if (!getCommTranslationAndType (rank, pId, comm, sourceIn, type, &cInfo, &source, &dInfo, false))
        return GTI_ANALYSIS_SUCCESS;

    //Get logical time stamp (if we have a listener)
    MustLTimeStamp eventLTime = 0;
    bool active = true;
    if (myListener)
    {
        cInfo->copy();
        eventLTime = myListener->newP2POp (
                pId,
                lId,
                cInfo,
                false,
                source,
                source == myConsts->getAnySource(),
                MUST_UNKNOWN_SEND,
                tag,
                true,
                request,
                &active
        );
    }

    //== 1b) Set channel information in mySuspension
    GTI_STRATEGY_TYPE direction;
    unsigned int channel;
    myFloodControl->getCurrentRecordInfo(&direction, &channel);
    mySuspension[rank].direction = direction;
    mySuspension[rank].channel = channel;

    //Create op
    DP2POp* newOp = new DP2POp (
            this,
            false,
            tag,
            source /*TRANSLATED source*/,
            request,
            cInfo,
            dInfo,
            count,
            pId,
            lId,
            eventLTime);

    //== 2) Process or Queue ?
    handleNewOp (rank, newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getCommTranslationAndType
//=============================
bool DP2PMatch::getCommTranslationAndType (
        int rank,
        MustParallelId pId,
        MustCommType comm,
        int rankIn,
        MustDatatypeType type,
        I_CommPersistent**pOutComm,
        int *pOutTranslatedRank,
        I_DatatypePersistent**pOutType,
        bool isFromRemotePlace)
{
    //==1) Comm
    I_CommPersistent* cInfo = NULL;
    if (!isFromRemotePlace)
        cInfo = myCTrack->getPersistentComm(rank,comm);
    else
        cInfo = myCTrack->getPersistentRemoteComm(rank,comm);

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
    I_DatatypePersistent *dInfo = NULL;
    if (!isFromRemotePlace)
        dInfo = myDTrack->getPersistentDatatype (rank, type);
    else
        dInfo = myDTrack->getPersistentRemoteDatatype (rank, type);

    if (!dInfo) return false; //Unknown datatype

    if (pOutType) *pOutType = dInfo;

    return true;
}

//=============================
// translateDestSource
//=============================
int DP2PMatch::translateDestSource (I_Comm* comm, int destSource)
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
int DP2PMatch::invTranslateIssuingRank (I_Comm* comm, int rank)
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
int DP2PMatch::invTranslateDestSource (I_Comm* comm, int rank)
{
    int ret;
    if (rank != myConsts->getAnySource())
    {
        if (!comm->isIntercomm())
            comm->getGroup()->containsWorldRank(rank, &ret);
        else
        {
            if (!comm->getRemoteGroup()->containsWorldRank(rank, &ret))
                comm->getGroup()->containsWorldRank(rank, &ret);
        }
    }
    else
    {
        ret = rank;
    }

    return ret;
}

//=============================
// startPersistent
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::startPersistent (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request,
        MustLTimeStamp remoteTS)
{
    //== 1) Check the request for correctness
    I_Request* info = myRTrack->getRequest(pId, request);

    if (info == NULL || !info->isPersistent())
        return GTI_ANALYSIS_SUCCESS;

    /*We are mapped before RequestTrack, so we check here whether the request is still active!*/
    /**
     * Note: if this is a remote request, it will have the state that the original request had when we
     * FIRST sent it! However, we only sent this request if it was active, unless somone else also
     * sends these requests (with another state), this is all fine. But it still is something to be very
     * wary of.
     */
    if (info->isActive())
        return GTI_ANALYSIS_SUCCESS;

    //== 2) Prepare persitent infos and dest/source translation
    int destSource;
    if (info->isSend())
        destSource = info->getDest();
    else
        destSource = info->getSource();

    //Don't do anything for ops with MPI_PROC_NULL
    if (destSource == myConsts->getProcNull())
        return GTI_ANALYSIS_SUCCESS;

    I_CommPersistent* comm = info->getCommCopy();
    if (comm == NULL)
        return GTI_ANALYSIS_SUCCESS;//Unknown comm
    if (comm->isNull())
    {
        comm->erase();
        return GTI_ANALYSIS_SUCCESS;//MPI_COMM_NULL
    }

    I_DatatypePersistent* type = info->getDatatypeCopy();

    int destSourceTranslate = translateDestSource (comm, destSource);

    //== 3alpha) Get time stamp
    int sourcePlace;
    if (myListener)
        getLevelIdForApplicationRank (myPIdMod->getInfoForId(pId).rank, &sourcePlace);

    MustLTimeStamp eventLTime = 0;
    bool active = true;
    if (myListener && sourcePlace == myPlaceId)
    {
        comm->copy();
        eventLTime = myListener->newP2POp (
                pId,
                lId,
                comm,
                info->isSend(),
                destSourceTranslate,
                destSourceTranslate == myConsts->getAnySource(),
                info->getSendMode(),
                info->getTag(),
                true,
                request,
                &active
        );
    }
    else
    {
        //If this is a remote event, use the timestamp that is associated there
        eventLTime = remoteTS;
    }

    //== 3) Pass the start across if necessary (if send not targeting this place)
    if (info->isSend())
    {
        int targetPlace;
        getLevelIdForApplicationRank (destSourceTranslate, &targetPlace);

        if (myPlaceId != targetPlace && myPlaceId >= 0)
        {
            //Pass persistent request across
            myRTrack->passRequestAcross(pId, request, targetPlace);

            if (active)
            {
                if (myPassSendStartFunction)
                    (*myPassSendStartFunction) (pId, lId, request, eventLTime, targetPlace);

                //Free persistent handles
                if (comm)
                    comm->erase();
                if (type)
                    type->erase();
            }
            else
            {
                PassSendInfo sInfo;
                sInfo.pId = pId;
                sInfo.lId = lId;
                sInfo.destIn = 0;
                sInfo.tag = 0;
                sInfo.comm = 0;
                sInfo.type = 0;
                sInfo.count = 0;
                sInfo.mode = 0;
                sInfo.targetPlace = targetPlace;
                sInfo.hasRequest = true;
                sInfo.isPersistent = true;
                sInfo.request = request;
                sInfo.cInfo = comm;
                sInfo.dInfo = type;
                mySendsToTransfer.insert(std::make_pair(
                        std::make_pair(myPIdMod->getInfoForId(pId).rank, eventLTime),
                        sInfo));
            }

            return GTI_ANALYSIS_SUCCESS;
        }
    }

    //== 4) Create the op
    DP2POp* newOp = new DP2POp (
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
            eventLTime,
            info->getSendMode());

    //== 5) Handle the op
    handleNewOp (newOp->getIssuerRank(), newOp);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handleNewOp
//=============================
void DP2PMatch::handleNewOp (int rank, DP2POp* op)
{
    //Can we process at all?
    if (mySuspension[rank].isSuspended)
    {
        suspendOp (op, NULL); //Reason already exists, so we use NULL here
        return;
    }

    //Process
    op->process(rank);
}

//=============================
// recvUpdate
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::recvUpdate (
		MustParallelId pId,
		MustLocationId lId,
		int source)
{
	int from = myPIdMod->getInfoForId(pId).rank;

	//==Perform the update
	std::list<int> reopenedRanks;
	findRecvForUpdate (from, source, false, 0, &reopenedRanks);

#ifdef MUST_MATCH_DEBUG
	std::cout << "DP2PMatch: WcRecvUpdate newSource=" << source << " from rank " << myPIdMod->getInfoForId(pId).rank << std::endl;
	printQs ();
#endif

	//Process any events of unsuspended ranks
	processQueuedEvents (&reopenedRanks);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// irecvUpdate
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::irecvUpdate (
		MustParallelId pId,
		MustLocationId lId,
		int source,
		MustRequestType request)
{
	int from = myPIdMod->getInfoForId(pId).rank;

	//==Perform the update
	std::list<int> reopenedRanks;
	findRecvForUpdate (from, source, true, request, &reopenedRanks);

#ifdef MUST_MATCH_DEBUG
	std::cout << "LostMessage: WcIrecvUpdate newSource=" << source << " request=" << request << " from rank " << myPIdMod->getInfoForId(pId).rank << std::endl;
	printQs ();
#endif

	//Process any events of unsuspended ranks
	processQueuedEvents (&reopenedRanks);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// cancel
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::cancel (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	static bool warned = false;
	if (!warned)
		std::cerr << "DP2PMatch: detected a cancel, not supported, outputs may be wrong!" << std::endl;
	warned = true;

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// printLostMessages
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::printLostMessages (
		I_ChannelId *thisChannel)
{
	//Log the lost messages
	printLostMessages ();

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// printLostMessages
//=============================
GTI_ANALYSIS_RETURN DP2PMatch::printLostMessages ()
{
	QT::iterator qIter;

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
	                    std::list<DP2POp*>::iterator lIter;
	                    for (lIter = oIter->second.begin(); lIter != oIter->second.end(); lIter++)
	                    {
	                        if (*lIter) (*lIter)->logAsLost(qIter->first);
	                    }
	                }

	                //==== RECVS
	                for (oIter = t->recvs.begin(); oIter != t->recvs.end(); oIter++)
	                {
	                    std::list<DP2POp*>::iterator lIter;
	                    for (lIter = oIter->second.begin(); lIter != oIter->second.end(); lIter++)
	                    {
	                        if (*lIter) (*lIter)->logAsLost(qIter->first);
	                    }
	                }

	                //==== WC-RECVS
	                std::list<DP2POp*>::iterator lIter;
	                for (lIter = t->wcRecvs.begin(); lIter != t->wcRecvs.end(); lIter++)
	                {
	                    if (*lIter) (*lIter)->logAsLost(qIter->first);
	                }
	    }//for comms
	}//for processes

#ifdef MUST_DEBUG
	//Print the match histories
	printMatchHistories();
	clearMatchHistory(&myMatches);
	std::cout << "DP2PMatch: printed match histories for each comm in files named \"must_match_history_comm_%d.dot\" use DOT to visualize them." << std::endl;
#endif

#ifdef MUST_MATCH_DEBUG
    std::cout << "DP2PMatch: Final configuration of the queues" << std::endl;
    printQs ();
#endif

	//Do a graceful free of all remaining data
	//==Free other data
	//Matching structure
	clearQ (&myQs);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// findMatchingRecv
//=============================
bool DP2PMatch::findMatchingRecv (
        DP2POp* op,
		bool *pOutNeedsToSuspend,
		DP2POp** pOutReason)
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
		std::list<DP2POp*>::iterator recv;
		for (recv = r->second.begin(); recv != r->second.end(); recv++)
		{
			if (op->matchTags(*recv))
			{
				//!!! HIT, remove the recv
#ifdef MUST_DEBUG
			    addMatchToHistory (op, *recv);
#endif
			    if (myListener) myListener->notifyP2PRecvMatch((*recv)->getPId(), (*recv)->getLTimeStamp(),op->getPId(),op->getLTimeStamp());
			    op->matchTypes (*recv);
			    DP2POp *other = *recv;
			    r->second.erase(recv);

			    //Also update flood control queue size
			    myQSize--;
			    myFloodControl->modifyQueueSize(
			            mySuspension[other->getIssuerRank()].direction,
			            mySuspension[other->getIssuerRank()].channel,
			            -1);

			    delete (other);
				return true;
			}
		}
	}

	//==4) Search the wcRecvs
	std::list<DP2POp*>::iterator recv;
	for (recv = t->second.wcRecvs.begin(); recv != t->second.wcRecvs.end(); recv++)
	{
	    DP2POp* other = *recv;
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
                    if (myListener) myListener->notifyP2PRecvMatch((*recv)->getPId(), (*recv)->getLTimeStamp(),op->getPId(),op->getLTimeStamp());
                    op->matchTypes (*recv);
				    r->second.erase(recv);

				    //Also update flood control queue size
				    myQSize--;
				    myFloodControl->modifyQueueSize(
				            mySuspension[other->getIssuerRank()].direction,
				            mySuspension[other->getIssuerRank()].channel,
				            -1);

				    delete (other);
					return true;
				}
				else
				{
				    //Hit, but we need to suspend!!!
				    //Suspend the op that we querry for
					if (pOutReason)
					    *pOutReason = other;

					if (pOutNeedsToSuspend)
						*pOutNeedsToSuspend = true;

					//Mark the wc-recv issuing rank as suspended
					mySuspension[other->getIssuerRank()].addReason(other);
					mySuspension[other->getIssuerRank()].isSuspended = true;

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
void DP2PMatch::addOutstandingSend (
        DP2POp* op)
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
		t->second.sends.insert (std::make_pair (op->getToRank(), std::list<DP2POp*>()));
		r = t->second.sends.find(op->getToRank());
	}

	//==4) Add to queue for the position in sends map
	r->second.push_back(op);

	//==5) Update queue size for flood control
	myQSize++;
	if (myQSize > myMaxQSize)
	    myMaxQSize = myQSize;
	myFloodControl->modifyQueueSize(
	        mySuspension[op->getIssuerRank()].direction,
	        mySuspension[op->getIssuerRank()].channel,
	        +1);
}

//=============================
// findMatchingSend
//=============================
bool DP2PMatch::findMatchingSend (
        DP2POp* op,
		bool *pOutNeedsToSuspend,
		DP2POp** pOutReason)
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
			std::list<DP2POp*>::iterator send;
			for (send = s->second.begin(); send != s->second.end(); send++)
			{
			    DP2POp* other = *send;
				if (op->matchTags(other))
				{
					if (op->getToRank() != myConsts->getAnySource())
					{
						//!!! HIT, remove the send
#ifdef MUST_DEBUG
					    addMatchToHistory (other, op);
#endif
					    if (myListener) myListener->notifyP2PRecvMatch(op->getPId(), op->getLTimeStamp(),other->getPId(),other->getLTimeStamp());
					    op->matchTypes (other);
					    s->second.erase(send);

					    //Also update flood control queue size
					    myQSize--;
					    myFloodControl->modifyQueueSize(
					            mySuspension[other->getIssuerRank()].direction,
					            mySuspension[other->getIssuerRank()].channel,
					            -1);

					    delete (other);

						return true;
					}
					else
					{
                        //!!! Potential hit, we go into suspension now
					    //Store information for current op to suspend itself
	                    if (pOutReason)
	                        *pOutReason = op;

						if (pOutNeedsToSuspend)
							*pOutNeedsToSuspend = true;

						//Add the send as a suspension reason
						mySuspension[other->getIssuerRank()].addReason(op);
						mySuspension[other->getIssuerRank()].isSuspended = true;

						break; //Abort for this rank, but go on with the next one
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
void DP2PMatch::addOutstandingRecv (
        DP2POp* op)
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
	std::list<DP2POp*> *list;

	if (op->getToRank() != myConsts->getAnySource() && t->second.wcRecvs.empty())
	{
		RankOutstandings::iterator r;
		r = t->second.recvs.find(op->getToRank());
		if (r == t->second.recvs.end())
		{
			t->second.recvs.insert (std::make_pair (op->getToRank(), std::list<DP2POp*>()));
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

	//==5) Update queue size for flood control
	myQSize++;
	if (myQSize > myMaxQSize)
	    myMaxQSize = myQSize;
	myFloodControl->modifyQueueSize(
	        mySuspension[op->getIssuerRank()].direction,
	        mySuspension[op->getIssuerRank()].channel,
	        +1);
}

//=============================
// findRecvForUpdate
//=============================
void DP2PMatch::findRecvForUpdate (
		int from,
		int newSource,
		bool hasRequest,
		MustRequestType request,
		std::list<int>* pOutReopenedRanks)
{
	//==1) Search in the tables
	QT::iterator qIter = myQs.find(from);
	if (qIter != myQs.end())
	{
		ProcessQueues::iterator cIter;
		for (cIter = qIter->second.begin(); cIter != qIter->second.end(); cIter++)
		{
			ProcessTable *table = &(cIter->second);

			std::list<DP2POp*>::iterator rIter;
			for (rIter = table->wcRecvs.begin(); rIter != table->wcRecvs.end(); rIter++)
			{
				DP2POp* entry = *rIter;

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

				//Remove this suspension reason if necessary
				if (mySuspension[entry->getIssuerRank()].isSuspended)
				{
				    //Remove the reason from this rank and any other, get any reopened rank with non empty queue
				    removeReasonFromRank (entry->getIssuerRank(), entry, pOutReopenedRanks);
				}

				//Add to the right recv queue
				RankOutstandings::iterator toRecv = table->recvs.find(newSourceTranslated);
				if (toRecv == table->recvs.end())
				{
					table->recvs.insert(std::make_pair(newSourceTranslated, std::list<DP2POp*>()));
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
						table->recvs.insert(std::make_pair((*rIter)->getToRank(), std::list<DP2POp*>()));
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
	std::list<DP2POp*>::iterator susIter;

	for (susIter = mySuspension[from].queue.begin(); susIter != mySuspension[from].queue.end(); susIter++)
	{
		DP2POp* wc = *susIter;

		//Is it a wildcard receive at all?
		if (wc->isSend())
		    continue;
		if (wc->getToRank() != myConsts->getAnySource())
		    continue;

		//Request property must match
		if (wc->hasRequest() != hasRequest)
			continue;

		//If there is an associated request it must match
		if (hasRequest && wc->getRequest() != request)
			continue;

		//Hit! Update ! (No translation needed, it was not yet processed and will be translated at that time)
		wc->updateToSource(translateDestSource (wc->getComm(), newSource));

		//Remove this suspension reason if necessary
		if (mySuspension[from].isSuspended)
		{
		    //Remove the reason from this rank and any other, get any reopened rank with non empty queue
		    removeReasonFromRank (from, wc, pOutReopenedRanks);
		}

		break;
	}
}

//=============================
// printQs
//=============================
void DP2PMatch::printQs (void)
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

				std::list<DP2POp*>::iterator lIter;
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

				std::list<DP2POp*>::iterator lIter;
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
			std::list<DP2POp*>::iterator lIter;
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
	std::map<int, SuspensionInfo>::iterator susIter;
	for (susIter = mySuspension.begin(); susIter != mySuspension.end(); susIter++)
	{
	    std::cout << "[" << susIter->first << "] suspended="<<susIter->second.isSuspended << std::endl;

	    std::list<DP2POp*>::iterator qIter;
	    for (qIter = susIter->second.queue.begin(); qIter != susIter->second.queue.end(); qIter++)
	    {
	        std::cout << "                ";
	        if(*qIter) (*qIter)->print(std::cout);
	        std::cout << std::endl;
	    }
	}
}

//=============================
// suspendOp
//=============================
void DP2PMatch::suspendOp (DP2POp* opToSuspend, DP2POp* suspensionReason)
{
    SuspensionInfo &ref = mySuspension[opToSuspend->getIssuerRank()];

    if (!ref.isSuspended)
    {
        ref.isSuspended = true;
    }

    if (suspensionReason)
        ref.addReason(suspensionReason);

    if (!myIsInProcessQueues)
    {
        /*We only add if we are not processing queues*/
        ref.queue.push_back(opToSuspend);

        /*Also update flood control queue size*/
        myQSize++;
        if (myQSize > myMaxQSize)
            myMaxQSize = myQSize;
        myFloodControl->modifyQueueSize(
                ref.direction,
                ref.channel,
                +1);
    }
}

//=============================
// addMatchToHistory
//=============================
#ifdef MUST_DEBUG
void DP2PMatch::addMatchToHistory (DP2POp* send, DP2POp* recv)
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
void DP2PMatch::printMatchHistories (void)
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
        stream << "must_match_history_comm_" << myPlaceId << "_" << d << ".dot";
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
void DP2PMatch::clearMatchHistory(AllMatchHistories *history)
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
// clearQ
//=============================
void DP2PMatch::clearQ (QT* queue)
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
            std::list<DP2POp*>::iterator opIter;

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
// removeReasonFromRank
//=============================
void DP2PMatch::removeReasonFromRank (int rank, DP2POp *reason, std::list<int>* pOutReopenedRanks)
{
    //Was this a reason for this task
    bool wasAReason = mySuspension[rank].removeReason(reason);

    //If it was a reason for this task it may also be a reason for others
    if (wasAReason)
    {
        //Do we need to add this rank to the list of reopened ranks?
        if (!mySuspension[rank].suspensionReason)
        {
            mySuspension[rank].isSuspended = false;

            if (!mySuspension[rank].queue.empty())
                pOutReopenedRanks->push_back (rank);
        }

        //Reopen any other rank that also uses this reason
        std::map<int, SuspensionInfo>::iterator susInfoIter;
        for (susInfoIter = mySuspension.begin(); susInfoIter != mySuspension.end(); susInfoIter++)
        {
            SuspensionInfo &ref = susInfoIter->second;
            if (ref.removeReason(reason))
            {
                if (!ref.suspensionReason)
                {
                    ref.isSuspended = false;

                    if (!ref.queue.empty())
                        pOutReopenedRanks->push_back(susInfoIter->first);
                }
            }
        }//For all other ranks
    }//If was a reason for the original rank
}

//=============================
// processQueuedEvents
//=============================
void DP2PMatch::processQueuedEvents (std::list<int>* ranksToProcess)
{
    myIsInProcessQueues = true;

    std::list<int>::iterator iter;
    for (iter = ranksToProcess->begin(); iter != ranksToProcess->end(); iter++)
    {
        int rank = *iter;

        while (!mySuspension[rank].queue.empty())
        {
            if (mySuspension[rank].isSuspended)
                break;

            DP2POp* op = mySuspension[rank].queue.front();

            PROCESSING_RETURN ret = op->process(rank);

            if (ret == PROCESSING_REEXECUTE)
            {
                //Was suspended
                //due to myIsInProcessQueues == true, we did not enqueue the op again, so everything stays as is and
                //we simply continue with the next rank
                break;
            }
            else if (ret == PROCESSING_SUCCESS)
            {
                //Got rid of that one, lets remove it
                mySuspension[rank].queue.pop_front();
                //No need to free anything, the op frees itself if it is completed

                //Also update flood control queue size
                myQSize--;
                myFloodControl->modifyQueueSize(
                        mySuspension[rank].direction,
                        mySuspension[rank].channel,
                        -1);
            }
            else
            {
                std::cerr << "Internal Error when processing an operaiton! " << __FILE__ << ":" << __LINE__ << std::endl;
                assert (0);
                break;
            }
        }//while we have queued ops for the current ran
    }//While we have open ranks

    myIsInProcessQueues = false;
}

//=============================
// registerListener
//=============================
void DP2PMatch::registerListener (I_DP2PListener *listener)
{
    myListener = listener;
}

//=============================
// isWorldRankSuspended
//=============================
bool DP2PMatch::isWorldRankSuspended (int worldRank)
{
    std::map<int, SuspensionInfo>::iterator pos = mySuspension.find(worldRank);

    if (pos == mySuspension.end())
        return false;

    return pos->second.isSuspended;
}

//=============================
// notifySendActivated
//=============================
void DP2PMatch::notifySendActivated (int rank, MustLTimeStamp ts)
{
    std::map<std::pair<int, MustLTimeStamp>, PassSendInfo >::iterator pos;
    pos = mySendsToTransfer.find(std::make_pair(rank,ts));

    //Is it a meaningful send? (one to a remote place)
    if (pos == mySendsToTransfer.end())
        return;

    if (!pos->second.isPersistent)
    {
        //Forward associated resources first
        myCTrack->passCommAcross(rank, pos->second.cInfo, pos->second.targetPlace, &(pos->second.comm));
        myDTrack->passDatatypeAcross(rank, pos->second.dInfo, pos->second.targetPlace, &(pos->second.type));
    }

    //do the passSend
    if (!pos->second.hasRequest)
    {
        if (myPassSendFunction)
            (*myPassSendFunction) (
                    pos->second.pId,
                    pos->second.lId,
                    pos->second.destIn,
                    pos->second.tag,
                    pos->second.comm,
                    pos->second.type,
                    pos->second.count,
                    pos->second.mode,
                    ts,
                    pos->second.targetPlace);
    }
    else if (!pos->second.isPersistent)
    {
        if (myPassIsendFunction)
            (*myPassIsendFunction) (
                    pos->second.pId,
                    pos->second.lId,
                    pos->second.destIn,
                    pos->second.tag,
                    pos->second.comm,
                    pos->second.type,
                    pos->second.count,
                    pos->second.mode,
                    pos->second.request,
                    ts,
                    pos->second.targetPlace);
    } else
    {
        if (myPassSendStartFunction)
            (*myPassSendStartFunction) (
                    pos->second.pId,
                    pos->second.lId,
                    pos->second.request,
                    ts,
                    pos->second.targetPlace);
    }

    //Free persistent handles now that we forwarded the send finally
    if (pos->second.cInfo)
        (pos->second.cInfo)->erase();
    if (pos->second.dInfo)
        (pos->second.dInfo)->erase();

    //Remove from interesting sends
    mySendsToTransfer.erase(pos);
}

/*EOF*/
