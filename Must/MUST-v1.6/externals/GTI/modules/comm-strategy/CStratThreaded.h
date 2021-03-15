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
 * @file CStratThreaded.h
 *        Application phase aware communication strategy.
 *
 * Provides the interface functionality with an implementation
 * that tries to avoid any comm. during phases in which the
 * application is communicating.
 *
 * @author Tobias Hilbrich
 * @date 25.08.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommProtocol.h"

#include <list>
#include <vector>
#include <map>

#ifndef CSTRAT_THREADED_H
#define CSTRAT_THREADED_H

namespace gti
{
    /**
     * Buffer free function for long messages.
     */
    GTI_RETURN longMsgBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf);

    /**
     * Buffer free function for received aggregates.
     */
    GTI_RETURN returnedAggregateBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf);

	/**
	 * Base class for the non-blocking send based communication
	 * strategy.
	 */
	class CStratThreaded
	{
	protected:
        static uint64_t 		  BUF_LENGTH;
        static uint64_t 		  MAX_NUM_MSGS;
        static const uint64_t 	  myTokenLongMsg;
		static const uint64_t 	  myTokenShutdownSync;
		static const uint64_t 	  myTokenMessage;

	public:
		CStratThreaded ();
		virtual ~CStratThreaded ();
	};

	/*Forward declaration*/
	class CStratAggregateInfo;

	/**
	 * Interface for aggrgate infos (see CStratAggregateInfo) to notify their
	 * manager that their aren't used by any caller anymore.
	 *
	 * Rational: in the receive calls we do not copy data out of the aggregates,
	 *      instead we return the full buffer of the aggregate (offseted to fit the
	 *      actuall message). We use the buf free function to determine when an
	 *      aggregate can be reused. If an aggregate has no remaining messages
	 *      and its last user called the buf_free_function, we invoke the single
	 *      function of this interface.
	 */
	class I_AggregateManager
	{
	public:
	    /**
	     * Must be called by the user of aggregates that are returned in a receive
	     * function of the strategy. Must be called if the aggregate was completed
	     * -- num_msgs_left == 0 -- and if this was the last user of the aggregate
	     * -- num_in_use == 0 (after accounting for the call to buf_free_function).
	     * @param info for the aggregate that can now be recycled.
	     */
	    virtual void notifyOfLastUserFinishedEmptyAggregate (CStratAggregateInfo *info) = 0;
	};

	/**
	 * Base class for receivers of aggregates
	 */
	class CStratAggregateReceiver : public I_AggregateManager
	{
	protected:
	    /** Reference to masters communication protocol. */
	    I_CommProtocol **myManagedProtocol;

// 	    std::map<CStratAggregateInfo*, int> myAggregatesInUse;
	    std::list<CStratAggregateInfo*> myFreeAggregates;

	    unsigned int  myTestRequest;    //Request for wildcard receiving
	    CStratAggregateInfo* myTestAggregate; //Aggregate for wildcard receiving

	    class TestInfo
	    {
	    public:
	        TestInfo (void);
	        unsigned int request;
	        CStratAggregateInfo* aggregate;
	    };
	    std::vector<TestInfo> myTests;  //State for non-wildcard receiving
	    int myNumNonWcTests; //Number of irecvs started that use a specific channel

	    CStratAggregateInfo* myOpenAggregate;


	    uint64_t myAggregateSize;

	    /**
	     * Returns a free aggregate from myFreeAggregates or
	     * creates a new one if necessary.
	     * @return a free aggregate.
	     */
	    CStratAggregateInfo* get_free_aggregate (void);

	    /**
	     * Handles receival of a message from the currently open
	     * aggregate (open aggregate required).
	     * @see I_CommStrategyDown::test
	     */
	    GTI_RETURN msg_from_open_aggregate (
	            int* out_flag,
	            uint64_t* out_num_bytes,
	            void** out_buf,
	            void** out_free_data,
	            GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
	            uint64_t *outChannel
	    );

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
	    CStratAggregateReceiver (I_CommProtocol **managedProtocol, uint64_t aggregateSize);
	    ~CStratAggregateReceiver (void);

	    /**
	     * @see I_AggregateManager::notifyOfLastUserFinishedEmptyAggregate
	     */
	    void notifyOfLastUserFinishedEmptyAggregate (CStratAggregateInfo *info);
	};


	/**
	 * Class with aggregate information of a received aggregate
	 * buffer.
	 */
	class CStratAggregateInfo
	{
	public:
	    char* buf; /**< Buffer that holds the aggregate.*/
	    uint64_t current_position; /**< Byte index into current position of buf.*/
	    uint64_t num_msgs_left; /**< Number of unprocessed messages in this aggregate.*/
	    uint64_t num_in_use; /**< Number of external users that may still read from this aggregate.*/
	    uint64_t channel; /**< Communication channel from which we received this aggregate.*/
	    I_AggregateManager *instance; /**< Interface to notify if we happen to be the last external user of an otherwise completed aggregate.*/

	    CStratAggregateInfo(uint64_t buf_size); /**< @param buf_size memory size of the buffer to allocate. */
	    CStratAggregateInfo(char* buf); /**< @param buf existing buffer for an aggregate. */
	    ~CStratAggregateInfo(void);
	};

	/**
	 * Class for aggregation strategy that sends out aggregates.
	 */
	class CStratThreadedAggregator : public CStratThreaded
	{
	protected:

	    struct AggRequestInfo
	    {
	        char* buf;
	        unsigned int request;
	        AggRequestInfo (char* buf, unsigned int request)
	            : buf (buf),
	              request (request) {}
	    };

	    /** Reference to masters communication protocol. */
	    I_CommProtocol **myManagedProtocol;

	    std::list<AggRequestInfo> myRequests; /**< List of all open communication requests and their associated aggregate buffers.*/
	    int myMaxNumReqs; /**< High-water mark for number of concurrent open requests .*/
	    std::list<char*> myFreeBufs; /**< List of available aggregate buffers (pool for recycling).*/
	    std::vector<char*>  myCommBufs, /**< Buffer for current communication.*/
	               myCurAggregateBufs; /**< Buffer of current aggregation.*/
	    std::vector<uint64_t>  myCurrAggregateLens; /**< Length of current aggregation.*/

        /**
         * Checks whether any ongoing communication can be completed,
         * puts the current aggregate buffer into the myCommBuf for transfer
         * (but does not starts the transfer), and seeds myCurAggregateBuf
         * with a new (or pooled buffer).
         */
        void swap (uint64_t channel);

        /**
         * Sends a long message which exceeds the size of
         * the aggregate buffers.
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
         * Sets up the aggregate buffer (myCurrAggregateLen) with its basic data.
         */
        void prepareAggregateBuffer (uint64_t channel);

        /**
         * Sends the current communication buffer and sets it to null.
         */
        void sendCommBuf (bool sendSynchronizing, uint64_t len, uint64_t channel);

        /**
         * Completes at least one outstanding send request either in myRequests
         * or a given request.
         * Will only return after that.
         * Default implementation will call a wait_msg, other clases may
         * overwrite this behavior to not only wait for the completion but
         * to also poll for incoming messages.
         */
        virtual void completeOutstandingSendRequest (bool useMyRequests, unsigned int request);

	public:
        CStratThreadedAggregator (I_CommProtocol **managedProtocol);

        virtual ~CStratThreadedAggregator (void);
	};
} //namespace gti

#endif /*CSTRAT_THREADED_H*/
