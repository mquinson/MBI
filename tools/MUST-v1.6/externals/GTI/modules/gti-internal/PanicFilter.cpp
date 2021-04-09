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
 * @file PanicFilter.cpp
 *       @see MUST::PanicFilter.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "PanicFilter.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(PanicFilter);
mFREE_INSTANCE_FUNCTION(PanicFilter);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PanicFilter);

//=============================
// Constructor
//=============================
PanicFilter::PanicFilter (const char* instanceName)
    : gti::ModuleBase<PanicFilter, I_PanicFilter> (instanceName),
      myHadPanic (false)
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
PanicFilter::~PanicFilter ()
{
    /*Nothing to do*/
}

//=============================
// propagate
//=============================
GTI_ANALYSIS_RETURN PanicFilter::propagate (
        gti::I_ChannelId *thisChannel,
        std::list<gti::I_ChannelId*> *outFinishedChannels)
{
    /*
     * If we already handled this we can filter out all other events of this type.
     */
    if (myHadPanic)
        return GTI_ANALYSIS_SUCCESS;

    myHadPanic = true;

#ifdef GTI_DEBUG
    std::cout << getpid() << "MUST: got a raise panic at the PanicFilter!" << std::endl;
#endif /*GTI_DEBUG*/

    /*
     * If it is not the first event we recreate(forward) it.
     */
    gtiRaisePanicP f;
    if (getWrapperFunction ("gtiRaisePanic", (GTI_Fct_t*) &f) == GTI_SUCCESS)
    {
        (*f) ();
    }
    else
    {
        std::cerr << "Internal MUST error: could not find wrapper function to create the GTI internal event \"gtiRaisePanic\", this should not happen (" << __FILE__ << ":" << __LINE__ << ")." << std::endl;
        return GTI_ANALYSIS_FAILURE;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// timeout
//=============================
void PanicFilter::timeout (void)
{
    /*Nothing to do, as we never return GTI_ANALYSIS_WAITING*/
}

/*EOF*/
