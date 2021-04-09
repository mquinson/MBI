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
 * @file CStratPRecvDown.h
 *        @see gti::CStratPRecvDown
 *
 * Based on CStratThreaded, this is a modified copy!
 *
 * @author Tobias Hilbrich
 * @date 13.03.2013
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyDown.h"
#include "I_CommProtocol.h"
#include "ModuleBase.h"
#include "CStratPRecv.h"
#include "CStratQueue.h"

#ifndef CSTRAT_PRECV_DOWN_H
#define CSTRAT_PRECV_DOWN_H

namespace gti
{
    /**
     * Class that describes the communication protocol interface.
     */
    class CStratPRecvDown : public ModuleBase<CStratPRecvDown, CStratDownQueue>, public CStratPRecv, public CStratBufReceiver
    {
    protected:
        /** The communication protocol. */
        I_CommProtocol *protocol;

        /**
         * Helper class to store information on outstanding broadcasts.
         */
        class OutstandingBroadcastInfo
        {
        public:
            void* buf;
            void* bufLengthMessage;
            uint64_t num_bytes;
            void* free_data;
            GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf);
            std::list<unsigned int> requests;
        };
        std::list<OutstandingBroadcastInfo> myOutstandingBroadcasts; /**< List of outstanding broadcasts.*/

        /**
         * Tries to complete broadcasts.
         * If block=false then the function tests until the first broadcast could not be completed and otherwise
         * it cycles until all broadcasts are finished.
         */
        void tryToCompleteBroadcasts (bool block);

    public:

        /**
         * Constructor.
         * @ref ModConf see for details on module configuration.
         * @param instanceName name of the instance.
         */
        CStratPRecvDown (const char* instanceName);

        /**
         *Destructor.
         */
        ~CStratPRecvDown (void);

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
         * @see I_CommStrategyDown::wait
         */
        GTI_RETURN wait (
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t *outChannel
                );

        /**
         * @see I_CommStrategyDown::acknowledge
         */
        GTI_RETURN acknowledge (
        		uint64_t channel
        );

        /**
         * @see I_CommStrategyDown::flush
         */
        GTI_RETURN flush (
                void
                );
    };
}

#endif /* CSTRAT_PRECV_DOWN_H */
