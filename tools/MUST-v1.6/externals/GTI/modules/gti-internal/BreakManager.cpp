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
 * @file BreakManager.cpp
 *       @see MUST::BreakManager.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "BreakManager.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(BreakManager);
mFREE_INSTANCE_FUNCTION(BreakManager);
mPNMPI_REGISTRATIONPOINT_FUNCTION(BreakManager);

//=============================
// Constructor
//=============================
BreakManager::BreakManager (const char* instanceName)
    : gti::ModuleBase<BreakManager, I_BreakManager> (instanceName),
      myRequestedBreak (false)
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
    ModuleBase<BreakManager, I_BreakManager>::getBroadcastFunction("gtiBroadcastBreak", (GTI_Fct_t*)&myFBroadcastBreak);
    assert (myFBroadcastBreak); //otherwise this is an inconsistent tool configuration which will not work
}

//=============================
// Destructor
//=============================
BreakManager::~BreakManager ()
{
    /*Nothing to do*/
}

//=============================
// requestBreak
//=============================
GTI_ANALYSIS_RETURN BreakManager::requestBreak (I_ChannelId *thisChannel)
{
    /*
     * We assume that the reduction is around,
     * so we always only get a single request for requesting
     * or removing a break.
     * Thus, no state tracking necessary.
     */
    assert (!myRequestedBreak);
    (*myFBroadcastBreak) (1);
    myRequestedBreak = true;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// removeBreak
//=============================
GTI_ANALYSIS_RETURN BreakManager::removeBreak (I_ChannelId *thisChannel)
{
    /*
     * We assume that the reduction is around,
     * so we always only get a single request for requesting
     * or removing a break.
     * Thus, no state tracking necessary.
     */
    assert (myRequestedBreak);
    (*myFBroadcastBreak) (0);
    myRequestedBreak = false;
    return GTI_ANALYSIS_SUCCESS;
}

/**
 * @todo The BreakManager should also listen to timeouts, such that it can remove a break in case i can somehow not be resolved.
 * E.g. some proces first must create 10mil isends before anything can progress on tha application, than we may have also
 *        10mil operations in an DWaitState trace and may be unable to change that in any case.
 */

/*EOF*/
