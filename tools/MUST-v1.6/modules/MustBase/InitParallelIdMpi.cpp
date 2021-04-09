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
 * @file InitParallelIdMpi.cpp
 *       @see must::InitParallelIdMpi.
 *
 *  @date 16.04.2014
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "InitParallelIdMpi.h"

using namespace must;

mGET_INSTANCE_FUNCTION(InitParallelIdMpi)
mFREE_INSTANCE_FUNCTION(InitParallelIdMpi)
mPNMPI_REGISTRATIONPOINT_FUNCTION(InitParallelIdMpi)

//=============================
// Constructor
//=============================
InitParallelIdMpi::InitParallelIdMpi (const char* instanceName)
    : gti::ModuleBase<InitParallelIdMpi, I_InitParallelId> (instanceName),
      myInitedId (false),
      myPId (0)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();
    //No sub modules needed ...
}

//=============================
// init
//=============================
GTI_ANALYSIS_RETURN InitParallelIdMpi::init (MustParallelId *pStorage)
{
    if (!pStorage)
        return GTI_ANALYSIS_FAILURE;

    //Init if necessary
    if(!myInitedId)
    {
        /**
         * In our MPI specific case the pId is exactly equal to the layer id from GTI, so we use that one.
         */
        myPId = this->buildLayer ();
    }

    //Store it
    *pStorage = myPId;

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
