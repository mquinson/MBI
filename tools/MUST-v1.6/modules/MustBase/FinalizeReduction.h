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
 * @file FinalizeReduction.h
 *       @see MUST::FinalizeReduction.
 *
 *  @date 04.04.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "CompletionTree.h"

#include "I_FinalizeReduction.h"

#ifndef FINALIZEREDUCTION_H
#define FINALIZEREDUCTION_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_FinalizeReduction
     */
    class FinalizeReduction : public gti::ModuleBase<FinalizeReduction, I_FinalizeReduction>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		FinalizeReduction (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~FinalizeReduction (void);

    		/**
    		 * @see I_FinalizeReduction::reduce
    		 */
    		GTI_ANALYSIS_RETURN reduce (
    				gti::I_ChannelId *thisChannel,
    				std::list<gti::I_ChannelId*> *outFinishedChannels);

    		/**
    		 * The timeout function, see gti::I_Reduction.timeout
    		 */
    		void timeout (void);

    protected:

    		std::list<I_ChannelId*> myReductionPartners; /**< list of reduction partners.*/
    		/**
    		 * Completion tree, IMPORTANT: we only do one single reduction here,
    		 * it either fails or succeeds, so we don't need a history of timed-out
    		 * reductions here.
    		 */
    		CompletionTree *myCompletion;
    		bool myTimedOut;
    };
} /*namespace MUST*/

#endif /*FINALIZEREDUCTION_H*/
