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
 * @file CStratThreadedIntra.h
 *        @see CStratThreadedIntra.
 *
 * @author Tobias Hilbrich
 * @date 12.04.2012
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyIntra.h"
#include "ModuleBase.h"
#include "CStratThreaded.h"
#include "CStratQueue.h"

#ifndef CSTRAT_THREADED_INTRA_H
#define CSTRAT_THREADED_INTRA_H

namespace gti
{
    /**
     * Implementation of I_CommStrategyIntra that aggregates multiple messages into one larger
     * buffer in order to achieve higher bandwith.
     */
    class CStratThreadedIntra : public ModuleBase<CStratThreadedIntra, CStratIntraQueue>, public CStratThreadedAggregator, public CStratAggregateReceiver
    {
    protected:
        /** The communication protocol. */
        I_CommProtocol *protocol;

        //==Additional tokens
        static const uint64_t myTokenUpdate;
        static const uint64_t myTokenAcknowledge;

        //==Information for testing whether all communication in the layer has finished
        //For everyone
        long myNumMsgsSent; //Number of sends we had
        long myNumMsgsReceived; //Number of successful tests or waits we had
        std::map <int, std::list<CStratQueueItem> > myReceivedUnexpectedMessages; //maps channel from which we got it to the buffer (either an aggregate OR a long message, long messages will have a free_data of "(void*)1")

        //For Master
        long mySumDiffs; //Intermediate sum of received updates
        int myNumUpdates;
        std::vector<bool> myChannelState;

        bool myCommFinished;
        bool myAggregationAllowed;

        /**
         * Checks whether an unexpected message exists that was received
         * during communicationFinished, if so it puts the first of these into
         * the given fields for a retrieved message. If not it does not touches
         * the fields (except outFlag) and returns false.
         *
         * For params see CstratIsendIntra::test
         * @return true if a unexpected message was available, false otherwise.
         */
        bool handleUnexpectedMessagesForReceive (
                int* outFlag,
                uint64_t *outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        );

        /**
         * Handles an unexpeced update, the master may receive
         * these.
         * @param buf token message of the update
         */
        void handleUnexpectedUpdate (uint64_t* buf, uint64_t channel);

        /**
         * Internal implementation of flush, offers choice of to block for completion of flushed messages or not.
         */
        GTI_RETURN flush (bool block);

        /**
         * @see CStratThreadedAggregator::completeOutstandingSendRequest
         */
        void completeOutstandingSendRequest (bool useMyRequests, unsigned int request);

    public:
        /**
         * Constructor.
         * @ref ModConf see for details on module configuration.
         * @param instanceName name of this module instance.
         */
        CStratThreadedIntra (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~CStratThreadedIntra ();

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN shutdown (
                GTI_FLUSH_TYPE flush_behavior
        );

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN communicationFinished (
                bool *pOutIsFinished
        );

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN getNumPlaces (uint64_t *outNumPlaces);

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN getOwnPlaceId (uint64_t *outId);

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN send (
                uint64_t toPlace,
                void* buf,
                uint64_t numBytes,
                void* freeData,
                GTI_RETURN (*bufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        );

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN test (
                int* outFlag,
                uint64_t *outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        );

        /**
         * @see I_CommStrategyIntra::
         */
        GTI_RETURN wait (
                uint64_t* outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        );

        /**
         * @see I_PanicListener::flush
         */
        GTI_RETURN flush (
                void
        );

        /**
         * @see I_PanicListener::flushAndSetImmediate
         */
        GTI_RETURN flushAndSetImmediate (void);
    };
}

#endif /* CSTRAT_THREADED_INTRA_H */
