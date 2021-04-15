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
 * @file ArgumentAnalysis.h
 *       @see MUST::ArgumentAnalysis.
 *
 *  @date 28.02.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ArgumentAnalysis.h"

#ifndef ARGUMENTANALYSIS_H
#define ARGUMENTANALYSIS_H

using namespace gti;

namespace must
{
	/**
     * Implementation header for I_ArgumentAnalysis.
     */
    class ArgumentAnalysis : public gti::ModuleBase<ArgumentAnalysis, I_ArgumentAnalysis>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		ArgumentAnalysis (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~ArgumentAnalysis (void);

    		/**
    		 * @see I_ArgumentAnalysis::getIndex.
    		 */
    		int getIndex (MustArgumentId id);

    		/**
    		 * @see I_ArgumentAnalysis::getArgName.
    		 */
    		std::string getArgName (MustArgumentId id);

    protected:
    		std::string *myArgNames;

    }; /*class ArgumentAnalysis */
} /*namespace MUST*/

#endif /*ARGUMENTANALYSIS_H*/
