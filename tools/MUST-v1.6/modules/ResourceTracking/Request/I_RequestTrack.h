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
 * @file I_RequestTrack.h
 *       @see I_RequestTrack.
 *
 *  @date 21.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#include "I_Request.h"
#include "I_TrackBase.h"

#include <list>

#ifndef I_REQUESTTRACK_H
#define I_REQUESTTRACK_H

/**
 * Interface for request tracking analysis module.
 *
 * Important: This analysis module only tracks requests,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies:
 *  - ParallelIdAnalysis
 *  - LocationAnalysis
 *  - DatatypeTrack
 *  - CommTrack
 *  - BaseConstants
 */
class I_RequestTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Request>
{
public:
	/**
	 * Adds a persistent send request.
	 *
	 * @param pId parallel id of the callsite.
	 * @param lId location id of the callsite.
	 * @param count send count.
	 * @param datatype send type.
	 * @param dest send destination.
	 * @param tag send tag.
	 * @param comm send communicator.
	 * @param sendMode one out of  must::MustSendMode.
	 * @param request to add.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN createPersistentSend (
			MustParallelId pId,
			MustLocationId lId,
			int count,
			MustDatatypeType datatype,
			int dest,
			int tag,
			MustCommType comm,
			int sendMode,
			MustRequestType request) = 0;

	/**
	 * Adds a persistent recv request.
	 *
	 * @param pId parallel id of the callsite.
	 * @param lId location id of the callsite.
	 * @param count send count.
	 * @param datatype send type.
	 * @param source receive source.
	 * @param tag send tag.
	 * @param comm send communicator.
	 * @param request to add.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN createPersistentRecv (
			MustParallelId pId,
			MustLocationId lId,
			int count,
			MustDatatypeType datatype,
			int source,
			int tag,
			MustCommType comm,
			MustRequestType request) = 0;

	/**
	 * Marks the given request for cancellation.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param request to cancel.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN cancel (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType request) = 0;

	/**
	 * Adds a new point-to-point request in the active non persistent state.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param isSend, 1 if this is a send request, 0 otherwise (it is a receive then).
	 * @param request to add.
	 * @param destSource of the isend/irecv, used to identify MPI_PROC_NULL and not activate these requests (P2PMatch discards them so this is a good choice).
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addActive (
			MustParallelId pId,
			MustLocationId lId,
			int isSend,
			MustRequestType request,
			int destSource) = 0;

	/**
	 * Adds a new collective request in the active non persistent state.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param request to add.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addActiveCollective (
	        MustParallelId pId,
	        MustLocationId lId,
	        MustRequestType request) = 0;

	/**
	 * Frees a request without completing it.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param request to free.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN forceFree (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType request) = 0;

	/**
	 * Starts a persistent point-to-point request, transition from
	 * persistent inactive to persistent active state.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param request to start.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN startPersistent (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType request) = 0;

	/**
	 * Starts an array of persistent point-to-point requests, transitions them from
	 * persistent inactive to persistent active state.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param requests to start.
	 * @param count number of requests in array.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN startPersistentArray (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType* requests,
			int count) = 0;

	/**
	 * Completes a request, performs:
	 * - non persistent active -> invalid
	 * - persistent active -> persistent inactive
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param request to complete.
	 * @param flag 1 if the request was completed, 0 otherwise.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN complete (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType request,
			int flag) = 0;

	/**
	 * Completes a request, performs:
	 * - non persistent active -> invalid
	 * - persistent active -> persistent inactive
	 * The request is selected out of an array, based on an index.
	 *
	 * @todo performance wise this is a very bad function, rather should
	 *            MPI_Testany and MPI_Testsome be mapped to the regular complete.
	 *            This would be possible with a pre and a post operation, the pre operation
	 *            would map all requests in the array to an int, the post operation would
	 *            select the right integer based on the index. However, this is currently
	 *            not possible as operations are not allowed to be input to operations.
	 *            So this should motivate a future GTI extension.
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param requests to complete one of.
	 * @param count number of requests in array.
	 * @param index of request to complete.
	 * @param flag 1 if the request was completed, 0 otherwise.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN completeAny (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType* requests,
			int count,
			int index,
			int flag) = 0;

	/**
	 * Completes an array of requests, performs the following
	 * for each request:
	 * - non persistent active -> invalid
	 * - persistent active -> persistent inactive
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param requests to complete.
	 * @param count number of requests to complete.
	 * @param flag 1 if the requests were completed, 0 otherwise.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN completeArray (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType *requests,
			int count,
			int flag) = 0;

	/**
	 * Completes an some requests of an array, performs the following
	 * for each completed request:
	 * - non persistent active -> invalid
	 * - persistent active -> persistent inactive
	 *
	 * @param pId parallel id of the call site.
	 * @param lId location id of the call site.
	 * @param requests to complete.
	 * @param count number of requests.
	 * @param indices indices of completed requests.
	 * @param numIndices size of the indices array.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN completeSome (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType *requests,
			int count,
			int *indices,
			int numIndices) = 0;

	/**
	 * Receives a request from another place in the same layer.
	 * Param descriptions see must::Datatype.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addRemoteRequest (
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
	        MustLocationId cancelLId) = 0;

	/**
	 * Receives a free for a request that was received
	 * from another place in the same layer.
	 * @param rank context for the request to be freed.
	 * @param remoteId of the request to be freed.
	 */
	virtual gti::GTI_ANALYSIS_RETURN freeRemoteRequest (
	        int rank,
	        MustRemoteIdType remoteId) = 0;

	/**
	 * Returns pointer request information.
     * Is NULL if this is an unknown handle, note that
     * a MPI_REQUEST_NULL handle returns a valid pointer though.
     *
     * Memory must not be freed and is valid until I_RequestTrack
     * receives the next event, if you need the information longer
     * query getPersistentRequest instead.
     *
     * @param pId of the request context.
     * @param request to query for.
     * @return information for the given request or NULL.
	 */
	virtual must::I_Request* getRequest (
			MustParallelId pId,
			MustRequestType request) = 0;

	/** As I_RequestTrack::getRequest with rank instead of pid.*/
	virtual must::I_Request* getRequest (
			int rank,
			MustRequestType comm) = 0;

	/**
	 * Like I_RequestTrack::getRequest though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_RequestPersistent * requestInfo = myRequestTrack->getPersistentRequest (pId, handle);
     if (requestInfo == NULL) return;
     .... //Do something with requestInfo
     requestInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
     *
     * BEWARE: this has nothing to do with requests of the persistent type (e.g. MPI_Send_init->r),
     * a requests type can be queried from the inforamtion.
     *
     * @param pId of the request context.
     * @param request to query for.
     * @return information for the given request or NULL.
	 */
	virtual must::I_RequestPersistent* getPersistentRequest (
	        MustParallelId pId,
	        MustRequestType request) = 0;

	/** As I_RequestTrack::getPersistentRequest with rank instead of pid.*/
	virtual must::I_RequestPersistent* getPersistentRequest (
	        int rank,
	        MustRequestType comm) = 0;

	/**
	 * Adds the integer (MustRequestType) values for all predefined
	 * (named) handles for requests.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
            MustParallelId pId,
	        MustRequestType requestNull,
			int numPredefs,
			int* predefinedIds,
			MustRequestType* predefinedValues) = 0;

	/**
	 * Returns a list of all currently known user handles.
	 * Usage scenarios involve logging lost handles at finalize.
	 * @return a list of pairs of the form (rank, handle id).
	 */
	virtual std::list<std::pair<int, MustRequestType> > getUserHandles (void) = 0;

	/**
	 * Passes the given request to the given place on this tool level.
	 * @param pId context of the request to pass
	 * @param request to pass
	 * @param toPlaceId place to send to
	 * @return true iff successful.
	 *
	 * Reasons for this to fail include the unavailability of intra layer
	 * communication.
	 */
	virtual bool passRequestAcross (
	        MustParallelId pId,
	        MustRequestType request,
	        int toPlaceId) = 0;

	/**
	 * Like the other passRequestAcross version but with rank instead of a pId.
	 */
	virtual bool passRequestAcross (
	        int rank,
	        MustRequestType request,
	        int toPlaceId) = 0;

	/**
	 * Allows other modules to notify this module of an ongoing shutdown.
	 * This influcences the behavior of passing free calls across to other layers.
	 */
	virtual void notifyOfShutdown (void) = 0;
        virtual bool isPredefined(must::I_Request * type) {return false;}    
        
}; /*class I_RequestTrack*/

#endif /*I_REQUESTTRACK_H*/
