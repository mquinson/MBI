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
 * @file RequestChecks.h
 *       @see MUST::RequestChecks.
 *
 *  @date 05.04.2011
 *  @author Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"

#include "I_RequestChecks.h"

#include <string>

#ifndef REQUESTCHECKS_H
#define REQUESTCHECKS_H

using namespace gti;

namespace must
{
	/**
     * RequestChecks for correctness checks interface implementation.
     */
    class RequestChecks : public gti::ModuleBase<RequestChecks, I_RequestChecks>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		RequestChecks (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~RequestChecks (void);

    		/**
    		 * @see I_RequestChecks::errorIfNotKnown.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNotKnown (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestChecks::errorIfNull.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNull (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestChecks::errorIfPersistentButInactive.
    		 */
    		GTI_ANALYSIS_RETURN errorIfPersistentButInactive (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);


    		/**
    		 * @see I_RequestChecks::warningIfNullOtInactive.
    		 */
    		GTI_ANALYSIS_RETURN warningIfNullOrInactive (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestChecks::warningIfActiveRecv.
    		 */
    		GTI_ANALYSIS_RETURN warningIfActiveRecv (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);


    		/**
    		 * @see I_RequestChecks::warningIfCanceled.
    		 */
    		GTI_ANALYSIS_RETURN warningIfCanceled(
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);

    		/**
    		 * @see I_RequestChecks::errorIfActive.
    		 */
    		GTI_ANALYSIS_RETURN errorIfActive (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType request);

    		/**
    		* @see I_RequestChecks::errorIfActiveArray.
    		*/
    	    GTI_ANALYSIS_RETURN errorIfActiveArray (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		MustRequestType* requests,
    	    		int size);

    		/**
    		* @see I_RequestChecks::errorIfNotKnownArray.
    		*/
    	    GTI_ANALYSIS_RETURN errorIfNotKnownArray (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		MustRequestType* requests,
    	    		int size);

    		/**
    		 * @see I_RequestChecks::warningIfNullOtInactiveArray.
    		 */
    		GTI_ANALYSIS_RETURN warningIfNullOrInactiveArray (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustRequestType* requests,
    				int size);

    		/**
    		* @see I_RequestChecks::errorIfNullArray.
    		*/
    	    GTI_ANALYSIS_RETURN errorIfNullArray (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		MustRequestType* requests,
    	    		int size);
    	    		
    protected:
    		I_ParallelIdAnalysis* myPIdMod;
    	    I_CreateMessage* myLogger;
    	    I_ArgumentAnalysis* myArgMod;
    	    I_RequestTrack* myReqMod;
	};
} /*namespace MUST*/

#endif /*REQUESTCHECKS_H*/
