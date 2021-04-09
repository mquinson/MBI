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
 * @file CStratSimpleDown.h
 *       Implementation for the I_CommStrategyDown interface.
 *
 * Sends all messages immediately and blocking.
 *
 * @author Tobias Hilbrich
 * @date 15.04.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyDown.h"
#include "I_CommProtocol.h"
#include "ModuleBase.h"
#include "CStratQueue.h"

#include <list>

#ifndef CSTRAT_SIMPLE_DOWN_H
#define CSTRAT_SIMPLE_DOWN_H

namespace gti
{
    /**
     * Helper for managing outstanding communications.
     */
    struct SimpleDownRequestInfo
    {
        unsigned int request;
        std::list<unsigned int> requests; /**< if non empty list these are the real requests, otherwise the above is the request.*/
        void* buf;
        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf);
        void* free_data;
        int num_bytes;
    };

    /**
     * Simple implementation of upwards strategy.
     * Properties:
     * - no aggregation
     * - uses ssend to send (upwards and downwards)
     * - sends an acknowledgment after a received message was processed
     */
    class CStratSimpleDown : public ModuleBase<CStratSimpleDown, CStratDownQueue>
    {
    protected:
        /** The communication protocol. */
        I_CommProtocol *protocol;

        unsigned int  test_request;
        uint64_t test_buff[2];
        static const uint64_t myTokenShutdownSync;
        static const uint64_t myTokenMessage;
        static const uint64_t myTokenAck;

        std::list<SimpleDownRequestInfo> myBcastReqs;

        /**
         * Completes requests in myBcastReqs that result from
         * calls to broadcast.
         * If block is false it will start with the oldest request and
         * test for completion till the first request is still active,
         * afterwards it returns.
         * If block is true, it completes all requests.
         * @param block conrolls whether completion is enforced.
         */
        void completeBcastRequests (bool block);

    public:

        /**
         * Constructor.
         * @ref ModConf see for details on module configuration.
         * @param instanceName name of the module instance.
         */
        CStratSimpleDown (const char* instanceName);

        /**
         *Destructor.
         */
        ~CStratSimpleDown (void);

        /**
         * @see I_CommStrategyDown::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see I_CommStrategyDown::getNumClients
         */
        GTI_RETURN getNumClients (uint64_t *outNumClients);

        /**
         * @see I_CommStrategyDown::registerNewClientCallback
         */
        GTI_RETURN registerNewClientCallback (void(*fun)(void), bool& isUsed) {if(!protocol) return GTI_ERROR_NOT_INITIALIZED; return protocol->registerNewClientCallback(fun, isUsed);}

        /**
         * @see I_CommStrategyDown::shutdown
         */
        GTI_RETURN shutdown (
                GTI_FLUSH_TYPE flush_behavior,
                GTI_SYNC_TYPE sync_behavior);

        /**
         * @see I_CommStrategyDown::broadcast
         */
        GTI_RETURN broadcast (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                );

        /**
         * @see I_CommStrategyDown::test
         */
        GTI_RETURN test (
                int* out_flag,
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t *outChannel,
                uint64_t preferChannel
                );

        /**
         * @see I_CommStrategyDown::acknowledge
         */
        GTI_RETURN acknowledge (
        		uint64_t channel
        );

        /**
         * @see I_CommStrategyDown::wait
         */
        GTI_RETURN wait (
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_fee_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t *outChannel
                );

        /**
         * @see I_CommStrategyDown::flush
         */
        GTI_RETURN flush (
                void
                );
    };
}

#endif /* CSTRAT_SIMPLE_DOWN_H */
