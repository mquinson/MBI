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
 * @file OperationChecks.h
 *       @see MUST::OperationChecks.
 *
 *  @date 26.05.2011
 *  @author Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_OperationChecks.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"
#include "I_OpTrack.h"

#include <string>

#ifndef OPERATIONCHECKS_H
#define OPERATIONCHECKS_H

using namespace gti;

namespace must
{
	/**
     * Implementation for I_OperationChecks.
     */
    class OperationChecks : public gti::ModuleBase<OperationChecks, I_OperationChecks>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		OperationChecks (const char* instanceName);

    	/**
    	 * Destructor.
    	 */
    		virtual ~OperationChecks (void);

   		/**
   		 * @see I_OperationChecks::errorIfPredefined.
   		 */
    		GTI_ANALYSIS_RETURN errorIfPredefined (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustOpType op
    				);

   		/**
   		 * @see I_OperationChecks::errorIfNotKnown.
   		 */
    		GTI_ANALYSIS_RETURN errorIfNotKnown (
    				MustParallelId pId,
    				MustLocationId lId,
    				int aId,
    				MustOpType op
    				);

       	/**
       	 * @see I_OperationChecks::errorIfNull.
       	 */
        	GTI_ANALYSIS_RETURN errorIfNull (
        			MustParallelId pId,
        			MustLocationId lId,
        			int aId,
        			MustOpType op
        			);

    protected:
    		I_ParallelIdAnalysis* myPIdMod;
    	    I_CreateMessage* myLogger;
    	    I_ArgumentAnalysis* myArgMod;
    	    I_OpTrack* myOpMod;
    };
} /*namespace MUST*/

#endif /*OPERATIONCHECKS_H*/
