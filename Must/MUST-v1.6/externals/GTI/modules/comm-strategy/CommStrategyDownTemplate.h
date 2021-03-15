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
 * @file CommStrategyDownTemplate.h
 *       Implementation for the I_CommStrategyDown interface.
 *
 *  This implementation may be used as a template for other implementations.
 *
 * @author Tobias Hilbrich
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyDown.h"
#include "I_CommProtocol.h"
#include "I_Module.h"
#include "ModuleBase.h"

#ifndef COMM_STRATEGY_DOWN_TEMPLATE_H
#define COMM_STRATEGY_DOWN_TEMPLATE_H

namespace gti
{
    /**
     * Class that describes the communication protocol interface.
     */
    class CommStrategyDownTemplate : public ModuleBase<CommStrategyDownTemplate, I_CommStrategyDown>
    {
    protected:
        /** The communication protocol. */
        I_CommProtocol *protocol;

    public:

        /**
         * Constructor.
         * @ref ModConf see for details on module configuration.
         * @param instanceName name of this module.
         */
        CommStrategyDownTemplate (const char* instanceName);

        /**
         *Destructor.
         */
        ~CommStrategyDownTemplate (void);

        /**
         * @see I_CommStrategyDown::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see I_CommStrategyDown::shutdown
         */
        GTI_RETURN shutdown (
                GTI_FLUSH_TYPE flush_behavior,
                GTI_SYNC_TYPE sync_behavior);

        /**
         * @see I_CommStrategyDown::registerNewClientCallback
         */
        GTI_RETURN registerNewClientCallback (void(*fun)(void), bool& isUsed) {if(!protocol) return GTI_ERROR_NOT_INITIALIZED; return protocol->registerNewClientCallback(fun, isUsed);}

        /**
         * @see I_CommStrategyDown::getNumClients
         */
        GTI_RETURN getNumClients (uint64_t *outNumClients);

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
                GTI_RETURN (**out_buf_fee_function) (void* free_data, uint64_t num_bytes, void* buf),
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

#endif /* COMM_PROTOCOL_TEMPLATE_H */
