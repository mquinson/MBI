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
 * @file ParallelIdImpl.h
 *       Implementation for the parallel id analysis interface.
 *
 *  @date 07.01.2010
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"

#ifndef PARALLELIMPL_H
#define PARALLELIMPL_H

using namespace gti;

namespace must
{
	/**
     * Implementation for the parallel id analysis.
     */
    class ParallelIdImpl : public gti::ModuleBase<ParallelIdImpl, I_ParallelIdAnalysis>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		ParallelIdImpl (const char* instanceName);

        /**
         * @see I_ParallelIdAnalysis::getInfoForId.
         */
    		ParallelInfo getInfoForId (MustParallelId id);

    		/**
    		 * @see I_ParallelIdAnalysis::toString.
    		 */
    		std::string toString (MustParallelId id);
    }; /*class ParallelIdImpl */
} /*namespace MUST*/

#endif /*PARALLELIMPL_H*/
