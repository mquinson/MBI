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
 * @file CProtSimpleTcp.h
 *       Simple TCP based implementation of the communication
 *       protocol interface.
 *
 * @author Tobias Hilbrich
 * @date 21.04.2009
 */

#include "I_CommProtocol.h"
#include "ModuleBase.h"
#include <list>
#include <map>
#include <vector>

#ifndef CPROT_SIMPLE_TCP_H
#define CPROT_SIMPLE_TCP_H

namespace gti
{
    /**
     * Class that describes the communication protocol interface.
     */
    class CProtSimpleTCP : public ModuleBase<CProtSimpleTCP, I_CommProtocol>
    {
    protected:

        static const uint64_t TOKEN_MSG_SIZE_HEADER;
        static const uint64_t TOKEN_SYNC_MSG;

        uint64_t myPlaceId;
        unsigned int     myNumSockets; /**< Number of sockets. */
        int              *mySockets; /**< The communication sockets. */
        typedef std::list<std::pair<uint64_t,void*> > MsgsList;
        std::vector<MsgsList> myOutstandingMsgs;
        bool 			 myIsShutdown;

        typedef std::pair < uint64_t, std::pair<void*, uint64_t> >RequestInfo; //timestamp, buffer, max length for buffer (bytes)
        typedef std::pair <unsigned int, RequestInfo > Request;
        std::vector<std::list< Request > > myActiveRequests;
        typedef std::pair<uint64_t, uint64_t> CompletionInfo; //channel, received length
        std::map <unsigned int, CompletionInfo> myCompletedRequests;
        unsigned int myNextRequestID;

        /**
         * Starts the connection with the communication partners.
         */
        GTI_RETURN startup (void);

        GTI_RETURN recv_msg_content (uint64_t channel, uint64_t size, void* target_buffer, bool *target_was_used, bool *request_was_used, unsigned int *request_finished);

    public:

        /**
         * Constructor.
         * @ref ModConf - The module configuration syntax
         * @param instanceName name of this module instance
         */
        CProtSimpleTCP (const char* instanceName);

        /**
         * Destructor.
         */
        ~CProtSimpleTCP (void);

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
         * @see I_CommProtocol::getNumChannels
         */
        GTI_RETURN getNumChannels (uint64_t* out_numChannels);

        /**
         * @see gti::I_CommProtocol::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see I_CommProtocol::removeOutstandingRequests
         */
        GTI_RETURN removeOutstandingRequests (void);

        /**
         * @see I_CommProtocol::shutdown
         */
        GTI_RETURN shutdown (void);

        /**
         * @see I_CommProtocol::ssend
         */
        GTI_RETURN ssend (
                void* buf,
                uint64_t num_bytes,
                uint64_t channel
                );

        /**
         * @see I_CommProtocol::isend
         */
        GTI_RETURN isend (
                void* buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see I_CommProtocol::recv
         */
        GTI_RETURN recv (
                void* out_buf,
                uint64_t num_bytes,
                uint64_t* out_length,
                uint64_t channel,
                uint64_t *out_channel
                );

        /**
         * @see I_CommProtocol::irecv
         */
        GTI_RETURN irecv (
                void* out_buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see I_CommProtocol::test_msg
         */
        GTI_RETURN test_msg (
                unsigned int request,
                int* out_completed,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );

        /**
         * @see I_CommProtocol::wait_msg
         */
        GTI_RETURN wait_msg (
                unsigned int request,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );
    }; /*class CProtSimpleTCP*/
} /*namespace gti*/

#endif /* CPROT_SIMPLE_TCP_H */
