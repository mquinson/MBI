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
 * @file CStratPRecv.h
 *        @see gti::CStratPRecv.
 *
 * Based on CStratThreaded, this is a modified copy!
 *
 * @author Tobias Hilbrich
 * @date 13.03.2013
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommProtocol.h"
#include "CStratIsend.h"

#include <list>
#include <vector>
#include <map>

#ifndef CSTRAT_PRECV_H
#define CSTRAT_PRECV_H

namespace gti
{
    /**
     * Buffer free function for long messages.
     */
    GTI_RETURN longMsgBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf);

    /**
     * Buffer free function for received bufs.
     */
    GTI_RETURN returnedBufBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf);

	/**
	 * Base class for partial receive based strategy.
	 */
	class CStratPRecv
	{
	protected:
        static uint64_t 		  BUF_LENGTH;
        static uint64_t 		  MAX_NUM_MSGS;
        static const uint64_t 	  myTokenLongMsg;
		static const uint64_t 	  myTokenShutdownSync;
		static const uint64_t 	  myTokenMessage;

	public:
		CStratPRecv ();
		virtual ~CStratPRecv ();
	};

	/*Forward declaration*/
	class CStratPRecvBufInfo;

	/**
	 * Interface for buf infos (see CStratPRecvBufInfo) to notify their
	 * manager that their aren't used by any caller anymore.
	 *
	 * Rational: in the receive calls we do not copy data out of the bufs,
	 *      instead we return the full buffer of the buf (offseted to fit the
	 *      actuall message). We use the buf free function to determine when a
	 *      buf can be reused.
	 */
	class I_BufManager
	{
	public:
	    /**
	     * Must be called by the user of a buf when if is done with it.
	     * @param info for the buf that can now be recycled.
	     */
	    virtual void notifyOfLastUserFinishedEmptyBuf (CStratPRecvBufInfo *info) = 0;
	};

	/**
	 * Base class for receivers of buffers
	 */
	class CStratBufReceiver : public I_BufManager
	{
	protected:
	    /** Reference to masters communication protocol. */
	    I_CommProtocol **myManagedProtocol;

	    std::list<CStratPRecvBufInfo*> myFreeBufs;

	    unsigned int  myTestRequest;    //Request for wildcard receiving
	    CStratPRecvBufInfo* myTestBuf; //Buf for wildcard receiving

	    class TestInfo
	    {
	    public:
	        TestInfo (void);
	        unsigned int request;
	        CStratPRecvBufInfo* buf;
	    };
	    std::vector<TestInfo> myTests;  //State for non-wildcard receiving
	    int myNumNonWcTests; //Number of irecvs started that use a specific channel

	    uint64_t myBufSize;

	    /**
	     * Returns a free buf from myFreeBufs or
	     * creates a new one if necessary.
	     * @return a free buf.
	     */
	    CStratPRecvBufInfo* get_free_buf (void);

	    /**
	     * Handles receival of a long message.
	     * @param in_length the length of the msg (received with the long msg token).
	     * @param channel the channel to use.
	     * @param out_flag flag for testing.
	     * @param out_num_bytes number of bytes received.
	     * @param out_buf buffer with received content.
	     * @param out_free_data associated data for freeing out_buffer.
	     * @param out_buf_free_function function to use for freeing out_buffer.
	     * @see I_CommStrategyDown::test
	     */
	    GTI_RETURN long_msg_from_info (
	            uint64_t in_length,
	            uint64_t channel,
	            int* out_flag,
	            uint64_t* out_num_bytes,
	            void** out_buf,
	            void** out_free_data,
	            GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
	            uint64_t *outChannel
	    );

	public:
	    CStratBufReceiver (I_CommProtocol **managedProtocol, uint64_t bufSize);
	    ~CStratBufReceiver (void);

	    /**
	     * @see I_BufManager::notifyOfLastUserFinishedEmptyBuf
	     */
	    void notifyOfLastUserFinishedEmptyBuf (CStratPRecvBufInfo *info);
	};


	/**
	 * Class with buf information of a received buf
	 * buffer.
	 */
	class CStratPRecvBufInfo
	{
	public:
	    char* buf; /**< Buffer that holds the buf.*/
	    uint64_t channel; /**< Communication channel from which we received this buf.*/
	    I_BufManager *instance; /**< Interface to notify if we happen to be the last external user of an otherwise completed buf.*/

	    CStratPRecvBufInfo(uint64_t buf_size); /**< @param buf_size memory size of the buffer to allocate. */
	    CStratPRecvBufInfo(char* buf); /**< @param buf existing buffer for an buf. */
	    ~CStratPRecvBufInfo(void);
	};

	/**
	 * Class for partial receive strategy that sends out bufs.
	 */
	class CStratPRecvSender : public CStratPRecv
	{
	protected:
	    /** Reference to masters communication protocol. */
	    I_CommProtocol **myManagedProtocol;

	    std::list<CStratIsendRequest> myRequests; /**< List of all open communication requests and their associated buf buffers.*/
	    int myMaxNumReqs; /**< High-water mark for number of concurrent open requests .*/

        /**
         * Sends a long message which exceeds the size of
         * the buf buffers.
         * @see I_CommStrategyUp::send (arguments are similar)
         */
        GTI_RETURN send_long_message (
                uint64_t toPlace,
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
                );

        /**
         * Sends a communication buffer.
         */
        void sendCommBuf (
                void *buf,
                bool sendSynchronizing,
                uint64_t len,
                uint64_t channel,
                void* free_data ,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf));

        /**
         * Completes at least one outstanding send request either in myRequests
         * or a given request.
         * Will only return after that.
         * Default implementation will call a wait_msg, other clases may
         * overwrite this behavior to not only wait for the completion but
         * to also poll for incoming messages.
         */
        virtual void completeOutstandingSendRequest (bool useMyRequests, CStratIsendRequest request);

	public:
        CStratPRecvSender (I_CommProtocol **managedProtocol);

        virtual ~CStratPRecvSender (void);
	};
} //namespace gti

#endif /*CSTRAT_PRECV_H*/
