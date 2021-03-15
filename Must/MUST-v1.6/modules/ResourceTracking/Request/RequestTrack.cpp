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
 * @file RequestTrack.cpp
 *       @see MUST::RequestTrack.
 *
 *  @date 24.01.2010
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "RequestTrack.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(RequestTrack)
mFREE_INSTANCE_FUNCTION(RequestTrack)
mPNMPI_REGISTRATIONPOINT_FUNCTION(RequestTrack)

//=============================
// Constructor
//=============================
RequestTrack::RequestTrack (const char* instanceName)
	: TrackBase<Request, I_Request, MustRequestType, MustMpiRequestPredefined, RequestTrack, I_RequestTrack> (instanceName)
{
    //Get the DatatypeTrack and CommTrack modules
    if (myFurtherMods.size () < 3)
    {
        std::cerr << "Error: the RequestTrack module needs the DatatypeTrack and CommTrack modules as childs, but at least one of them was not available." << std::endl;
        assert (0);
    }

    myDTrack = (I_DatatypeTrack*) myFurtherMods[0];
    myCTrack = (I_CommTrack*) myFurtherMods[1];
    myConsts = (I_BaseConstants*) myFurtherMods[2];

    //Initialize module data
    getWrapAcrossFunction("passRequestAcross",(GTI_Fct_t*) &myPassRequestAcrossFunc);
    getWrapAcrossFunction("passFreeRequestAcross",(GTI_Fct_t*) &myPassFreeAcrossFunc);
}

//=============================
// Destructor
//=============================
RequestTrack::~RequestTrack ()
{
    //Notify HandleInfoBase of ongoing shutdown
    HandleInfoBase::disableFreeForwardingAcross();
    myDTrack->notifyOfShutdown();
    myCTrack->notifyOfShutdown();
}

//=============================
// createPersistentSend
//=============================
GTI_ANALYSIS_RETURN RequestTrack::createPersistentSend (
		MustParallelId pId,
		MustLocationId lId,
		int count,
		MustDatatypeType datatype,
		int dest,
		int tag,
		MustCommType comm,
		int sendMode,
		MustRequestType request)
{
	//==1) Is this request already a known request
	if (isAlreadyKnown(pId, request))
		return GTI_ANALYSIS_FAILURE;

	//==2)  Create the full information record
	Request* info = new Request ();
	info->myIsActive = false;
	info->myIsPersistent = true;
	info->myIsSend = true;
	info->myIsCanceled = false;
	info->myIsNull = false;
	info->myKind = MUST_REQUEST_P2P;

	//persistent info, see MustPersistentSendInfo, MustPersistentRecvInfo
	info->myCount = count;
	info->myDatatype = myDTrack->getPersistentDatatype(pId, datatype);
	info->myDestSource = dest;
	info->myTag = tag;
	info->myComm = myCTrack->getPersistentComm(pId, comm);
	info->mySendMode = (MustSendMode) sendMode;

	//Usage history, see MustRequestHistory
	info->myCreationPId = pId;
	info->myCreationLId = lId;

	//Mark if MPI_PROC_NULL
	if (myConsts->getProcNull() == info->myDestSource)
	    info->myIsProcNull = true;
	else
	    info->myIsProcNull = false;

	//==3) Add the full information to myUserHandles
	submitUserHandle (pId, request, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// createPersistentRecv
//=============================
GTI_ANALYSIS_RETURN RequestTrack::createPersistentRecv (
		MustParallelId pId,
		MustLocationId lId,
		int count,
		MustDatatypeType datatype,
		int source,
		int tag,
		MustCommType comm,
		MustRequestType request)
{
	//==1) Is this request already a known request
	if (isAlreadyKnown(pId, request))
		return GTI_ANALYSIS_FAILURE;

	//==2)  Create the full information record
	Request* info = new Request ();
	info->myIsActive = false;
	info->myIsPersistent = true;
	info->myIsSend = false;
	info->myIsCanceled = false;
	info->myIsNull = false;
	info->myKind = MUST_REQUEST_P2P;

	//persistent info, see MustPersistentSendInfo, MustPersistentRecvInfo
	info->myCount = count;
	info->myDatatype = myDTrack->getPersistentDatatype(pId, datatype);
	info->myDestSource = source;
	info->myTag = tag;
	info->myComm = myCTrack->getPersistentComm(pId, comm);

	//Usage history, see MustRequestHistory
	info->myCreationPId = pId;
	info->myCreationLId = lId;

	//Mark if MPI_PROC_NULL
	if (myConsts->getProcNull() == info->myDestSource)
	    info->myIsProcNull = true;
	else
	    info->myIsProcNull = false;

	//==3) Add the full information to myUserHandles
	submitUserHandle (pId, request, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// cancel
//=============================
GTI_ANALYSIS_RETURN RequestTrack::cancel (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	Request* info = getHandleInfo (pId, request);
	if (!info || info->isNull()) return GTI_ANALYSIS_SUCCESS;

	//== Possible errors:
	//1) not known (handled above with return)
	//2) is MPI_REQUEST_NULL (handled above with return)
	//3) is not active
	if (!info->isActive())
		return GTI_ANALYSIS_SUCCESS;
	//4) is already canceled (does not hurts for tracking)

	//== Save the cancel
	info->myIsCanceled = true;
	info->myCancelLId = lId;
	info->myCancelPId = pId;

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addActive
//=============================
GTI_ANALYSIS_RETURN RequestTrack::addActive (
		MustParallelId pId,
		MustLocationId lId,
		int isSend,
		MustRequestType request,
        int destSource)
{
    //==1) Verify that this request is not already in our map
    Request* reqInfo = getHandleInfo (pId, request);

    if (reqInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do
        //For user handles, inc the MPI ref count
        if (!reqInfo->isNull())
        {
            reqInfo->mpiIncRefCount();
            reqInfo->myIsActive = true;
        }

        return GTI_ANALYSIS_SUCCESS;
    }

	//==2) Create the full info
	Request* info = new Request ();
	info->myIsActive = true;
	info->myIsPersistent = false;
	info->myIsSend = (bool)isSend;
	info->myIsCanceled = false;
	info->myIsNull = false;
	info->myKind = MUST_REQUEST_P2P;

	//==2b) Mark if MPI_PROC_NULL
	if (myConsts->getProcNull() == destSource)
	    info->myIsProcNull = true;
	else
	    info->myIsProcNull = false;

	//Usage history, see MustRequestHistory
	//for non-persistent requests, the activation call creates the request
	info->myCreationPId = info->myActivationPId = pId;
	info->myCreationLId = info->myActivationLId = lId;

	//==3) Add the full information to myUserHandles
	submitUserHandle (pId, request, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addActiveCollective
//=============================
GTI_ANALYSIS_RETURN RequestTrack::addActiveCollective (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    //==1) Verify that this request is not already in our map
    Request* reqInfo = getHandleInfo (pId, request);

    if (reqInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do
        //For user handles, inc the MPI ref count
        if (!reqInfo->isNull())
        {
            reqInfo->mpiIncRefCount();
            reqInfo->myIsActive = true;
        }

        return GTI_ANALYSIS_SUCCESS;
    }

    //==2) Create the full info
    Request* info = new Request ();
    info->myIsActive = true;
    info->myIsPersistent = false;
    info->myIsSend = false;
    info->myIsCanceled = false;
    info->myIsNull = false;
    info->myKind = MUST_REQUEST_COLL;
    info->myIsProcNull = false;

    //Usage history, see MustRequestHistory
    //for non-persistent requests, the activation call creates the request
    info->myCreationPId = info->myActivationPId = pId;
    info->myCreationLId = info->myActivationLId = lId;

    //==3) Add the full information to myUserHandles
    submitUserHandle (pId, request, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// forceFree
//=============================
GTI_ANALYSIS_RETURN RequestTrack::forceFree (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	//==1) find the request
    Request* info = getHandleInfo (pId, request);
    if (!info || info->isNull()) return GTI_ANALYSIS_SUCCESS;

	//==2) Potential errors:
	//Is not known (already handled above)
	//Is MPI_REQUEST_NULL (already handled above)

	//==3) Remove the request
    removeUserHandle (pId, request);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// startPersistent
//=============================
GTI_ANALYSIS_RETURN RequestTrack::startPersistent (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	//==1) Find the request
    Request* info = getHandleInfo (pId, request);
    if (!info || info->isNull()) return GTI_ANALYSIS_SUCCESS;

	//==2) Potential errors:
	//Not known (handled above)
	//Is MPI_REQUEST_NULL (handled above)
	//Not persistent
	if (!info->myIsPersistent)
		return GTI_ANALYSIS_SUCCESS;
	//Still active
	if (info->myIsActive)
		return GTI_ANALYSIS_SUCCESS;
	//Canceled -> and not completed yet! (We reset canceled below)

	//==3) Handle the start
	info->myIsCanceled = false;
	info->myIsActive = true;
	info->myActivationPId = pId;
	info->myActivationLId = lId;

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// startPersistentArray
//=============================
GTI_ANALYSIS_RETURN RequestTrack::startPersistentArray (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType* requests,
		int count)
{
	//Loop over all requests, and call startPersistent
	for (int i = 0; i < count; i++)
	{
		if (startPersistent (pId, lId, requests[i]) == GTI_ANALYSIS_FAILURE)
			return GTI_ANALYSIS_FAILURE;
	}

	// Important: set isCanceled to false!
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// complete
//=============================
GTI_ANALYSIS_RETURN RequestTrack::complete (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request,
		int flag)
{
	//==1) Evaluate flag
	if (flag != 1)
		return GTI_ANALYSIS_SUCCESS;

	//==2) Find the request
	Request* info = getHandleInfo (pId, request);
	if (!info || info->isNull()) return GTI_ANALYSIS_SUCCESS;

	//==3) Potential errors:
	//Not known, MPI_REQUEST_NULL -> both handled above
	//Not active
	if (!info->myIsActive)
	{
	    //We should still free the beast
	    if (!info->myIsPersistent)
	        removeUserHandle (pId, request);

	    return GTI_ANALYSIS_SUCCESS;
	}


	//==4) Handle complete
	info->myIsActive = false;
	info->myIsCanceled = false;

	if (!info->myIsPersistent)
	{
	    removeUserHandle (pId, request);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completeAny
//=============================
GTI_ANALYSIS_RETURN RequestTrack::completeAny (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType* requests,
			int count,
			int index,
			int flag)
{
	//==1) Evaluate flag
	if (flag != 1)
		return GTI_ANALYSIS_SUCCESS;

	//==2) Check index
	if (index < 0 || index >= count)
		return GTI_ANALYSIS_SUCCESS;

	//==3) Propagate to regular complete
	return complete (pId, lId, requests[index], flag);
}

//=============================
// completeArray
//=============================
GTI_ANALYSIS_RETURN RequestTrack::completeArray (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType *requests,
		int count,
		int flag)
{
	if (!flag)
		return GTI_ANALYSIS_SUCCESS;

	//Loop over all requests
	for (int i = 0; i < count; i++)
	{
		if (complete (pId, lId, requests[i], flag) != GTI_ANALYSIS_SUCCESS)
			return GTI_ANALYSIS_SUCCESS;
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completeSome
//=============================
GTI_ANALYSIS_RETURN RequestTrack::completeSome (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType *requests,
		int count,
		int *indices,
		int numIndices)
{
	//Loop over all indices
	for (int i = 0; i < numIndices; i++)
	{
		if (indices[i] >= count)
		{
			std::cerr << "Error: an index in completeSome (" << __FILE__<< "@" << __LINE__ << ") is larger outside the array of indices (indices[" << i << "]=" << indices[i] << ", count=" << count << "). This is an error in the MPI or MUST implementation." << std::endl;
			return GTI_ANALYSIS_FAILURE;
		}

		if (complete (pId, lId, requests[indices[i]], true) != GTI_ANALYSIS_SUCCESS)
			return GTI_ANALYSIS_FAILURE;
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteRequest
//=============================
GTI_ANALYSIS_RETURN RequestTrack::addRemoteRequest (
        int rank,
        int hasHandle,
        MustRequestType requestHandle,
        MustRemoteIdType remoteId,
        int isActive,
        int isPersistent,
        int isSend,
        int isNull,
        int isCanceled,
        int isProcNull,
        int count,
        MustRemoteIdType datatype,
        int tag,
        MustRemoteIdType comm,
        int destSource,
        int sendMode,
        MustParallelId creationPId,
        MustParallelId activationPId,
        MustParallelId cancelPId,
        MustLocationId creationLId,
        MustLocationId activationLId,
        MustLocationId cancelLId)
{
    //Create the resource
    Request* resource = new Request ();

    resource->myIsActive = (bool) isActive;
    resource->myIsPersistent = (bool) isPersistent;
    resource->myIsSend = (bool) isSend;
    resource->myIsNull = (bool) isNull;
    resource->myIsCanceled = (bool) isCanceled;
    resource->myIsProcNull = (bool) isProcNull;
    resource->myKind = MUST_REQUEST_P2P;

    resource->myCount = count;

    if (datatype)
        resource->myDatatype = myDTrack->getPersistentRemoteDatatype (rank, datatype);
    else
        resource->myDatatype = NULL;

    resource->myTag = tag;

    if (comm)
        resource->myComm = myCTrack->getPersistentRemoteComm (rank, comm);
    else
        resource->myComm = NULL;

    resource->myDestSource = destSource;
    resource->mySendMode = (MustSendMode) sendMode;
    resource->myCreationPId = creationPId;
    resource->myCreationLId = creationLId;
    resource->myActivationPId = activationPId;
    resource->myActivationLId = activationLId;
    resource->myCancelPId = cancelPId;
    resource->myCancelLId = cancelLId;

    //Register the new remote comm
    submitRemoteResource(rank, remoteId, hasHandle, requestHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// freeRemoteRequest
//=============================
GTI_ANALYSIS_RETURN RequestTrack::freeRemoteRequest (
        int rank,
        MustRemoteIdType remoteId)
{
    removeRemoteResource(rank, remoteId);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getRequest
//=============================
I_Request* RequestTrack::getRequest (
		MustParallelId pId,
		MustRequestType request)
{
	return getRequest (pId2Rank(pId), request);
}

//=============================
// getRequest
//=============================
I_Request* RequestTrack::getRequest (
		int rank,
		MustRequestType request)
{
	return getHandleInfo (rank, request);
}

//=============================
// getPersistentRequest
//=============================
I_RequestPersistent* RequestTrack::getPersistentRequest (
        MustParallelId pId,
        MustRequestType request)
{
    return getPersistentRequest (pId2Rank(pId), request);
}

//=============================
// getPersistentRequest
//=============================
I_RequestPersistent* RequestTrack::getPersistentRequest (
        int rank,
        MustRequestType request)
{
    Request* ret = getHandleInfo (rank, request);
    if (ret) ret->incRefCount();
    return ret;
}

//=============================
// createPredefinedInfo
//=============================
Request* RequestTrack::createPredefinedInfo (int value, MustRequestType handle)
{
    if (handle == myNullValue)
        return new Request ();
    return NULL;//There should not be any other cases
}

//=============================
// passRequestAcross
//=============================
bool RequestTrack::passRequestAcross (
        MustParallelId pId,
        MustRequestType request,
        int toPlaceId)
{
    return passRequestAcross (pId2Rank (pId), request, toPlaceId);
}

//=============================
// passRequestAcross
//=============================
bool RequestTrack::passRequestAcross (
        int rank,
        MustRequestType requestHandle,
        int toPlaceId)
{
    if (!myPassRequestAcrossFunc || !myPassFreeAcrossFunc)
        return false;

    //Get comm
    Request* req = getHandleInfo (rank, requestHandle);

    if (!req)
        return false;

    //Did we already pass this request?
    if (req->wasForwardedToPlace(toPlaceId, rank))
        return true;

    //Pass base resources of the request
    if (!req->isNull() && req->isPersistent())
        myLIdMod->passLocationToPlace(req->myCreationPId, req->myCreationLId, toPlaceId);

    if (!req->isNull() && req->isActive())
        myLIdMod->passLocationToPlace(req->myActivationPId, req->myActivationLId, toPlaceId);

    if (!req->isNull() && req->isCanceled())
        myLIdMod->passLocationToPlace(req->myCancelPId, req->myCancelLId, toPlaceId);

    MustRemoteIdType   commRemoteId = 0,
                                    datatypeRemoteId = 0;

    if (req->myDatatype)
        myDTrack->passDatatypeAcross(rank, req->myDatatype, toPlaceId, &datatypeRemoteId);

    if (req->myComm)
        myCTrack->passCommAcross(rank, req->myComm, toPlaceId, &commRemoteId);

    //Pass the actuall request across
    (*myPassRequestAcrossFunc) (
            rank,
            (int) true,
            requestHandle,
            req->getRemoteId(),
            (int)req->myIsActive,
            (int)req->myIsPersistent,
            (int)req->myIsSend,
            (int)req->myIsNull,
            (int)req->myIsCanceled,
            (int)req->myIsProcNull,
            req->myCount,
            datatypeRemoteId,
            req->myTag,
            commRemoteId,
            req->myDestSource,
            (int)req->mySendMode,
            req->myCreationPId,
            req->myActivationPId,
            req->myCancelPId,
            req->myCreationLId,
            req->myActivationLId,
            req->myCancelLId,
            toPlaceId
        );

    //Tell the comm that we passed it across
    req->setForwardedToPlace(toPlaceId, rank, myPassFreeAcrossFunc);

    return true;
}

/*EOF*/
