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
 * @file CommStrategyDownTemplate.cpp
 *       Template for implementations of the communication
 *       protocol interface.
 *
 * All functions of the interface are offered
 * and will lead to an assertion.
 *
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pnmpimod.h>
#include "CommStrategyDownTemplate.h"
#include "GtiMacros.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(CommStrategyDownTemplate)
mFREE_INSTANCE_FUNCTION(CommStrategyDownTemplate)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommStrategyDownTemplate)

//=============================
// CommStrategyDownTemplate
//=============================
CommStrategyDownTemplate::CommStrategyDownTemplate (const char* instanceName)
: ModuleBase<CommStrategyDownTemplate, I_CommStrategyDown> (instanceName)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    if (subModInstances.size() >= 1)
        protocol = (I_CommProtocol*) subModInstances[0];
    else
        protocol = NULL;
}

//=============================
// ~CommStrategyDownTemplate
//=============================
CommStrategyDownTemplate::~CommStrategyDownTemplate (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;

}

//=============================
// shutdown
//=============================
GTI_RETURN CommStrategyDownTemplate::shutdown (
        GTI_FLUSH_TYPE flush_behavior,
        GTI_SYNC_TYPE sync_behavior)
{
    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CommStrategyDownTemplate::getPlaceId (uint64_t* outPlaceId)
{
    //==Check whether a connection exists
    if (!protocol || !protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==getPlaceId   
    if (outPlaceId)   
        protocol->getPlaceId(outPlaceId);

    return GTI_SUCCESS;
}

//=============================
// getNumClients
//=============================
GTI_RETURN CommStrategyDownTemplate::getNumClients (uint64_t *outNumClients)
{
    *outNumClients = 0;
    return GTI_SUCCESS;
}

//=============================
// broadcast
//=============================
GTI_RETURN CommStrategyDownTemplate::broadcast (
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
GTI_RETURN CommStrategyDownTemplate::test (
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
        uint64_t *outChannel,
        uint64_t preferChannel
        )
{
    *out_flag=false;
    return GTI_SUCCESS;
}

//=============================
// wait
//=============================
GTI_RETURN CommStrategyDownTemplate::wait (
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
        uint64_t *outChannel
        )
{
    /*???*/
    while( 1 == 1) {}
    return GTI_SUCCESS;
}

//=============================
// acknowledge
//=============================
GTI_RETURN CommStrategyDownTemplate::acknowledge (
	uint64_t channel
    )
{

	return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CommStrategyDownTemplate::flush (
        void
        )
{
    return GTI_SUCCESS;
}

/*EOF*/
