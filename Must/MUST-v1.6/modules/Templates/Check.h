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
 * @file Template.h
 *       @see MUST::Template.
 *
 *  @date 01.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"

#include "I_Template.h"

#include <string>

#ifndef TEMPLATE_H
#define TEMPLATE_H

using namespace gti;

namespace must
{
	/**
     * Template for correctness checks interface implementation.
     */
    class Template : public gti::ModuleBase<Template, I_Template>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		Template (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~Template (void);

    		/**
    		 * @see I_Template::analysisFunction.
    		 */
    		GTI_ANALYSIS_RETURN analysisFunction (MustParallelId pId, MustLocationId lId, int aId);

    protected:
    		I_ParallelIdAnalysis* myPIdMod;
    	    I_CreateMessage* myLogger;
    	    I_ArgumentAnalysis* myArgMod;
    };
} /*namespace MUST*/

#endif /*TEMPLATE_H*/
