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
 * @file I_PlaceReceival.h
 *       Interface for receival of trace and log data on a tool place.
 *
 * This interface receives trace and logging data received on a tool
 * place. Implementations will usually be auto generated.
 *
 * @author Tobias Hilbrich
 * @date 15.04.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include "I_Module.h"
#include "I_ChannelId.h"

#ifndef I_PLACE_RECEIVAL_H
#define I_PLACE_RECEIVAL_H

namespace gti
{
    /**
     * Class that describes the communication protocol interface.
     */
    class I_PlaceReceival : public I_Module
    {
    public:

    	/**
    	 * Virtual destructor.
    	 */
    	virtual ~I_PlaceReceival () {}

        /**
         * Receives a record, and analyzes it.
         * The call may cause communication calls,
         * depending on the used forwarding communication
         * strategies used for this module.
         *
         * Control over the given buffer is passed to the receival
         * module that implements this interface. It will free the
         * buffer with the given free function when it does not
         * needs the buffer any longer. The caller must assume
         * that the buffer was already freed when the function
         * returns.
         *
         * I_PlaceReceival::getUpdatedChannelId must be called
         * before this call.
         *
         * This call only processes inter layer communication
         * events.
         *
         * @param buf buffer that contains the trace record.
         * @param num_bytes length of the buffer in bytes.
         * @param free_data data associated with the buffer,
         *        used to store state for it.
         * @param buf_free_function function to call when the
         *        buffer may be freed.
         * @param numRemainingClients count of remaining
         *        places that are connected in the downwards
         *        direction, this number must be decremented
         *        when a shutdown token arrives, if it was
         *        the shutdown token of the last remaining
         *        connected place the receival must forward
         *        the shutdown event to all upwards connected
         *        places.
         * @param recordChannelId channel id of the incoming record
         *        must have been retrieved with a prior call to
         *        getUpdatedChannelId, the id will be freed at some point
         *        after the call returns, if the id is needed for a longer
         *        time its user must copy it.
         * @param outWasChannelIdSuspended pointer to storage for a
         *        boolean value, is set to true if a reduction was triggered for
         *        the record and still needs further data to complete.
         * @param outChannelIdsToOpen pointer to storage for a list of channel ids.
         *        Is used to store which channel ids were
         *        removed from suspension due to a successful reduction.
         *        The channel ids in the list must be freed by the caller of this function.
         *        The list must be empty when calling the function.
         */
        virtual GTI_RETURN ReceiveRecord (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t *numRemainingClients,
                I_ChannelId *recordChannelId,
                bool *outWasChannelIdSuspended,
                std::list<I_ChannelId*> *outChannelIdsToOpen
                ) = 0;

        /**
         * Updates and returns the channel id of a newly received record.
         * The placement driver needs to pass the channel number from which it received
         * the record in order to allow the update.
         * The returned channel id should be used by the placement driver to suspend
         * and reactivate channels.
         *
         * This function must be called for each new received event inter layer
         * communication event, but not for intra layer events, as they have no
         * associated channel id.
         * I_PlaceReceival::ReceiveRecord may
         * only be called after this calls.
         *
         * Further, this call returns whether the event is a finalizer event,
         * if so, and if this is the last missing finalizer call to shut down the place,
         * the placement driver must complete all intra layer communication
         * before passing this event to I_PlaceReceival::ReceiveIntraRecord.
         * I.e. he must call I_CommStrategyIntra::communicationFinished with
         * a successful return value and check for any outstanding intra messages
         * if it returns that intra communication is not yet finished.
         *
         * @param buf of serialized record
         * @param num_bytes size of buffer
         * @param channel from which the placement driver received the record.
         * @param numChannels number of channels connected to this place (total, not number of still active channels!)
         * @param pOutChannelId pointer to storage of channel id interface pointer,
         *               is going to be set to a pointer to the read and updated channel id.
         * @param pOutIsFinalizer pointer to storage for a bool value, is set to true if this is a finalizer
         *               event and to false otherwise.
         * @param pOutIsOutOfOrder pointer to storage for a bool value, is set to true if this event must
         *               be evaluated irrespective of any ongoing aggregation, i.e., must be processed
         *               out of order.
         * @return GTI_SUCCESS if successful.
         */
        virtual GTI_RETURN getUpdatedChannelId (
        		void* buf,
        		uint64_t num_bytes,
        		uint64_t channel,
        		uint64_t numChannels,
        		I_ChannelId** pOutChannelId,
        		bool *pOutIsFinalizer,
        		bool *pOutIsOutOfOrder
        		) = 0;

        /**
         * Receives an intra layer record, and analyzes it.
         *
         * Control over the given buffer is passed to the receival
         * module that implements this interface. It will free the
         * buffer with the given free function when it does not
         * needs the buffer any longer. The caller must assume
         * that the buffer was already freed when the function
         * returns.
         *
         * This call only processes intra layer communication
         * events.
         *
         * @param buf buffer that contains the trace record.
         * @param num_bytes length of the buffer in bytes.
         * @param free_data data associated with the buffer,
         *        used to store state for it.
         * @param buf_free_function function to call when the
         *        buffer may be freed.
         * @param channel from which this record was received.
         */
        virtual GTI_RETURN ReceiveIntraRecord (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t channel
        ) = 0;

        /**
         * Notifies the receival module that the placement driver stopped
         * blocking channels with ongoing reductions and that all ongoing
         * reductions must be notified that they must not succeed in their
         * ongoing reduction. The queued forwards must be executed as
         * a restult.
         *
         * Note: for reductions with semantic restrictions, e.g. a barrier
         *          is executed on ALL processes, the reduction should still
         *          track for the semantic completeness of the aborted reduction
         *          and start its next working reduction afterwards.
         *
         * @return GTI_SUCCESS if successful.
         */
        virtual GTI_RETURN timeOutReductions (void) = 0;

        /**
         * Triggers all analyses that are "continuous".
         *
         * @param usecSinceLastTrigger microseconds since the last issuing of this function.
         * @return GTI_SUCCESS if successful.
         */
        virtual GTI_RETURN triggerContinuous (uint64_t usecSinceLastTrigger) = 0;

        /**
         * Adds all upwards directed communications strategies that the receival
         * module uses to the given list. The strategies are still managed by the
         * receival module and should only be used for communication by the
         * placement driver.
         *
         * Must be called by the placement driver before it starts to poll for
         * any incoming events.
         *
         * Note: if either the receival module or the placement driver is
         *      multithreaded, races could occur on the strategy. If multiple
         *      threads are used by either component, this needs to be enhanced.
         *
         * @param ups pointer to a list to which the strategies are added.
         * @return GTI_SUCCESS if successful.
         */
        virtual GTI_RETURN getUpwardsCommunications (std::list<I_CommStrategyUp*> *ups) = 0;

        /**
         * Called when the placement driver received an event that was broadcasted
         * towards this TBON node. It shall be unpacked and analysed by the
         * receival module.
         *
         * Control over the given buffer is passed to the receival
         * module that implements this interface. It will free the
         * buffer with the given free function when it does not
         * needs the buffer any longer. The caller must assume
         * that the buffer was already freed when the function
         * returns.
         *
         * This call only processes broadcast events.
         *
         * @param buf buffer that contains the trace record.
         * @param num_bytes length of the buffer in bytes.
         * @param free_data data associated with the buffer,
         *        used to store state for it.
         * @param buf_free_function function to call when the
         *        buffer may be freed.
         * @param pOutWasFinalizeEvent true if this event indicated
         *               that the place can be shut down.
         * @return GTI_SUCCESS if successful.
         */
        virtual GTI_RETURN ReceiveBroadcastRecord (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                bool *pOutWasFinalizeEvent) = 0;
    };
}

#endif /* I_PLACE_RECEIVAL_H */
