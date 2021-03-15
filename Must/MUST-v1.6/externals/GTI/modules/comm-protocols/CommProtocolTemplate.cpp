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
 * @file CommProtocolTemplate.cpp
 *       Template for implementations of the communication
 *       protocol interface.
 *
 * All functions of the interface are offered, but
 * will lead to an assertion.
 *
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>

#include "CommProtocolTemplate.h"
#include "GtiMacros.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(CommProtocolTemplate)
mFREE_INSTANCE_FUNCTION(CommProtocolTemplate)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommProtocolTemplate)

//=============================
// CommProtocolTemplate
//=============================
CommProtocolTemplate::CommProtocolTemplate (const char* instanceName)
: ModuleBase<CommProtocolTemplate, I_CommProtocol> (instanceName)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A Comm Protocol likely needs no sub modules
    assert (subModInstances.empty());
}

//=============================
// ~CommProtocolTemplate
//=============================
CommProtocolTemplate::~CommProtocolTemplate (void)
{
    //TODO
}

//=============================
// isConnected
//=============================
bool CommProtocolTemplate::isConnected (void)
{
	return false;
}

//=============================
// isInitialized
//=============================
bool CommProtocolTemplate::isInitialized (void)
{
	return false;
}

//=============================
// isFinalized
//=============================
bool CommProtocolTemplate::isFinalized (void)
{
	return false;
}

//=============================
// shutdown
//=============================
GTI_RETURN CommProtocolTemplate::shutdown (void)
{
    assert (0);
    return GTI_SUCCESS;
}

//=============================
// getNumChannels
//=============================
GTI_RETURN CommProtocolTemplate::getNumChannels (
        uint64_t* out_numChannels)
{
    assert (0);
    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CommProtocolTemplate::getPlaceId (uint64_t* outPlaceId)
{
    assert (0);
    return GTI_SUCCESS;
}

//=============================
// removeOutstandingRequests
//=============================
GTI_RETURN CommProtocolTemplate::removeOutstandingRequests (void)
{
    return GTI_SUCCESS;
}

//=============================
// ssend
//=============================
GTI_RETURN CommProtocolTemplate::ssend (
        void* buf,
        uint64_t num_bytes,
        uint64_t channel
        )
{
    assert (0);
    return GTI_SUCCESS;
}

//=============================
// isend
//=============================
GTI_RETURN CommProtocolTemplate::isend (
        void* buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
    assert (0);
    return GTI_SUCCESS;
}

//=============================
// recv
//=============================
GTI_RETURN CommProtocolTemplate::recv (
        void* out_buf,
        uint64_t num_bytes,
        uint64_t* out_length,
        uint64_t channel,
        uint64_t *out_channel
        )
{
    assert (0);
    return GTI_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_RETURN CommProtocolTemplate::irecv (
        void* out_buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
    assert (0);
   return GTI_SUCCESS;
}

//=============================
// test_msg
//=============================
GTI_RETURN CommProtocolTemplate::test_msg (
        unsigned int request,
        int* out_completed,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
    assert (0);
   return GTI_SUCCESS;
}

//=============================
// wait_msg
//=============================
GTI_RETURN CommProtocolTemplate::wait_msg (
        unsigned int request,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
    assert (0);
    return GTI_SUCCESS;
}

/*EOF*/
