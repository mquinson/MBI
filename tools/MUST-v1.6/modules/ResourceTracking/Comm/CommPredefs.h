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
 * @file CommPredefs.h
 *       @see MUST::CommPredefs.
 *
 *  @date 04.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "ModuleBase.h"

#include "I_CommPredefs.h"

#ifndef COMMPREDEFS_H
#define COMMPREDEFS_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_CommPredefs.
     */
    class CommPredefs : public gti::ModuleBase<CommPredefs, I_CommPredefs>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		CommPredefs (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~CommPredefs (void);

    		/**
    		 * @see I_CommPredefs::propagate.
    		 */
    		GTI_ANALYSIS_RETURN propagate (
                MustParallelId pId
            );

    protected:

    };
} /*namespace MUST*/

#endif /*COMMPREDEFS_H*/
