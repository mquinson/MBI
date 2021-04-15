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
 * @file BreakReduction.cpp
 *       @see MUST::BreakReduction.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "BreakReduction.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(BreakReduction);
mFREE_INSTANCE_FUNCTION(BreakReduction);
mPNMPI_REGISTRATIONPOINT_FUNCTION(BreakReduction);

//=============================
// Constructor
//=============================
BreakReduction::BreakReduction (const char* instanceName)
    : gti::ModuleBase<BreakReduction, I_BreakReduction> (instanceName),
      myRequestCount (0)
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
BreakReduction::~BreakReduction ()
{
    /*Nothing to do*/
}

//=============================
// addBreakRequest
//=============================
GTI_ANALYSIS_RETURN BreakReduction::addBreakRequest (
        gti::I_ChannelId *thisChannel,
        std::list<gti::I_ChannelId*> *outFinishedChannels)
{
    myRequestCount++;

    //If we just got a first request from a situation where we did not vote for break beforehand,
    //we let the request propagate to the next layer
    if (myRequestCount == 1)
    {
        return GTI_ANALYSIS_IRREDUCIBLE;
    }

    //If we are 0, or below we have an error (Someone did too many removes)
    if (myRequestCount <= 0)
    {
        std::cerr << getpid() << "in BreakReduction::addBreakRequest, someone issued too many consumes or too few requests! Internal Error." << std::endl;
        assert(0);
    }

    //If we are >1 we already voted for "break", so nothing to forward
    /*
     * @rational GTI_ANALYSIS_SUCCESS signals we finished an aggregation and should af injected an
     * aggregated event, since we did not inject, this yields the filtering we want
     */
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// removeBreakRequest
//=============================
GTI_ANALYSIS_RETURN BreakReduction::removeBreakRequest (
        gti::I_ChannelId *thisChannel,
        std::list<gti::I_ChannelId*> *outFinishedChannels)
{
    myRequestCount--;

    //If we just switched our voting decision from yes to no, then forward the change to the next layer
    if (myRequestCount == 0)
    {
        return GTI_ANALYSIS_IRREDUCIBLE;
    }

    //If we are below 0 we have an error (Someone did too many removes)
    if (myRequestCount < 0)
    {
        std::cerr << getpid() << "in BreakReduction::addBreakRequest, someone issued too many consumes or too few requests! Internal Error." << std::endl;
        assert(0);
    }

    //If we are >0 we still vote for "break", so nothing to forward
    /*
     * @rational GTI_ANALYSIS_SUCCESS signals we finished an aggregation and should af injected an
     * aggregated event, since we did not inject, this yields the filtering we want
     */
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// timeout
//=============================
void BreakReduction::timeout (void)
{
    /*
     * Nothing to do.
     * We never actually return a waiting, we never aggregate at all, we just filter.
     */
}

/*EOF*/
