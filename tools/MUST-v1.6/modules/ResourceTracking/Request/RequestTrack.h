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
 * @file RequestTrack.h
 *       @see MUST::RequestTrack.
 *
 *  @date 24.01.2010
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_RequestTrack.h"
#include "TrackBase.h"
#include "Request.h"
#include "I_DatatypeTrack.h"
#include "I_CommTrack.h"
#include "I_BaseConstants.h"
#include "ResourceApi.h"

#ifndef REQUESTTRACK_H
#define REQUESTTRACK_H

using namespace gti;

namespace must
{
	/**
	 * Enumeration of all predefined requests (none).
	 */
	enum MustMpiRequestPredefined
	{
		MUST_MPI_REQUEST_UNKNOWN = 0
	};

	/**
     * Implementation of I_RequestTrack.
     */
    class RequestTrack : public TrackBase<Request, I_Request, MustRequestType, MustMpiRequestPredefined, RequestTrack, I_RequestTrack>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		RequestTrack (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~RequestTrack (void);

    		/**
    		 * @see I_RequestTrack::createPersistentSend
    		 */
    		GTI_ANALYSIS_RETURN createPersistentSend (
    				MustParallelId pId,
    				MustLocationId lId,
    				int count,
    				MustDatatypeType datatype,
    				int dest,
    				int tag,
    				MustCommType comm,
    				int sendMode,
    				MustRequestType request);

    		/**
    		 * @see I_RequestTrack::createPersistentRecv
    		 */
    		GTI_ANALYSIS_RETURN createPersistentRecv (
    				MustParallelId pId,
    				MustLocationId lId,
    				int count,
    				MustDatatypeType datatype,
    				int source,
    				int tag,
    				MustCommType comm,
    				MustRequestType request);

    		/**
    		 * @see I_RequestTrack::cancel
    		 */
    		GTI_ANALYSIS_RETURN cancel (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestTrack::addActive
    		 */
    		GTI_ANALYSIS_RETURN addActive (
    				MustParallelId pId,
    				MustLocationId lId,
    				int isSend,
    				MustRequestType request,
    	            int destSource);

    		/**
    		 * @see I_RequestTrack::addActiveCollective
    		 */
    		GTI_ANALYSIS_RETURN addActiveCollective (
    		        MustParallelId pId,
    		        MustLocationId lId,
    		        MustRequestType request);

    		/**
    		 * @see I_RequestTrack::forceFree
    		 */
    		GTI_ANALYSIS_RETURN forceFree (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestTrack::startPersistent
    		 */
    		GTI_ANALYSIS_RETURN startPersistent (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestTrack::startPersistentArray
    		 */
    		GTI_ANALYSIS_RETURN startPersistentArray (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType* requests,
    				int count);

    		/**
    		 * @see I_RequestTrack::complete
    		 */
    		GTI_ANALYSIS_RETURN complete (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request,
    				int flag);

    		/**
    		 * @see I_RequestTrack::completeAny
    		 */
    		GTI_ANALYSIS_RETURN completeAny (
    					MustParallelId pId,
    					MustLocationId lId,
    					MustRequestType* requests,
    					int count,
    					int index,
    					int flag);

    		/**
    		 * @see I_RequestTrack::completeArray
    		 */
    		GTI_ANALYSIS_RETURN completeArray (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType *requests,
    				int count,
    				int flag);

    		/**
    		 * @see I_RequestTrack::completeSome
    		 */
    		GTI_ANALYSIS_RETURN completeSome (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType *requests,
    				int count,
    				int *indices,
    				int numIndices);

    		/**
    		 * @see I_RequestTrack::addRemoteRequest
    		 */
    		GTI_ANALYSIS_RETURN addRemoteRequest (
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
    		        MustLocationId cancelLId);

    		/**
    		 * @see I_RequestTrack::freeRemoteRequest
    		 */
    		GTI_ANALYSIS_RETURN freeRemoteRequest (
    		        int rank,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_RequestTrack::getRequest
    		 */
    		I_Request* getRequest (
    		        MustParallelId pId,
    		        MustRequestType request);

    		/**
    		 * @see I_RequestTrack::getRequest
    		 */
    		I_Request* getRequest (
    		        int rank,
    		        MustRequestType comm);

    		/**
    		 * @see I_RequestTrack::getPersistentRequest
    		 */
    		I_RequestPersistent* getPersistentRequest (
    		        MustParallelId pId,
    		        MustRequestType request);

    		/**
    		 * @see I_RequestTrack::getPersistentRequest
    		 */
    		I_RequestPersistent* getPersistentRequest (
    		        int rank,
    		        MustRequestType comm);

    		/**
    		 * @see I_RequestTrack::passRequestAcross
    		 */
    		bool passRequestAcross (
    		        MustParallelId pId,
    		        MustRequestType request,
    		        int toPlaceId);

    		/**
    		 * @see I_RequestTrack::passRequestAcross
    		 */
    		bool passRequestAcross (
    		        int rank,
    		        MustRequestType request,
    		        int toPlaceId);

    protected:

    		I_DatatypeTrack* myDTrack;
    		I_CommTrack* myCTrack;
    		I_BaseConstants* myConsts;

    		//Function pointers for passing requests across
    		passFreeAcrossP myPassFreeAcrossFunc;
    		passRequestAcrossP myPassRequestAcrossFunc;

    		/**
    		 * Implementation of TrackBase::createPredefinedInfo.
    		 */
    		Request* createPredefinedInfo (int value, MustRequestType handle);

    }; /*class RequestTrack */
} /*namespace MUST*/

#endif /*RequestTrack_H*/
