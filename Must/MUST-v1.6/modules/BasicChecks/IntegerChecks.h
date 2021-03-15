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
 * @file IntegerChecks.h
 *       @see MUST::IntegerChecks.
 *
 *  @date 01.03.2011
 *  @author Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_IntegerChecks.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"
#include "I_BaseConstants.h"

#include <string>

#ifndef INTEGERCHECKS_H
#define INTEGERCHECKS_H

using namespace gti;

namespace must
{
	/**
     * Implementation for I_IntegerChecks.
     */
    class IntegerChecks : public gti::ModuleBase<IntegerChecks, I_IntegerChecks>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		IntegerChecks (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~IntegerChecks (void);

    		/**
    		 * @see I_IntegerChecks::errorIfLessThanZero.
    		 */
    		GTI_ANALYSIS_RETURN errorIfLessThanZero (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::warningIfZero.
    		 */
    		GTI_ANALYSIS_RETURN warningIfZero (
    			MustParallelId pId, 
    			MustLocationId lId, 
	    		int aId, 
    			int value);

    		/**
    		 * @see I_IntegerChecks::errorIfZero.
    		 */
    		GTI_ANALYSIS_RETURN errorIfZero (
    			MustParallelId pId, 
	    		MustLocationId lId, 
    			int aId, 
    			int value);

    		/**
    		 * @see I_IntegerChecks::warningIfNotOneOrZero.
    		 */
    		GTI_ANALYSIS_RETURN warningIfNotOneOrZero (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::errorIfLessThanZeroArray.
    		 */
    		GTI_ANALYSIS_RETURN errorIfLessThanZeroArray (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		const int* array,
    			int size);

    		/**
    		 * @see I_IntegerChecks::warningIfZeroArray.
    		 */
    		GTI_ANALYSIS_RETURN warningIfZeroArray (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		const int* array,
    			int size);

    		/**
    		 * @see I_IntegerChecks::warningIfNotOneOrZeroArray.
    		 */
    		GTI_ANALYSIS_RETURN warningIfNotOneOrZeroArray (
    			MustParallelId pId, 
	    		MustLocationId lId, 
    			int aId, 
    			const int* array,
	    		int size);

    		/**
    		 * @see I_IntegerChecks::errorIfEntryIsGreaterOrEqualArray.
    		 */
    		GTI_ANALYSIS_RETURN errorIfEntryIsGreaterOrEqualArray (
    			MustParallelId pId, 
    			MustLocationId lId, 
	    		int aId, 
    			const int* array,
    			int size, 
	    		int border);

    		/**
    		 * @see I_IntegerChecks::errorIfNegativNotProcNullAnySource.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNegativNotProcNullAnySource (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::errorIfNegativNotProcNull.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNegativNotProcNull (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::errorIfNotWithinRangeZeroAndLessTagUb.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNotWithinRangeZeroAndLessTagUb (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::errorIfNotWithinRangeZeroAndLessTagUbAnyTag.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNotWithinRangeZeroAndLessTagUbAnyTag (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks:: warningIfIsHighButLessTagUb.
    		 */
    		GTI_ANALYSIS_RETURN  warningIfIsHighButLessTagUb (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::errorIfNegativNotProcNullArray.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNegativNotProcNullArray (
    			MustParallelId pId, 
	    		MustLocationId lId, 
    			int aId, 
    			const int* array,
	    		int size);

    		/**
    		 * @see I_IntegerChecks::errorIfNegativProcNullAnySource.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNegativProcNullAnySource (
    			MustParallelId pId, 
    			MustLocationId lId, 
	    		int aId, 
    			int value);

    		/**
    		 * @see I_IntegerChecks::errorIfNegativNotUndefined.
    		 */
    		GTI_ANALYSIS_RETURN errorIfNegativNotUndefined (
	    		MustParallelId pId, 
    			MustLocationId lId, 
    			int aId, 
	    		int value);

    		/**
    		 * @see I_IntegerChecks::errorIfDuplicateRank.
    		 */
    		GTI_ANALYSIS_RETURN errorIfDuplicateRank (
	    		MustParallelId pId,
    			MustLocationId lId,
    			int aId,
	    		const int* array,
	    		int size);

    		/**
    		 * @see I_IntegerChecks::checkGroupRangeArray.
    		 */
    		GTI_ANALYSIS_RETURN checkGroupRangeArray (
	    		MustParallelId pId,
    			MustLocationId lId,
    			int aId,
	    		const int* array,
	    		int size);

    protected:
    		I_ParallelIdAnalysis* myPIdMod;
    	    I_CreateMessage* myLogger;
    	    I_ArgumentAnalysis* myArgMod;
    	    I_BaseConstants* myConstMod;
    };
} /*namespace MUST*/

#endif /*INTEGERCHECKS_H*/
