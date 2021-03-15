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
 * @file DCollectiveInitNotify.cpp
 *       @see MUST::DCollectiveInitNotify.
 *
 *  @date 03.05.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Fabian Haensel, Joachim Protze
 */

#include "GtiMacros.h"
#include "DCollectiveInitNotify.h"
#include "DistributedDeadlockApi.h"

using namespace must;

mGET_INSTANCE_FUNCTION(DCollectiveInitNotify)
mFREE_INSTANCE_FUNCTION(DCollectiveInitNotify)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DCollectiveInitNotify)

//=============================
// Constructor
//=============================
DCollectiveInitNotify::DCollectiveInitNotify (const char* instanceName)
    : gti::ModuleBase<DCollectiveInitNotify, I_DCollectiveInitNotify> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    //None
}

//=============================
// Destructor
//=============================
DCollectiveInitNotify::~DCollectiveInitNotify ()
{
    //TODO
}

//=============================
// notifyInit
//=============================
GTI_ANALYSIS_RETURN DCollectiveInitNotify::notifyInit (void)
{
    dCollMatchAncestorHasIntraP f;
    getWrapperFunction ("dCollMatchAncestorHasIntra", (GTI_Fct_t*)&f);

    if (f)
        (*f) (0);

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
