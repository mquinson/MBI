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
 * @file I_WcUpdate.h
 *       @see I_WcUpdate.
 *
 *  @date 15.03.2011
 *  @author Tobias Hilbrich, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_RequestTrack.h"

#ifndef I_WCUPDATE_H
#define I_WCUPDATE_H

/**
 * Interface for providing wildcard receive updates
 * to the lost message detector (I_LostMessage.h).
 *
 * Dependencies (order as listed):
 * - RequestTrack
 */
class I_WcUpdate : public gti::I_Module
{
public:

	/**
	 * Passes the value of MPI_ANY_SOURCE to the module.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
			int anySource) = 0;

	/**
	 * Called in the post of completed receives.
	 * If it was a wc receive it creates a message
	 * with a wrapp-everywhere call to inform the
	 * lost message tool.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param source of the receive.
	 * @param statusSource source in the MPI_Status argument.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN recvPost (
    		MustParallelId pId,
    		MustLocationId lId,
    		int source,
    		int statusSource) = 0;

    /**
    	 * Notification of a new MPI_Irecv.
    	 *
    	 * @param pId parallel id of the call site.
    	 * @param lId location id of the call site.
    	 * @param source of the irecv.
    	 * @param request to add.
    	 * @return @see gti::GTI_ANALYSIS_RETURN.
    	 */
    	virtual gti::GTI_ANALYSIS_RETURN irecv (
    			MustParallelId pId,
    			MustLocationId lId,
    			int source,
    			MustRequestType request) = 0;

    	/**
    	 * Notification of an MPI_Start.
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
    	 * Notification of an MPI_Startall.
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
    	 * Notification of the completion of a single request.
    	 *
    	 * @param pId parallel id of the call site.
    	 * @param lId location id of the call site.
    	 * @param request that was completed.
    	 * @param flag 1 if the request was completed, 0 otherwise.
    	 * @param statusSource
    	 * @param statusSource source in the MPI_Status argument.
    	 * @return @see gti::GTI_ANALYSIS_RETURN.
    	 */
    	virtual gti::GTI_ANALYSIS_RETURN complete (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType request,
    			int flag,
    			int statusSource) = 0;

    	/**
    	 * Completion where one request out of an array was completed.
    	 *
    	 * @param pId parallel id of the call site.
    	 * @param lId location id of the call site.
    	 * @param requests to complete one of.
    	 * @param count number of requests in array.
    	 * @param index of request to complete.
    	 * @param flag 1 if the request was completed, 0 otherwise.
    	 * @param statusSource source of the status argument.
    	 * @return @see gti::GTI_ANALYSIS_RETURN.
    	 */
    	virtual gti::GTI_ANALYSIS_RETURN completeAny (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType* requests,
    			int count,
    			int index,
    			int flag,
    			int statusSource) = 0;

    	/**
    	 * Completion of an array of requests.
    	 *
    	 * @param pId parallel id of the call site.
    	 * @param lId location id of the call site.
    	 * @param requests to complete.
    	 * @param count number of requests to complete.
    	 * @param flag 1 if the requests were completed, 0 otherwise.
    	 * @param statusSources sources array from the MPI_Status array.
    	 * @return @see gti::GTI_ANALYSIS_RETURN.
    	 */
    	virtual gti::GTI_ANALYSIS_RETURN completeArray (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType *requests,
    			int count,
    			int flag,
    			int* statusSources) = 0;

    	/**
    	 * Completion where some requests out of an array where completed.
    	 *
    	 * @param pId parallel id of the call site.
    	 * @param lId location id of the call site.
    	 * @param requests to complete.
    	 * @param count number of requests.
    	 * @param indices indices of completed requests.
    	 * @param numIndices size of the indices array.
    	 * @param statusSources sources array from the MPI_Status array.
    	 * @return @see gti::GTI_ANALYSIS_RETURN.
    	 */
    	virtual gti::GTI_ANALYSIS_RETURN completeSome (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType *requests,
    			int count,
    			int *indices,
    			int numIndices,
    			int* statusSources) = 0;
};/*class I_WcUpdate*/

#endif /*I_WCUPDATE_H*/
