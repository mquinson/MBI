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
 * @file CStratThreadedUp.h
 *        Application phase aware communication strategy.
 *
 * Provides the interface functionality with an implementation
 * that tries to avoid any comm. during phases in which the
 * application is communicating.
 *
 * @author Tobias Hilbrich
 * @date 24.08.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyUp.h"
#include "ModuleBase.h"
#include "CStratThreaded.h"
#include "CStratQueue.h"

#ifndef CSTRAT_THREADED_UP_H
#define CSTRAT_THREADED_UP_H

namespace gti
{
    /**
     * Class that describes the communication protocol interface.
     */
    class CStratThreadedUp : public ModuleBase<CStratThreadedUp, CStratUpQueue>, public CStratThreadedAggregator
    {
    protected:
        /** The communication protocol. */
        I_CommProtocol *protocol;

        uint64_t myBuf[2];
        unsigned int myRequest;
        bool myGotPing;
        bool myAggregationAllowed;

        std::list<CStratQueueItem> myReceivedUnexpectedMessages; //List of unexpected messages

        /**
         * @see CStratThreadedAggregator::completeOutstandingSendRequest
         */
        void completeOutstandingSendRequest (bool useMyRequests, unsigned int request);

        /**
         * Handles available unexpected during test/wait.
         */
        bool handleUnexpectedMessagesForReceive (
                int* outFlag,
                uint64_t *outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        );


    public:

        /**
         * Constructor.
         * @ref ModConf see for details on module configuration.
         * @param instanceName name of this module instance.
         */
    	CStratThreadedUp (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~CStratThreadedUp ();

        /**
         * @see I_CommStrategyUp::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see I_CommStrategyUp::shutdown
         */
        GTI_RETURN shutdown (
                GTI_FLUSH_TYPE flush_behavior,
                GTI_SYNC_TYPE sync_behavior
                );

        /**
         * @see I_CommStrategyUp::send
         */
        GTI_RETURN send (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                );

        /**
         * @see I_CommStrategyUp::test
         */
        GTI_RETURN test (
                int* out_flag,
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                );

        /**
         * @see I_CommStrategyUp::wait
         */
        GTI_RETURN wait (
                uint64_t* out_num_bytes,
                void** out_buf,
                void** out_free_data,
                GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                );

        /**
         * @see I_CommStrategyUp::raisePanic
         */
        GTI_RETURN raisePanic (
                void
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

#endif /* CSTRAT_THREADED_UP_H */
