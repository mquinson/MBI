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
 * @file PanicHandler.cpp
 *       @see MUST::PanicHandler.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "PanicHandler.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(PanicHandler);
mFREE_INSTANCE_FUNCTION(PanicHandler);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PanicHandler);

//=============================
// Constructor
//=============================
PanicHandler::PanicHandler (const char* instanceName)
    : gti::ModuleBase<PanicHandler, I_PanicHandler> (instanceName),
      myHadPanic (false)
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
PanicHandler::~PanicHandler ()
{
    /*Nothing to do*/
}

//=============================
// raisePanic
//=============================
GTI_ANALYSIS_RETURN PanicHandler::raisePanic ()
{
    //Avoid creation of multiple panic notifications
    if (myHadPanic)
        return GTI_ANALYSIS_SUCCESS;

    myHadPanic = true;

#ifdef GTI_DEBUG
    std::cout << getpid() << "MUST: got a raise panic at the PanicHandler!" << std::endl;
#endif /*GTI_DEBUG*/

    //Create the panic notification
    gtiNotifyPanicP f;
    if (ModuleBase<PanicHandler, I_PanicHandler>::getBroadcastFunction ("gtiNotifyPanic", (GTI_Fct_t*)&f) != GTI_SUCCESS)
    {
        std::cerr << "GTI's implicit internal panic notification event was not found in the PanicHandler, internal error, aborting." << std::endl;
        return GTI_ANALYSIS_FAILURE;
    }

    (*f) ();

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
