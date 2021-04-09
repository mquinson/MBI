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
 * @file BasicIntegrities.h
 *       @see MUST::BasicIntegrities.
 *
 *  @date 10.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "mustConfig.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"
#include "I_BaseConstants.h"

#include "I_BasicIntegrities.h"

#ifndef BASICINTEGRITIES_H
#define BASICINTEGRITIES_H

using namespace gti;

namespace must
{
	/**
     * Template for correctness checks interface implementation.
     */
    class BasicIntegrities : public gti::ModuleBase<BasicIntegrities, I_BasicIntegrities>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		BasicIntegrities (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~BasicIntegrities (void);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullCondition
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullCondition (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		int size,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::errorIfNull
    		 */
    		GTI_ANALYSIS_RETURN errorIfNull (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullCommSize
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullCommSize (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		MustCommType comm,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullAndNotMpiBottomConditionCommSize
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomConditionCommSize (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		const int* array,
    		    		MustCommType comm,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullAndNotMpiBottom
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottom (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		int size,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::warningIfNull
    		 */
    		GTI_ANALYSIS_RETURN warningIfNull (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::warningForLowThreadlevel
    		 */
    		GTI_ANALYSIS_RETURN warningForLowThreadlevel (
						MustParallelId pId,
						MustLocationId lId,
						int requested,
						int provided);

			/**
    		 * @see I_BasicIntegrities::errorIfNullAndNotMpiBottomAtIndexCommSize
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomAtIndexCommSize (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		const int* array,
    		    		MustCommType comm,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullStatus
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullStatus (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		const void* pointer);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullStatuses
    		 */
    		GTI_ANALYSIS_RETURN errorIfNullStatuses (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		int aId,
    		    		const void* pointer);
    		/**
    		 * @see I_BasicIntegrities::errorIfNullAndNotMpiBottomOnlyOnRoot
    		 */
    	    GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomOnlyOnRoot (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		int size,
    	    		const void* pointer,
    	    		int root,
    	    		MustCommType comm
    				);
    		/**
    		 * @see I_BasicIntegrities::errorIfNullOnlyOnRoot
    		 */
    	    GTI_ANALYSIS_RETURN errorIfNullOnlyOnRoot (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		const void* pointer,
    	    		int root,
    	    		MustCommType comm
    				);

    		/**
    		 * @see I_BasicIntegrities::errorIfNullAndNotMpiBottomConditionCommSizeOnlyOnRoot
    		 */
    	    GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomConditionCommSizeOnlyOnRoot (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		const int* array,
    	    		MustCommType comm,
    	    		const void* pointer,
    	    		int root
    	    		);
    		/**
    		 * @see I_BasicIntegrities::errorIfNullCommSizeOnlyOnRoot
    		 */
    	    GTI_ANALYSIS_RETURN errorIfNullCommSizeOnlyOnRoot (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		MustCommType comm,
    	    		const void* pointer,
    	    		int root);

    		/**
    		 * @see I_BasicIntegrities::errorIfInPlaceOtherThanRoot
    		 */
    GTI_ANALYSIS_RETURN errorIfInPlaceOtherThanRoot (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm,
    		MustAddressType pointer,
    		int root);


    		/**
    		 * @see I_BasicIntegrities::errorIfInPlaceUsed
    		 */
    GTI_ANALYSIS_RETURN errorIfInPlaceUsed (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustAddressType pointer);

    protected:
    		I_ParallelIdAnalysis* myPIdMod;
    	    I_CreateMessage* myLogger;
    	    I_ArgumentAnalysis* myArgMod;
    	    I_BaseConstants* myConstMod;
    	    I_CommTrack* myCommMod;
    };
} /*namespace MUST*/

#endif /*BASICINTEGRITIES_H*/
