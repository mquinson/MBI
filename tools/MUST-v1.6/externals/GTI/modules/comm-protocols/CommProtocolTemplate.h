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
 * @file CommProtocolTemplate.h
 *       A implementation of the communication protocol interface.
 *
 *  This implementation may be used to create the base structure of other
 *  communication protocols.
 *
 * @author Tobias Hilbrich
 */

#include "I_CommProtocol.h"
#include "I_Module.h"
#include "ModuleBase.h"

#ifndef COMM_PROTOCOL_TEMPLATE_H
#define COMM_PROTOCOL_TEMPLATE_H

namespace gti
{
    /**
     * Class that describes the communication protocol interface.
     */
    class CommProtocolTemplate : public ModuleBase<CommProtocolTemplate, I_CommProtocol>
    {
    public:

        /**
         * Constructor.
         * @ref ModConf - The module configuration syntax
         * @param intanceName name of the module instance.
         */
        CommProtocolTemplate (const char* instanceName);

        /**
         * Destructor.
         */
        ~CommProtocolTemplate (void);

        /**
         * @see gti::I_CommProtocol::isConnected
         */
        bool isConnected (void);

        /**
         * @see gti::I_CommProtocol::isInitialized
         */
        bool isInitialized (void);

        /**
         * @see gti::I_CommProtocol::isFinalized
         */
        bool isFinalized (void);

        /**
         * @see gti::I_CommProtocol::getNumChannels
         */
        GTI_RETURN getNumChannels (uint64_t* out_numChannels);

        /**
         * @see gti::I_CommProtocol::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see gti::I_CommProtocol::shutdown
         */
        GTI_RETURN shutdown (void);

        /**
         * @see gti::I_CommProtocol::removeOutstandingRequests
         */
        GTI_RETURN removeOutstandingRequests (void);

        /**
         * @see gti::I_CommProtocol::ssend
         */
        GTI_RETURN ssend (
                void* buf,
                uint64_t num_bytes,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::isend
         */
        GTI_RETURN isend (
                void* buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::recv
         */
        GTI_RETURN recv (
                void* out_buf,
                uint64_t num_bytes,
                uint64_t* out_length,
                uint64_t channel,
                uint64_t *out_channel
                );

        /**
         * @see gti::I_CommProtocol::irecv
         */
        GTI_RETURN irecv (
                void* out_buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::test_msg
         */
        GTI_RETURN test_msg (
                unsigned int request,
                int* out_completed,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );

        /**
         * @see gti::I_CommProtocol::wait_msg
         */
        GTI_RETURN wait_msg (
                unsigned int request,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );
    }; /*class CommProtocolTemplate*/
} /*namespace gti*/

#endif /* COMM_PROTOCOL_TEMPLATE_H */
