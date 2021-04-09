/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"
#include "GtiHelper.h"
#include "ModuleBase.h"
#include "I_CommProtocol.h"
#include "ThreadChannel.h"
#include <pthread.h>
#include <sched.h>
#include <vector>

#ifndef C_PROT_SM_QUEUE_H
#define	C_PROT_SM_QUEUE_H

namespace gti {

    /***
     * This class represents a request (either send/receive)
     * it's instances are collected in a container and processed
     * by the tool-thread for sending/receiving.
     */
    class SMQueueRequest {
    public:
        SMQueueRequest( bool is_send = false ) : send(is_send), completed(false) {}
        ~SMQueueRequest() {}
        unsigned int requestid;
        unsigned int channel;
        BYTE *ptr;
        unsigned int size;
        // Is this a send or a receive request?
        bool send;
        bool completed;
    };

    /***
     * This class helps the tool thread to remember
     * all of the (yet) constructed thread-channels.
     */
    class SMQueueTIB
    {
    public:
        unsigned int thread_id;
        ThreadChannel *channel;
        std::vector<SMQueueRequest*> m_Requests;
        pthread_mutex_t m_RequestLock;
        bool toolthread;
        SMQueueTIB() : toolthread(false) {pthread_mutex_init (&m_RequestLock, NULL);}
    };

    class CProtSMQueue : public ModuleBase<CProtSMQueue, I_CommProtocol>, public GtiHelper {
    public:
        CProtSMQueue( const char *instanceName );
        ~CProtSMQueue(void);

        void start();
        void finish();

        /**
         * @see gti::I_CommProtocol::isConnected
         */
        bool isConnected (void);

        /**
         * @see gti::I_CommProtocol::isInitialized
         */
        bool isInitialized (void);

        /**
         * @see gti::I_CommProtocol::isFinalized
         */
        bool isFinalized (void);

        /**
         * @see gti::I_CommProtocol::getNumChannels
         */
        GTI_RETURN getNumChannels (uint64_t* out_numChannels);

        /**
         * @see gti::I_CommProtocol::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see gti::I_CommProtocol::shutdown
         */
        GTI_RETURN shutdown (void);

        /**
         * @see gti::I_CommProtocol::removeOutstandingRequests
         */
        GTI_RETURN removeOutstandingRequests (void);

        /**
         * @see gti::I_CommProtocol::ssend
         */
        GTI_RETURN ssend (
                void* buf,
                uint64_t num_bytes,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::isend
         */
        GTI_RETURN isend (
                void* buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::recv
         */
        GTI_RETURN recv (
                void* out_buf,
                uint64_t num_bytes,
                uint64_t* out_length,
                uint64_t channel,
                uint64_t *out_channel
                );

        /**
         * @see gti::I_CommProtocol::irecv
         */
        GTI_RETURN irecv (
                void* out_buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see gti::I_CommProtocol::test_msg
         */
        GTI_RETURN test_msg (
                unsigned int request,
                int* out_completed,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );

        /**
         * @see gti::I_CommProtocol::wait_msg
         */
        GTI_RETURN wait_msg (
                unsigned int request,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );

//        static void ModuleStartUp();
        static pthread_once_t key_once;
        static void make_key();
    private:
        static bool m_Terminate;
        static pthread_mutex_t m_TIBLock;
        static std::vector<SMQueueTIB*> m_TIBs;

        static pthread_key_t m_KeyBuffer;
        static unsigned int lastRequestId;
        static unsigned int lastThreadId;

        static unsigned int getNextRequestId();
        static SMQueueTIB *getTIB();
        static SMQueueTIB *getToolTIB();
        SMQueueTIB *getAppTIB( int channel );
        bool freeTIB();
        SMQueueRequest *pushRequest(unsigned int channel, void* buf, uint64_t num_bytes, bool send);

        SMQueueRequest *signalRemoteRequest( int channel, unsigned int req );
        SMQueueRequest *findRequestLocal( unsigned int request, bool remove = true );

        GTI_RETURN WaitMessageInternal(
                unsigned int request,
                uint64_t* out_receive_length,
                uint64_t* out_channel,
                int* out_completed,
                bool test_only
         );

        GTI_RETURN RecvInternal(
                SMQueueRequest *req,
                uint64_t* out_receive_length,
                uint64_t* out_channel,
                int* out_completed,
                bool block
        );

    protected:
        static void start_worker_thread(void);
        static void* threadProc( void *thread_data );
        static pthread_t m_Thread;
        static pthread_once_t thread_starter;
    };
}

#endif	/* C_PROT_SM_QUEUE_H */

