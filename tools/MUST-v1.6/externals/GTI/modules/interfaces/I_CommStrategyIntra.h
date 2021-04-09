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
 * @file I_CommStrategyIntra.h
 *       @see I_CommStrategyIntra.
 *
 * @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian H�nsel
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"
#include "I_PanicListener.h"

#ifndef I_COMM_STRATEGY_INTRA_H
#define I_COMM_STRATEGY_INTRA_H

namespace gti
{
    /**
     * An interface for intra layer communication, i.e., from one place to another
     * within the same tool layer.
     *
     * The interface offers all features, an Implementation
     *  may not support certain features, which should be
     *  listed in the communication strategy descriptions.
     *
     * @see I_CommStrategyDown
     */
    class I_CommStrategyIntra : public I_PanicListener
    {
    public:

        /**
         * Virtual destructor.
         */
        virtual ~I_CommStrategyIntra () {}

        /**
         * Shuts the communication strategy down and
         * releases the communication protocols.
         *
         * @param flush_behavior the behavior to use for outstanding
         *        messages.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN shutdown (
                GTI_FLUSH_TYPE flush_behavior
                ) = 0;

        /**
         * Can be called before a call to shutdown to determine whether
         * any outstanding messages exist in the intra communication
         * system. This is important as a place in a layer may not know
         * which other places still want to send messages to it.
         *
         * This call marks the place as "done", which means it is supposed
         * to not perform any additional communication. If the memory behind
         * the given pointer to bool is set to true, no other place intends to
         * send any message to this place.
         *
         * This call will almost certainly involve communication for its
         * implementation. It may block until all other places on the
         * layer also call this routine.
         *
         * Use successive calls to this function if it returned that
         * outstanding messages exist.
         *
         * Note: the implementation of this function may not be able to
         *          determine whether any outstanding messages are for this
         *          place or for another. Thus, eve if the returned bool value
         *          inidicates outstanding communications it may happen that
         *          this place receives no additional messages.
         *
         * @param pOutIsFinished pointer to storage for a bool value, storage is
         *               set to true if no place intends to send this place any intra
         *               message, is set to false otherwise.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN communicationFinished (
                bool *pOutIsFinished
        ) = 0;

        /**
         * Returns the total number of places in this layer.
         * @param outNumPlaces address to storage that will hold the number
         *              of places within the layer.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN getNumPlaces (uint64_t *outNumPlaces) = 0;

        /**
         * Returns the id (0-TierSize) of this place.
         * @param outId address to storage that will hold the id.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN getOwnPlaceId (uint64_t *outId) = 0;

        /**
         * Sends a buffer to a different place within the layer.
         *
         * @param toPlace place id of destination, communication from a
         *              place to itself if forbidden.
         * @param buf
         *      the data to be sent.
         *      For MPI trace records this data has to contain
         *      the thread and process id from the creator of the
         *      data.
         * @param numBytes
         *      number of bytes of the data.
         * @param freeData
         *      State data associated with the buffer, data is passed
         *      to the free function also given to the call.
         *      Rational: this data may for example contain a
         *                pointer to a struct with a reference count and
         *                a mutex to implement a reference count mechanisms
         *                with thread synchronization in the buf_free_function.
         * @param bufFreeFunction
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
                uint64_t toPlace,
                void* buf,
                uint64_t numBytes,
                void* freeData,
                GTI_RETURN (*bufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
                ) = 0;

        /**
         * Tests for a messages from another channel.
         *
         * @param outFlag
         *   Set to true if a message is available, false otherwise.
         * @param outFromPlace
         *   Channel id of the sender.
         * @param outNumBytes
         *   Set to contain the length of the message.
         * @param outBuf
         *   Set to contain the message data.
         * @param outFreeData
         *   Set to contain information needed to free the buffer.
         * @param outBufFreeFunction
         *   Function to be called to free the received buffer.
         *   It takes the outFreeFata, the buffer and its length
         *   as input. This function is provided by the communication
         *   strategy.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN test (
                int* outFlag,
                uint64_t *outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
                ) = 0;

        /**
         * Receives a message from another channel, if no message is available,
         * it blocks until such a message arrives.
         *
         * @param outFromPlace
         * @param outNumBytes
         *   Set to contain the length of the message.
         * @param outBuf
         *   Set to contain the message data.
         * @param outFreeData
         *   Set to contain information needed to free the buffer.
         * @param outBufFreeFunction
         *   Function to be called to free the received buffer.
         *   It takes the outFreeData, the buffer and its length
         *   as input. This function is provided by the communication
         *   strategy.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         */
        virtual GTI_RETURN wait (
                uint64_t* outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
                ) = 0;
    }; /*I_CommStrategyIntra*/
} /*namespace gti*/

#endif /* I_COMM_STRATEGY_INTRA_H */
