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
 * @file I_CommStrategyUp.h
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
#include "I_PanicListener.h"

#ifndef I_COMM_STRATEGY_UP_H
#define I_COMM_STRATEGY_UP_H

namespace gti
{
    /**
     * Extensive documentation see:
     * @see I_CommStrategyDown
     */
    class I_CommStrategyUp : public I_PanicListener
    {
    public:

    	/**
    	 * Virtual destructor.
    	 */
    	virtual ~I_CommStrategyUp () {}

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
                GTI_SYNC_TYPE sync_behavior
                ) = 0;

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
         * Sends a buffer towards the root of the network tree.
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
         * @todo No status for asynchronous messages available,
         *       possible options are (1) a callback mechanism were the
         *       comm strategy calls its owner when something goes wrong
         *       (2) a request mechanism were each send call gives a
         *       request that may be queried for success (3) errors are
         *       returned at the next call to the comm strategy and no
         *       send status is available. Do we need such a status ?
         */
        virtual GTI_RETURN send (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                ) = 0;

        /**
         * Tests for a messages from the upwards direction.
         * If successful, out_flag is set to true and the
         * remaining arguments are set, otherwise, out_flag
         * is set to false and the other arguments are not
         * modified.
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
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN test (
                int* out_flag,
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                ) = 0;

        /**
         * Receives a message from the upwards direction and blocks
         * until a message is available.
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
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN wait (
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                ) = 0;

        /**
         * Raises and broadcasts the presence of a panic. Should be called
         * if the application ran into an error/signal and is about to abort.
         * Its behavior should include:
         * - raise GTI's implicit panic event
         * - flush any outstanding communications
         * - switch this module to only use immediate communication
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN raisePanic (
                void
        ) = 0;
    };
}

#endif /* I_COMM_PROTOCOL_H */
