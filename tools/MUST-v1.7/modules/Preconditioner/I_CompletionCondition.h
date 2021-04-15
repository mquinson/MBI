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
 * @file I_CompletionCondition.h
 *       @see I_CompletionCondition.
 *
 *  @date 16.12.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Fabian Haensel, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_COMPLETIONCONDITION_H
#define I_COMPLETIONCONDITION_H

/**
 * This module takes completion calls (pre) and analyzes their respective
 * requests. It removes any inactive, null, or invalid requests from the
 * completion calls. This is only done for the array versions, most importantly
 * these are  Waitany/some calls which may have many unused
 * array entries. Currently we only apply this to the Wait calls, as this information
 * is not needed atm for Test calls.
 *
 * In contrast to I_RequestCondition this only removes inactive/null requests
 * whereas I_RequestCondition filters out completed requests for the after the
 * PMPI call of the Wait/Test returned.
 *
 * Dependencies (order as listed):
 * - I_RequestTrack
 */
class I_CompletionCondition : public gti::I_Module
{
public:
    /**
     * Notifcation of an MPI_Wait completion.
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param request to complete.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN wait (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType request) = 0;

    /**
    	 * Notifcation of a any completion.
    	 *
    	 * @param pId parallel id of the call site.
    	 * @param lId location id of the call site.
    	 * @param requests to complete one of.
    	 * @param count number of requests in array.
    	 * @param index of request to complete.
    	 * @return @see gti::GTI_ANALYSIS_RETURN.
    	 */
    	virtual gti::GTI_ANALYSIS_RETURN waitAny (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType* requests,
    			int count) = 0;

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
    	virtual gti::GTI_ANALYSIS_RETURN waitAll (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType *requests,
    			int count) = 0;

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
    	virtual gti::GTI_ANALYSIS_RETURN waitSome (
    			MustParallelId pId,
    			MustLocationId lId,
    			MustRequestType *requests,
    			int count) = 0;
};/*class I_CompletionCondition*/

#endif /*I_COMPLETIONCONDITION_H*/
