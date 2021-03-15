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
 * @file InitParallelIdMpi.h
 *       @see must::InitParallelIdMpi.
 *
 *  @date 16.04.2014
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_InitParallelId.h"
#include "GtiHelper.h"

#ifndef INITPARALLELIDMPI_H
#define INITPARALLELIDMPI_H

using namespace gti;

namespace must
{
	/**
     * Implementation to set the parallel ID with information from an MPI rank.
     * (Different implementations can use different pieces of information from
     * different sources)
     */
    class InitParallelIdMpi : public gti::ModuleBase<InitParallelIdMpi, I_InitParallelId>, public gti::GtiHelper
    {
    protected:
        bool myInitedId;
        MustParallelId myPId;

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        InitParallelIdMpi (const char* instanceName);

        /**
         * @see I_InitParallelId::init
         */
         GTI_ANALYSIS_RETURN init (MustParallelId *pStorage);

    }; /*class InitParallelIdMpi */
} /*namespace MUST*/

#endif /*PARALLELIMPL_H*/
