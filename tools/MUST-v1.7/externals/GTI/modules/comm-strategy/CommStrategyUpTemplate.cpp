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
 * @file CommStrategyUpTemplate.cpp
 *       Template for implementations of the communication
 *       protocol interface.
 *
 * All functions of the interface are offered
 * and will lead to an assertion.
 *
 */

#include "CommStrategyUpTemplate.h"
#include "GtiMacros.h"
#include "GtiApi.h"

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CommStrategyUpTemplate)
mFREE_INSTANCE_FUNCTION(CommStrategyUpTemplate)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommStrategyUpTemplate)
mCOMM_STRATEGY_UP_RAISE_PANIC(CommStrategyUpTemplate)

//=============================
// CommStrategyUpTemplate
//=============================
CommStrategyUpTemplate::CommStrategyUpTemplate (const char* instanceName)
    : ModuleBase<CommStrategyUpTemplate, I_CommStrategyUp> (instanceName)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    if (subModInstances.size() >= 1)
        protocol = (I_CommProtocol*) subModInstances[0];
    else
        protocol = NULL;
}

//=============================
// ~CommStrategyUpTemplate
//=============================
CommStrategyUpTemplate::~CommStrategyUpTemplate (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;
}

//=============================
// shutdown
//=============================
GTI_RETURN CommStrategyUpTemplate::shutdown (
        GTI_FLUSH_TYPE flush_behavior,
        GTI_SYNC_TYPE sync_behavior)
{
    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CommStrategyUpTemplate::getPlaceId (uint64_t* outPlaceId)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==getPlaceId   
    if (outPlaceId)   
        protocol->getPlaceId(outPlaceId);

    return GTI_SUCCESS;
}

//=============================
// send
//=============================
GTI_RETURN CommStrategyUpTemplate::send (
        void* buf,
        uint64_t num_bytes,
        void* free_data,
        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CommStrategyUpTemplate::test (
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    *out_flag = false;

    return GTI_SUCCESS;
}

//=============================
// wait
//=============================
GTI_RETURN CommStrategyUpTemplate::wait (
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    while (1==1) {}
    return GTI_SUCCESS;
}

//=============================
// raisePanic
//=============================
GTI_RETURN CommStrategyUpTemplate::raisePanic (void)
{
    //1) Flush

    //2) Disable future aggregation

    //3) Create the GTI internal panic event
    gtiRaisePanicP f;
    if (getWrapperFunction("gtiRaisePanic", (GTI_Fct_t*) &f) == GTI_SUCCESS)
    {
        //Raise the panic
        (*f) ();
    }
    else
    {
        std::cerr << "MUST internal error: could not find the creation function for the GTI internal event \"gtiRaisePanic\", this should never happen (" << __FILE__ << ":" << __LINE__ << ")." << std::endl;
        return GTI_ERROR;
    }

    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CommStrategyUpTemplate::flush (
        void
        )
{
    return GTI_SUCCESS;
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CommStrategyUpTemplate::flushAndSetImmediate ()
{
    //1) Flush
    flush ();

    //2) Set immediate

    return GTI_SUCCESS;
}

/*EOF*/
