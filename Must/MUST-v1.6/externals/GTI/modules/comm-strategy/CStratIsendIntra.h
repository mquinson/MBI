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
 * @file CStratSimpleIntra.h
 *       @see I_CommStrategyIntra.
 *
 * @author Tobias Hilbrich
 * @date 16.01.2012
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyIntra.h"
#include "I_CommProtocol.h"
#include "ModuleBase.h"
#include "CStratQueue.h"
#include "CStratIsend.h"

#ifndef CSTRAT_SIMPLE_INTRA_H
#define CSTRAT_SIMPLE_INTRA_H

namespace gti
{
    /**
     * Simple implementation of intra communication strategy.
     * Properties:
     * - no aggregation
     * - uses isend to send
     */
    class CStratIsendIntra : public ModuleBase<CStratIsendIntra, CStratIntraQueue>, public CStratIsend
    {
    protected:
        /** The communication protocol. */
        I_CommProtocol *protocol;

        //Receive  - irecv data
        unsigned int myRequest;
        uint64_t myBuf[2];

        ////Data to implement communicationFinished
        //Tokens
        static const uint64_t myTokenUpdate;
        static const uint64_t myTokenAcknowledge;

        //For everyone
        long myNumMsgsSent; //Number of sends we had
        long myNumMsgsReceived; //Number of successful tests or waits we had
        std::map <int, std::list<CStratQueueItem> > myReceivedUnexpectedMessages;

        //For Master
        long mySumDiffs; //Intermediate sum of received updates
        int myNumUpdates;
        std::vector<bool> myChannelState;

        bool myCommFinished;

        /**
         * Handles a length token and receives the described message.
         * @param tokenMessage the token message (2 uint64_ts).
         * @param channel from which we got the token message.
         * @param outNumBytes pointer to storage for actual message length.
         * @param outBuf pointer to void* that is set to a buffer that contains the message.
         * @param outFreeData data to use when we free the buffer.
         * @param outBufFreeFunction function to free the returned buffer.
         */
        GTI_RETURN handleReceivedMessageToken (
                uint64_t* tokenMessage,
                uint64_t channel,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
                );

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
         * Internal flush implementation, use block=true to block till completion.
         * @param block wait till completion?
         * @return GTI_SUCCESS iff successful.
         */
        GTI_RETURN flush (bool block);

        /**
         * Waits until the first send request in myRequests is completed.
         * It will not only wait for the communication but also check for
         * incoming messages to allow partners to continue their execution
         * if they also stall for a request completion.
         */
        void finishFirstSendRequest (void);

    public:

        /**
         * Constructor.
         * @ref ModConf see for details on module configuration.
         * @param instanceName name of the module instance.
         */
        CStratIsendIntra (const char* instanceName);

        /**
         *Destructor.
         */
        ~CStratIsendIntra (void);

        /**
         * @see I_CommStrategyIntra::shutdown
         */
        GTI_RETURN shutdown (GTI_FLUSH_TYPE flush_behavior);

        /**
         * @see I_CommStrategyIntra::communicationFinished
         */
        GTI_RETURN communicationFinished (bool *pOutIsFinished);

        /**
         * @see I_CommStrategyIntra::getNumPlaces
         */
        GTI_RETURN getNumPlaces (uint64_t *outNumPlaces);

        /**
         * @see I_CommStrategyIntra::getOwnPlaceId
         */
        GTI_RETURN getOwnPlaceId (uint64_t *outId);

        /**
         * @see I_CommStrategyIntra::send
         */
        GTI_RETURN send (
                uint64_t toPlace,
                void* buf,
                uint64_t numBytes,
                void* freeData,
                GTI_RETURN (*bufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        );

        /**
         * @see I_CommStrategyIntra::test
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
         * @see I_CommStrategyIntra::wait
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
        GTI_RETURN flush (void);

        /**
         * @see I_PanicListener::flushAndSetImmediate
         */
        GTI_RETURN flushAndSetImmediate (void);
    };
}

#endif /* CSTRAT_SIMPLE_INTRA_H */
