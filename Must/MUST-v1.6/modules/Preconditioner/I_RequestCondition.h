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
 * @file I_RequestCondition.h
 *       @see I_RequestCondition.
 *
 *  @date 06.06.2011
 *  @author Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#include "MustTypes.h"

#ifndef I_REQUESTCONDITION_H
#define I_REQUESTCONDITION_H

/**
 * Interface for providing wildcard receive updates
 * to the lost message detector (I_LostMessage.h).
 *
 * Dependencies (order as listed):
 * - none
 */
class I_RequestCondition : public gti::I_Module
{
public:

	/**
	 * Passes the value of MPI_ANY_SOURCE to the module.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
			int anySource) = 0;


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
};/*class I_RequestCondition*/

#endif /*I_REQUESTCONDITION_H*/
