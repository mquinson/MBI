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
 *  @date 21.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_Template.h"

#ifndef TEMPLATE_H
#define TEMPLATE_H

using namespace gti;

namespace must
{
	/**
     * Template for analysis interface implementation.
     */
    class Template : public gti::ModuleBase<Template, I_Template>
    {
    protected:

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
    		 *
    		GTI_ANALYSIS_RETURN analysisFunction (void);
    		 */

    }; /*class Template */
} /*namespace MUST*/

#endif /*TEMPLATE_H*/
