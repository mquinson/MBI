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
 * @file CProtIPC-SM.h
 *       A shared memory implementation of the communication protocol interface.
 *
 *  This implementation may be used to communicate between processes that share
 *  memory.
 *
 * @author Joachim Protze
 * @date 15.03.2012
 *
 */

#ifndef C_PROT_IPC_SHARED_MEMORY_H
#define C_PROT_IPC_SHARED_MEMORY_H

#include "I_CommProtocol.h"
#include "I_Module.h"
#include "ModuleBase.h"

#include <vector>
#include <list>
#include <map>
#include <sys/types.h>

#define GTI_SHM_SHORT_MSG_SIZE 4050

namespace gti
{

    typedef int pipe_t;

    typedef enum{
        GTI_SHM_SHORT_MSG=0,
        GTI_SHM_LONG_MSG
    }gti_shm_msg_type;

    enum gti_shm_err_type{
        MSGGET_ERROR=-100,
        MSGSND_ERROR,
        MSGRCV_ERROR,
        MSGCTL_ERROR,
        SHMGET_ERROR,
        SHMAT_ERROR,
        SHMDT_ERROR,
        SHMCTL_ERROR,
        INTERNAL_ERROR,
    // highest shm_err_type should be -2
        LAST_SHM_ERROR // < 0 by assert
    };


    struct msgbuf {
        long             mtype;         /* message type, must be > 0 */
        ssize_t          length;        /* length of mtext */
        gti_shm_msg_type type;          /* message or shmem */
        char             mtext[GTI_SHM_SHORT_MSG_SIZE];   /* message data */
    };

    struct msgpointer {
        long             mtype;         /* message type, must be > 0 */
        ssize_t          length;        /* size of shm-block */
        gti_shm_msg_type type;          /* message or shmem */
        pipe_t           shmid;           /* message data */
    };

    struct shmRequest{
        void* out_buf;
        uint64_t num_bytes;
        unsigned int request;
        ssize_t length;
        uint64_t channel;
        int finished;
        bool send;
        shmRequest(unsigned int id, void* out_buf, uint64_t num_bytes, uint64_t channel, bool send=false) : request(id), finished(false), out_buf(out_buf), num_bytes(num_bytes), channel(channel), send(send) {}
        shmRequest(const shmRequest& o) : out_buf(o.out_buf), num_bytes(o.num_bytes), request(o.request), length(o.length), channel(o.channel), finished(o.finished), send(o.send) {}
    };


    /**
     * Class that describes the communication protocol interface.
     */
    class CommProtIpcSM : public ModuleBase<CommProtIpcSM, I_CommProtocol>
    {
    protected:
        bool initialized;
        bool finalized;
        bool isTop;
        bool myIsIntra;

        std::vector<pipe_t>   localPipes;
        std::vector<key_t>  localKeys;

        std::vector<pipe_t> remotePipes;
        std::vector<key_t> remoteKeys;
        std::vector<key_t>   openLongSendKeys;

        int numPartners;
        int mySeed;

        std::map<key_t, int> key2Channel;

        GTI_DISTRIBUTION distrib;
        int blocksize;
        uint64_t gtiOwnLevel;
        uint64_t remoteTierSize;
        uint64_t tierSize;
        uint64_t selfID;
        int commId;
        uint64_t myPlaceId;
        char commSide;

        std::map<int, shmRequest*> requestMap;
        unsigned int requestId;


        /**
         * initialize connections
         */
        void connect();
        ssize_t recv_wrapper ( void* out_buf, size_t num_bytes, uint64_t channel, uint64_t* out_channel, int msgflg=0 );
    public:

        /**
         * Constructor.
         * @ref ModConf - The module configuration syntax
         * @param intanceName name of the module instance.
         */
        CommProtIpcSM (const char* instanceName);

        /**
         * Destructor.
         */
        ~CommProtIpcSM (void);

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
    }; /*class CommProtIpcSM*/
} /*namespace gti*/

#endif /* C_PROT_SHARED_MEMORY_H */
