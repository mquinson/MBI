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
 * @file ShutdownHandler.cpp
 *       @see MUST::ShutdownHandler.
 *
 *  @date 27.06.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "ShutdownHandler.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(ShutdownHandler);
mFREE_INSTANCE_FUNCTION(ShutdownHandler);
mPNMPI_REGISTRATIONPOINT_FUNCTION(ShutdownHandler);

//=============================
// Constructor
//=============================
ShutdownHandler::ShutdownHandler (const char* instanceName)
    : gti::ModuleBase<ShutdownHandler, I_ShutdownHandler> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODS 0
/*    if (subModInstances.size() < NUM_SUB_MODS)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }*/
    if (subModInstances.size() > NUM_SUB_MODS)
    {
            for (int i = NUM_SUB_MODS; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    //Initialize module data
}

//=============================
// Destructor
//=============================
ShutdownHandler::~ShutdownHandler ()
{

}

//=============================
// notifyShutdown
//=============================
GTI_ANALYSIS_RETURN ShutdownHandler::notifyShutdown (gti::I_ChannelId* channelId)
{
    gtiShutdownNotifyP f;
    if (ModuleBase<ShutdownHandler, I_ShutdownHandler>::getBroadcastFunction ("gtiShutdownNotify", (GTI_Fct_t*)&f) != GTI_SUCCESS)
    {
        std::cerr << "GTI's implicit internal shutdown event was not found in the ShutdownHandler, internal error, aborting." << std::endl;
        return GTI_ANALYSIS_FAILURE;
    }

    f ();

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
