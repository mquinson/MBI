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
 * @file CStratPRecvDown.cpp
 *        @see gti::CStratPRecvDown
 *
 * Based on CStratThreaded, this is a modified copy!
 *
 * @author Tobias Hilbrich
 * @date 13.03.2013
 */

#include <assert.h>
#include <stdio.h>
#include <mpi.h>
#include <pnmpimod.h>
#include "CStratPRecvDown.h"
#include "GtiMacros.h"
#include <stdlib.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratPRecvDown)
mFREE_INSTANCE_FUNCTION(CStratPRecvDown)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratPRecvDown)

//=============================
// CStratPRecvDown
//=============================
CStratPRecvDown::CStratPRecvDown (const char* instanceName)
: ModuleBase<CStratPRecvDown, CStratDownQueue> (instanceName),
  CStratBufReceiver (&protocol, BUF_LENGTH),
  myOutstandingBroadcasts ()
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
// ~CStratPRecvDown
//=============================
CStratPRecvDown::~CStratPRecvDown (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);

    //Free internal data
    if (!myOutstandingBroadcasts.empty())
    {
        std::cerr << "CStratPRecvDown::~CStratPRecvDown warning outstanding broadcast requests exist." << std::endl;
    }
    myOutstandingBroadcasts.clear();
}


//=============================
// shutdown
//=============================
GTI_RETURN CStratPRecvDown::shutdown (
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
            /*
             * We do not need to receive messages here, since UP side will
             * receive any outstanding messages we may still have towards
             * the other side, so we will eventually deliver our message.
             */
            //Send the ping
            protocol->ssend(buf,sizeof(uint64_t)*2,i);
        }

        //receive the pongs
        int pongCount = 0;
        do
        {
            CStratPRecvBufInfo* successfulBuf = NULL;
            completed = 0;

            //If no wildcard request exists, create one!
            if (myTestRequest == 0xFFFFFFFF)
            {
                myTestBuf = get_free_buf();
                protocol->irecv(myTestBuf->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
            }

            //Loop over all existing requests that use a specified channel
            for (int i = 0; i < myTests.size(); i++)
            {
                if (myTests[i].buf == NULL) continue;

                protocol->test_msg(myTests[i].request,&completed,&numbytes,&channel);
                if (completed)
                {
                    successfulBuf = myTests[i].buf;
                    myTests[i].buf = NULL;
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
                    successfulBuf = myTestBuf;
                    myTestBuf = NULL;
                    myTestRequest = 0xFFFFFFFF;
                }
            }

            if (completed)
            {
                assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries

                assert (((uint64_t*)successfulBuf->buf)[0] == myTokenMessage ||
                        ((uint64_t*)successfulBuf->buf)[0] == myTokenLongMsg    ||
                        ((uint64_t*)successfulBuf->buf)[0] == myTokenShutdownSync);

                if (((uint64_t*)successfulBuf->buf)[0] == myTokenShutdownSync)
                {
                    pongCount++;
                }
                else
                    if (((uint64_t*)successfulBuf->buf)[0] == myTokenMessage)
                    {
                        //Received an buf
                        //TODO give a warning here, received some other outstanding
                        //     message even though this strategy is being closed
                        //     Current decision: only give the warning in Debug	mode
                        //Receive the message, and discard it
#ifdef GTI_DEBUG
                        std::cerr << "WARNING: In shutdown sync (StrategyDown) "
                                << "received an outstanding message!"
                                << std::endl;
#endif
                    }
                    else
                    {
                        /**
                         * @todo if we have specific channel tests + a wc irecv,
                         * could the irecv consume the actual message instead?
                         */
                        //Received info for a long message
                        uint64_t buf_size = ((numbytes+sizeof(uint64_t)-1)/sizeof(uint64_t));
                        uint64_t* tempbuf = new uint64_t[buf_size]();
                        protocol->recv(tempbuf, numbytes ,&numbytes, channel, NULL);
                        delete [] tempbuf;

                        //TODO give a warning here, received some other outstanding
                        //     message even though this strategy is being closed
                        //Receive the message, and discard it
                        std::cerr << "WARNING: In shutdown sync (StrategDown) "
                                << "received an outstanding (long) message!"
                                << std::endl;
                    }

                myFreeBufs.push_back(successfulBuf);
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
// broadcast
//=============================
GTI_RETURN CStratPRecvDown::broadcast (
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

    //==Check whether we can complete a broadcast
    /**
     * @todo This is potentially dangerous business since we enforce no upper limit on the number
     *            of outstanding broadcasts.
     *            My guess (Tobias) here is that the number should in any common case be fairly low,
     *            but if someone does heavyweight broadcasting this is an issue indeed.
     */
    tryToCompleteBroadcasts (false);

    //==broadcast
    uint64_t num,i;
    protocol->getNumChannels(&num);

	uint64_t *temp_buf = new uint64_t[2];
    temp_buf[0] = myTokenMessage;
    temp_buf[1] = num_bytes;

    myOutstandingBroadcasts.push_back(OutstandingBroadcastInfo ());
    OutstandingBroadcastInfo &info = myOutstandingBroadcasts.back();
    info.buf = buf;
    info.buf_free_function = buf_free_function;
    info.num_bytes = num_bytes;
    info.free_data = free_data;
    info.bufLengthMessage = temp_buf;

    for (i = 0; i < num; i++)
    {
        unsigned int requestA, requestB;

        protocol->isend(temp_buf, sizeof(uint64_t)*2, &requestA, i);
        protocol->isend(buf, num_bytes, &requestB, i);

        info.requests.push_back(requestA);
        info.requests.push_back(requestB);
    }

    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CStratPRecvDown::getPlaceId (uint64_t* outPlaceId)
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
// getNumClients
//=============================
GTI_RETURN CStratPRecvDown::getNumClients (uint64_t *outNumClients)
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
GTI_RETURN CStratPRecvDown::test (
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
    //** (2) Do we already have a request for wildcard test calls ?
    //**
    if (myTestRequest == 0xFFFFFFFF)
    {
        if (preferChannel == 0xFFFFFFFF)
        {
            myTestBuf = get_free_buf();
            protocol->irecv(myTestBuf->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
        }
        else
        {
            if (!myTests[preferChannel].buf)
            {
                myTests[preferChannel].buf = get_free_buf();
                protocol->irecv(myTests[preferChannel].buf->buf, BUF_LENGTH, &(myTests[preferChannel].request), preferChannel);
                myNumNonWcTests++;
            }
        }
    }

    //**
    //** (3) Test for completion of the request
    //**
    int completed = false;
    uint64_t numbytes, channel;
    CStratPRecvBufInfo *successfulBuf = NULL;

    if (myTestRequest == 0xFFFFFFFF)
    {
        protocol->test_msg(myTests[preferChannel].request,&completed,&numbytes,&channel);
        if (completed)
        {
            successfulBuf = myTests[preferChannel].buf;
            myTests[preferChannel].buf = NULL;
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
                if (myTests[i].buf == NULL) continue;

                protocol->test_msg(myTests[i].request,&completed,&numbytes,&channel);
                if (completed)
                {
                    successfulBuf = myTests[i].buf;
                    myTests[i].buf = NULL;
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
                successfulBuf = myTestBuf;
                myTestBuf = NULL;
                myTestRequest = 0xFFFFFFFF;
            }
        }
    }

    //**
    //** (4) Handle new buf if the test succeeded
    //**
    if (completed)
    {
        assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
        assert (((uint64_t*)successfulBuf->buf)[0] == myTokenMessage ||
                ((uint64_t*)successfulBuf->buf)[0] == myTokenLongMsg    ); //has to be a message

        if (((uint64_t*)successfulBuf->buf)[0] == myTokenMessage)
        {
            if (out_flag) *out_flag = 1;
            if (out_num_bytes) *out_num_bytes =  ((uint64_t*)successfulBuf->buf)[1];
            if (out_buf) *out_buf = successfulBuf->buf+ 2*sizeof(uint64_t);
            if (out_free_data) *out_free_data = successfulBuf;
            if (out_buf_free_function) *out_buf_free_function = returnedBufBufFreeFunction;
            if (outChannel) *outChannel = channel;

            return GTI_SUCCESS;
        }
        else
        {
            //Received info for a long message
            GTI_RETURN ret = long_msg_from_info (
                    ((uint64_t*)successfulBuf->buf)[1], channel,
                    out_flag, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);

            myFreeBufs.push_back (successfulBuf);
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
GTI_RETURN CStratPRecvDown::wait (
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
    //** (2) Wait for a message
    //**
    uint64_t numbytes, channel;
    CStratPRecvBufInfo *successfulBuf;

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
            myTestBuf = get_free_buf();
            protocol->irecv(myTestBuf->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
        }

        //Loop till we got a message
        int completed = 0;
        while (!completed)
        {
            //Test for specific channel requests
            for (int i = 0; i < myTests.size(); i++)
            {
                if (myTests[i].buf == NULL) continue;

                protocol->test_msg(myTests[i].request,&completed,&numbytes,&channel);
                if (completed)
                {
                    successfulBuf = myTests[i].buf;
                    myTests[i].buf = NULL;
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
                    successfulBuf = myTestBuf;
                    myTestBuf = NULL;
                    myTestRequest = 0xFFFFFFFF;
                }
            }
        }//while not completed
    }

    //**
    //** (3) Handle new buf
    //**
    assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
    assert (((uint64_t*)successfulBuf->buf)[0] == myTokenMessage ||
            ((uint64_t*)successfulBuf->buf)[0] == myTokenLongMsg    ); //has to be a message

    GTI_RETURN ret;
    if (((uint64_t*)successfulBuf->buf)[0] == myTokenMessage)
    {
        if (out_num_bytes) *out_num_bytes =  ((uint64_t*)successfulBuf->buf)[1];
        if (out_buf) *out_buf = successfulBuf + 2*sizeof(uint64_t);
        if (out_free_data) *out_free_data = successfulBuf;
        if (out_buf_free_function) *out_buf_free_function = returnedBufBufFreeFunction;
        if (outChannel) *outChannel = channel;
        ret = GTI_SUCCESS;
    }
    else
    {
        //Received info for a long message
        ret = long_msg_from_info (
                ((uint64_t*)successfulBuf->buf)[1], channel,
                NULL, out_num_bytes, out_buf, out_free_data, out_buf_free_function, outChannel);

        myFreeBufs.push_back (successfulBuf);
        successfulBuf = NULL;
    }

    return ret;
}

//=============================
// acknowledge
//=============================
GTI_RETURN CStratPRecvDown::acknowledge (
        uint64_t channel
)
{
    /*Nothing to do*/
    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratPRecvDown::flush (
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
    tryToCompleteBroadcasts (true);

    return GTI_SUCCESS;
}

//=============================
// tryToCompleteBroadcasts
//=============================
void CStratPRecvDown::tryToCompleteBroadcasts (bool block)
{
    std::list<OutstandingBroadcastInfo>::iterator iter;
    for (iter = myOutstandingBroadcasts.begin(); iter != myOutstandingBroadcasts.end(); )
    {
        OutstandingBroadcastInfo &info = *iter;

        //Finish as many requests as possible
        std::list<unsigned int>::iterator rIter = info.requests.begin();
        int numOutstanding = info.requests.size();
        int numSuccesses = 0;

        while (numSuccesses < numOutstanding)
        {
            int completed = 1;
            uint64_t ignoreLength, ignoreChannel;
            protocol->test_msg(*rIter, &completed, &ignoreLength, &ignoreChannel);

            if (completed)
            {
                numSuccesses++;
                info.requests.erase(rIter);
                rIter = info.requests.begin();
            }
            else
            {
                if (block)
                {
                    rIter++;
                    if (rIter == info.requests.end())
                        rIter = info.requests.begin();
                }
                else
                {
                    break;
                }
            }
        }

        if (numSuccesses == numOutstanding)
        {
            //Free buffer
            if (info.buf_free_function)
                (*info.buf_free_function) (info.free_data, info.num_bytes, info.buf);

            if (info.bufLengthMessage)
                delete [] ((uint64_t*) info.bufLengthMessage);

            myOutstandingBroadcasts.erase (iter);
            iter = myOutstandingBroadcasts.begin();
        }
        else
        {
            //If block = true then we will not end here
            break;
        }
    }
}

/*EOF*/
