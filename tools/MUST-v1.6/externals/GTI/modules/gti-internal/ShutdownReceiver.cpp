/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file ShutdownReceiver.cpp
 *       @see MUST::ShutdownReceiver.
 *
 *  @date 27.06.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"

#include "ShutdownReceiver.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(ShutdownReceiver);
mFREE_INSTANCE_FUNCTION(ShutdownReceiver);
mPNMPI_REGISTRATIONPOINT_FUNCTION(ShutdownReceiver);

//=============================
// Constructor
//=============================
ShutdownReceiver::ShutdownReceiver (const char* instanceName)
    : gti::ModuleBase<ShutdownReceiver, I_ShutdownReceiver> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBS 0
/*    if (subModInstances.size() < NUM_SUBS)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }*/
    if (subModInstances.size() > NUM_SUBS)
    {
            for (int i = NUM_SUBS; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    //Initialize module data
    /*nothing to do*/
}

//=============================
// Destructor
//=============================
ShutdownReceiver::~ShutdownReceiver ()
{
    /*Nothing to do*/
}

//=============================
// receive
//=============================
GTI_ANALYSIS_RETURN ShutdownReceiver::receive ()
{
    /*Nothing to do*/
    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
