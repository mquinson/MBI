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
 * @file CStratThreadedDown.cpp
 *        Application phase aware communication strategy.
 *
 * Provides the interface functionality with an implementation
 * that tries to avoid any comm. during phases in which the
 * application is communicating.
 *
 * @author Tobias Hilbrich
 * @date 24.08.2009
 */

#include <assert.h>
#include <stdio.h>
#include <mpi.h>
#include <pnmpimod.h>
#include "CStratThreadedDown.h"
#include "GtiMacros.h"
#include <stdlib.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratThreadedDown)
mFREE_INSTANCE_FUNCTION(CStratThreadedDown)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratThreadedDown)

//=============================
// CStratThreadedDown
//=============================
CStratThreadedDown::CStratThreadedDown (const char* instanceName)
: ModuleBase<CStratThreadedDown, CStratDownQueue> (instanceName),
  CStratAggregateReceiver (&protocol, BUF_LENGTH)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Either uses a regular comm protocol or a regular and a
    //fall-back one.
    assert ((subModInstances.size() == 1) || (subModInstances.size() == 2));

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up internal data

}

//=============================
// ~CStratThreadedDown
//=============================
CStratThreadedDown::~CStratThreadedDown (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);

    //Free internal data
}


//=============================
// shutdown
//=============================
GTI_RETURN CStratThreadedDown::shutdown (
        GTI_FLUSH_TYPE flush_behavior,
        GTI_SYNC_TYPE sync_behavior
        )
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
    	return GTI_ERROR_NOT_INITIALIZED;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
    	ProcessQueue ();

    //==shutdown
    if (flush_behavior == GTI_FLUSH)
        flush ();

    if (sync_behavior == GTI_SYNC)
    {
        uint64_t num,i;
        protocol->getNumClients(&num);

        uint64_t buf[2] = {myTokenShutdownSync,0};
        int completed = 0;
        uint64_t numbytes, channel;

        for (i = 0; i < num; i++)
        {
            //Send the ping
            protocol->ssend(buf,sizeof(uint64_t)*2,i);
        }

        //receive the pongs
        int pongCount = 0;
        do
        {
            CStratAggregateInfo* successfulAggregate = NULL;
            completed = 0;

            //If no wildcard request exists, create one!
            if (myTestRequest == 0xFFFFFFFF)
            {
                myTestAggregate = get_free_aggregate();
                protocol->irecv(myTestAggregate->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
            }

            //Loop over all existing requests that use a specified channel
            for (int i = 0; i < myTests.size(); i++)
            {
                if (myTests[i].aggregate == NULL) continue;

                protocol->test_msg(myTests[i].request,&completed,&numbytes,&channel);
                if (completed)
                {
                    successfulAggregate = myTests[i].aggregate;
                    myTests[i].aggregate = NULL;
                    myNumNonWcTests--;
                    break;
                }
            }

            //If no specific channel yielded anything, also try the wildcard one
            if (!completed)
            {
                protocol->test_msg(myTestRequest,&completed,&numbytes,&channel);

                if (completed)
                {
                    successfulAggregate = myTestAggregate;
                    myTestAggregate = NULL;
                    myTestRequest = 0xFFFFFFFF;
                }
            }

            if (completed)
            {
                assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries

                assert (((uint64_t*)successfulAggregate->buf)[0] == myTokenMessage ||
                        ((uint64_t*)successfulAggregate->buf)[0] == myTokenLongMsg    ||
                        ((uint64_t*)successfulAggregate->buf)[0] == myTokenShutdownSync);

                if (((uint64_t*)successfulAggregate->buf)[0] == myTokenShutdownSync)
                {
                    pongCount++;
                }
                else
                if (((uint64_t*)successfulAggregate->buf)[0] == myTokenMessage)
                {
                    //Received an aggregate
                    //@note we can give a warning here: received some other outstanding
                    //     message even though this strategy is being closed
                    //     Current decision: only give the warning in Debug mode
                    //Receive the message, and discard it
#ifdef GTI_DEBUG
                    std::cerr << "WARNING: In shutdown sync (StrategyDown) "
                            << "received an outstanding message!"
                            << std::endl;
#endif
                }
                else
                {
                    //Received info for a long message
                    uint64_t buf_size = ((numbytes+sizeof(uint64_t)-1)/sizeof(uint64_t));
                    uint64_t* tempbuf = new uint64_t[buf_size]();
                    protocol->recv(tempbuf, numbytes ,&numbytes, channel, NULL);
                    delete [] tempbuf;

                    //@note we can give a warning here: received some other outstanding
                    //     message even though this strategy is being closed
                    //     Current decision: only give the warning in Debug mode
                    //Receive the message, and discard it
#ifdef GTI_DEBUG
                    std::cerr << "WARNING: In shutdown sync (StrategDown) "
                            << "received an outstanding (long) message!"
                            << std::endl;
#endif
                }

                myFreeAggregates.push_back(successfulAggregate);
            }
        } while (pongCount != num);
    }

    // Remove any remaining protocol requests
    protocol->removeOutstandingRequests ();

    //Shutdown the protocols
    protocol->shutdown();
    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CStratThreadedDown::getPlaceId (uint64_t* outPlaceId)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==getPlaceId   
    if (outPlaceId)   
        protocol->getPlaceId(outPlaceId);

    return GTI_SUCCESS;
}

//=============================
// broadcast
//=============================
GTI_RETURN CStratThreadedDown::broadcast (
        void* buf,
        uint64_t num_bytes,
        void* free_data,
        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        AddToQueue (buf, num_bytes, free_data, buf_free_function);
        return GTI_SUCCESS;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==broadcast
    uint64_t num,i;
    protocol->getNumChannels(&num);
    /**
     * @todo ssend is expensive, even if we stick with syncronizing we might
     *            want to at least start all sends and then wait for them as to add more
     *            parallelism!
     */
    for (i = 0; i < num; i++)
    {
        uint64_t temp_buf[2] = {myTokenMessage,
                num_bytes};
        protocol->ssend(temp_buf,sizeof(uint64_t)*2,i);
        protocol->ssend(buf,num_bytes,i);
    }
    if (buf_free_function)
        (*buf_free_function) (free_data, num_bytes, buf);
    return GTI_SUCCESS;
}

//=============================
// getNumClients
//=============================
GTI_RETURN CStratThreadedDown::getNumClients (uint64_t *outNumClients)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==getNumClients
    if (outNumClients)
        protocol->getNumClients(outNumClients);

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratThreadedDown::test (
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
        uint64_t *outChannel,
        uint64_t preferChannel
        )
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
    	return GTI_ERROR_NOT_INITIALIZED;
    }

    //==Initialize myTests if necessary
    if (myTests.size() == 0)
    {
        uint64_t numChannels;
        protocol->getNumChannels(&numChannels);
        myTests.resize(numChannels);
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==test

    //**
    //** (1) we still have an open aggregate with remaining messages
    //** -> we simply ignore "preferChannel" in that case
    //**
    if (myOpenAggregate)
    {
        GTI_RETURN ret = msg_from_open_aggregate (out_flag, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);
        return ret;
    }

    //**
    //** (2) Do we already have a request for wildcard test calls ?
    //**
    if (myTestRequest == 0xFFFFFFFF)
    {
        if (preferChannel == 0xFFFFFFFF)
        {
            myTestAggregate = get_free_aggregate();
            protocol->irecv(myTestAggregate->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
        }
        else
        {
            if (!myTests[preferChannel].aggregate)
            {
                myTests[preferChannel].aggregate = get_free_aggregate();
                protocol->irecv(myTests[preferChannel].aggregate->buf, BUF_LENGTH, &(myTests[preferChannel].request), preferChannel);
                myNumNonWcTests++;
            }
        }
    }

    //**
    //** (3) Test for completion of the request
    //**
    int completed = false;
    uint64_t numbytes, channel;
    CStratAggregateInfo *successfulAggregate = NULL;

    if (myTestRequest == 0xFFFFFFFF)
    {
        protocol->test_msg(myTests[preferChannel].request,&completed,&numbytes,&channel);
        if (completed)
        {
            successfulAggregate = myTests[preferChannel].aggregate;
            myTests[preferChannel].aggregate = NULL;
            myNumNonWcTests--;
        }
    }
    else
    {
        //Loop over all existing requests that use a specified channel
        if (myNumNonWcTests)
        {
            for (int i = 0; i < myTests.size(); i++)
            {
                if (myTests[i].aggregate == NULL) continue;

                protocol->test_msg(myTests[i].request,&completed,&numbytes,&channel);
                if (completed)
                {
                    successfulAggregate = myTests[i].aggregate;
                    myTests[i].aggregate = NULL;
                    myNumNonWcTests--;
                    break;
                }
            }
        }

        //If no existing specific channel irecv yielded a message, also test the wildcard one
        if (!completed)
        {
            protocol->test_msg(myTestRequest,&completed,&numbytes,&channel);

            if (completed)
            {
                successfulAggregate = myTestAggregate;
                myTestAggregate = NULL;
                myTestRequest = 0xFFFFFFFF;
            }
        }
    }

    //**
    //** (4) Handle new aggregate if the test succeeded
    //**
    if (completed)
    {
        assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
        assert (((uint64_t*)successfulAggregate->buf)[0] == myTokenMessage ||
                ((uint64_t*)successfulAggregate->buf)[0] == myTokenLongMsg    ); //has to be a message

        if (((uint64_t*)successfulAggregate->buf)[0] == myTokenMessage)
        {
				//Received an aggregate
				myOpenAggregate = successfulAggregate;
				myOpenAggregate->current_position = 2*sizeof(uint64_t);
				myOpenAggregate->num_msgs_left = ((uint64_t*)myOpenAggregate->buf)[1];
				myOpenAggregate->num_in_use = 0;
				myOpenAggregate->channel = channel;
				myOpenAggregate->instance = this;

				//return a message from the new aggregate
				GTI_RETURN ret = msg_from_open_aggregate (out_flag, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);
				return ret;
        }
        else
        {
				//Received info for a long message
				GTI_RETURN ret = long_msg_from_info (
						((uint64_t*)successfulAggregate->buf)[1], channel,
						out_flag, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);

				myFreeAggregates.push_back (successfulAggregate);
				return ret;
        }
    }
    else
    {
        //No message received
        if (out_flag)
        	*out_flag = 0;
    }

    return GTI_SUCCESS;
}

//=============================
// wait
//=============================
GTI_RETURN CStratThreadedDown::wait (
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
        uint64_t *outChannel
        )
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==Initialize myTests if necessary
    if (myTests.size() == 0)
    {
        uint64_t numChannels;
        protocol->getNumChannels(&numChannels);
        myTests.resize(numChannels);
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==wait

    //**
    //** (1) we still have an open aggregate with remaining messages ?
    //**
    if (myOpenAggregate)
    {
        GTI_RETURN ret = msg_from_open_aggregate (NULL, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);
        return ret;
    }

    //**
    //** (2) Wait for a message
    //**
    uint64_t numbytes, channel;
    CStratAggregateInfo *successfulAggregate;

    /*CASE 1: wc request exists and no specific channel requests exist*/
    if (myTestRequest != 0xFFFFFFFF && myNumNonWcTests == 0)
    {
        protocol->wait_msg(myTestRequest,&numbytes,&channel);
    }
    /*CASE 2: we have a to test for multiple requests in a loop */
    else
    {
        //Do we need do create a wc request?
        if (myTestRequest == 0xFFFFFFFF && myNumNonWcTests != myTests.size())
        {
            myTestAggregate = get_free_aggregate();
            protocol->irecv(myTestAggregate->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
        }

        //Loop till we got a message
        int completed = 0;
        while (!completed)
        {
            //Test for specific channel requests
            for (int i = 0; i < myTests.size(); i++)
            {
                if (myTests[i].aggregate == NULL) continue;

                protocol->test_msg(myTests[i].request,&completed,&numbytes,&channel);
                if (completed)
                {
                    successfulAggregate = myTests[i].aggregate;
                    myTests[i].aggregate = NULL;
                    myNumNonWcTests--;
                    break;
                }
            }

            //If no existing specific channel irecv yielded a message, also test the wildcard one
            if (!completed && myTestRequest != 0xFFFFFFFF)
            {
                protocol->test_msg(myTestRequest,&completed,&numbytes,&channel);

                if (completed)
                {
                    successfulAggregate = myTestAggregate;
                    myTestAggregate = NULL;
                    myTestRequest = 0xFFFFFFFF;
                }
            }
        }//while not completed
    }

    //**
    //** (3) Handle new aggregate
    //**
    assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
    assert (((uint64_t*)successfulAggregate->buf)[0] == myTokenMessage ||
    		((uint64_t*)successfulAggregate->buf)[0] == myTokenLongMsg    ); //has to be a message

    GTI_RETURN ret;
    if (((uint64_t*)successfulAggregate->buf)[0] == myTokenMessage)
    {
        //Received an aggregate
        myOpenAggregate = successfulAggregate;
        myOpenAggregate->current_position = 2*sizeof(uint64_t);
        myOpenAggregate->num_msgs_left = ((uint64_t*)myOpenAggregate->buf)[1];
        myOpenAggregate->num_in_use = 0;
        myOpenAggregate->channel = channel;
        myOpenAggregate->instance = this;
        successfulAggregate = NULL;

        //return a message from the new aggregate
        ret = msg_from_open_aggregate (NULL, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);
    }
    else
    {
        //Received info for a long message
        ret = long_msg_from_info (
                ((uint64_t*)successfulAggregate->buf)[1], channel,
                NULL, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);

        myFreeAggregates.push_back (successfulAggregate);
        successfulAggregate = NULL;
    }

    return ret;
}

//=============================
// acknowledge
//=============================
GTI_RETURN CStratThreadedDown::acknowledge (
		uint64_t channel
        )
{
	/*Nothing to do*/
	return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratThreadedDown::flush (
        void
        )
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
      	ProcessQueue ();

    //==flush
    //No action needed
    //->broadcast uses ssend
    return GTI_SUCCESS;
}

/*EOF*/
