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
 * @file I_CommStrategyDown.h
 *       Interface for communication strategy modules.
 *
 *  The interface offers all features, an Implementation
 *  may not support certain features, which should be
 *  listed in the communication strategy descriptions.
 *
 * @author Tobias Hilbrich
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"

#ifndef I_COMM_STRATEGY_DOWN_H
#define I_COMM_STRATEGY_DOWN_H

namespace gti
{
    /**
     * A downwards directed communication strategy.
     * It connects two tiers of application
     * and or tool places. The application is the lower most layer and
     * all other layers are above this layer. Usually event trace data
     * will be sent upwards. When two layers are connected the lower one will
     * use a I_CommStrategyUp and the upper one a I_CommStrategyDown.
     *
     * Each pair of strategies are connected with a communication protocol
     * (gti::I_CommProtocol). The strategy decides when to send something
     * whereas a protocol decides how to send something. This offers a
     * wide flexibility for the communication system.
     *
     * A strategy is initialized when its constructor is called. It will
     * query for its communication protocol at that time. It may happen
     * that a communication protocol is not initialized at that time.
     * In such a case all calls except
     * gti::I_CommStrategyDown::broadcast and gti::I_CommStrategyUp::send
     * should return a GTI_ERROR_NOT_INITIALIZED in such a case.
     * The broadcast and send calls should buffer the data to be sent and
     * handle it once the communication protocol becomes available.
     *
     * Rational: For trace data this is a convenient mechanism to allow
     *           simple wrapper implementations. If a protocol is not yet
     *           available all trace data may still be given to strategies
     *           without the need of some buffering mechanism. The trace
     *           data will be sent once the protocol is initialized, which
     *           is usually acceptable.
     */
    class I_CommStrategyDown : public I_Module
    {
    public:

    	/**
    	 * Virtual destructor.
    	 */
    	virtual ~I_CommStrategyDown () {}

        /**
         * Shuts the communication strategy down and
         * releases the communication protocols.
         *
         * @param flush_behavior the behavior to use for outstanding
         *        messages.
         * @param sync_behavior specifies whether a synchronized
         *        shutdown between both communication partners is
         *        desired or not.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN shutdown (
                GTI_FLUSH_TYPE flush_behavior,
                GTI_SYNC_TYPE sync_behavior) = 0;

        /**
         * Returns the total number of clients connected
         * to this comm strategy.
         * @param outNumClients address of a value that will hold
         *                      the output.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN getNumClients (uint64_t *outNumClients) = 0;

        /**
         * Returns the id of this place within its layer, a value between
         * 0 and tier-size-1.
         *
         * @param outPlaceId id of place.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN getPlaceId (uint64_t* outPlaceId) = 0;

        /**
         * The callback `fun` is called, whenever a new client connected
         * 
         * @param fun provides a function pointer to be called when new clients show up
         * @param isUsed returns information whether the callback will be called at any time
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN registerNewClientCallback (void(*fun)(void), bool& isUsed) = 0;

        /**
         * Sends a buffer downwards in the network tree.
         *
         * @param buf
         *      the data to be sent.
         *      For MPI trace records this data has to contain
         *      the thread and process id from the creator of the
         *      data.
         * @param num_bytes
         *      number of bytes of the data.
         * @param free_data
         *      State data associated with the buffer, data is passed
         *      to the free function also given to the call.
         *      Rational: this data may for example contain a
         *                pointer to a struct with a reference count and
         *                a mutex to implement a reference count mechanisms
         *                with thread synchronization in the buf_free_function.
         * @param buf_free_function
         *      A function to free the data buffer.
         *      The free_data has to be passed to this function.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error.
         *
         * @todo Either specify this call as non-blocking or create a second
         *       none blocking send, is this actually necessary ?.
         *
         */
        virtual GTI_RETURN broadcast (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                ) = 0;

        /**
         * Tests for a messages from the downwards direction.
         * If successful, out_flag is set to true and the
         * remaining arguments are set, otherwise, out_flag
         * is set to false and the other arguments are not
         * modified.
         *
         * After processing the received message you must call
         * gti::I_CommStrategyDown::acknowledge.
         *
         * @param out_flag
         *   Set to true if a message is available, false otherwise.
         * @param out_num_bytes
         *   Set to contain the length of the message.
         * @param out_buf
         *   Set to contain the message data.
         * @param out_free_data
         *   Set to contain information needed to free the buffer.
         * @param out_buf_free_function
         *   Function to be called to free the received buffer.
         *   It takes the out_free_data, the buffer and its length
         *   as input. This function is provided by the communication
         *   strategy.
         * @param outChannel the channel from which the message was received,
         *               only set if not NULL and the test was successful.
         * @param preferChannel channel number from to which a test is prefereable.
         *              The implementation may return a message from a different channel,
         *              even if this value is specified. A good implementation should try
         *              to only test for a message from the given channel.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN test (
                int* out_flag,
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t *outChannel,
                uint64_t preferChannel = RECV_ANY_CHANNEL
                ) = 0;

        /**
         * Receives a message from the downwards direction and blocks
         * until a message is available.
         *
         * After processing the received message you must call
         * gti::I_CommStrategyDown::acknowledge.
         *
         * @param out_num_bytes
         *   Set to contain the length of the message.
         * @param out_buf
         *   Set to contain the message data.
         * @param out_free_data
         *   Set to contain information needed to free the buffer.
         * @param out_buf_free_function
         *   Function to be called to free the received buffer.
         *   It takes the out_free_data, the buffer and its length
         *   as input. This function is provided by the communication
         *   strategy.
         * @param outChannel the channel from which the message was received,
         *               only set if not NULL and the test was successful.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         *
         */
        virtual GTI_RETURN wait (
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t *outChannel
                ) = 0;

        /**
         * Notifies the strategy of a successfully processed message that was
         * previously received from the given channel.
         *
         * For asynchronous strategies this will be a no-op, for synchronous
         * strategies that ensure that sent data was processed before continuing
         * this will notify the sender site that processing was complete.
         */
        virtual GTI_RETURN acknowledge (
        		uint64_t channel
        		) = 0;

        /**
         * Causes the communication strategy to flush all its
         * not yet sent data.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         *
         * @todo Interaction with blocking receives !
         */
        virtual GTI_RETURN flush (
                void
                ) = 0;
    };
}

#endif /* I_COMM_PROTOCOL_H */
