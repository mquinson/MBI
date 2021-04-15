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
 * @file DatatypePredefs.h
 *       @see MUST::DatatypePredefs.
 *
 *  @date 18.02.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_DatatypePredefs.h"

#ifndef DATATYPEPREDEFS_H
#define DATATYPEPREDEFS_H

using namespace gti;

namespace must
{
	/**
     * Implementation for I_DatatypePredefs.
     */
    class DatatypePredefs : public gti::ModuleBase<DatatypePredefs, I_DatatypePredefs>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		DatatypePredefs (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~DatatypePredefs (void);

    		/**
    		 * @see I_DatatypePredefs::propagate.
    		 */
    		GTI_ANALYSIS_RETURN propagate (
                MustParallelId pId
            );

    }; /*class DatatypePredefs */
} /*namespace MUST*/

#endif /*DATATYPEPREDEFS_H*/
