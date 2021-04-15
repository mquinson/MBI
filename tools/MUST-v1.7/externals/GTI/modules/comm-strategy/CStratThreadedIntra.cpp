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
 * @file CStratThreadedIntra.cpp
 *        @see CStratThreadedIntra.
 *
 * @author Tobias Hilbrich
 * @date 12.04.2012
 */

#include "GtiMacros.h"
#include "GtiEnums.h"

#include "CStratThreadedIntra.h"

#include <assert.h>
#include <stdio.h>
#include <mpi.h>
#include <pnmpimod.h>
#include <stdlib.h>
#include <string.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratThreadedIntra)
mFREE_INSTANCE_FUNCTION(CStratThreadedIntra)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratThreadedIntra)

const uint64_t gti::CStratThreadedIntra::myTokenUpdate = 0xFFFFFFFC;
const uint64_t gti::CStratThreadedIntra::myTokenAcknowledge = 0xFFFFFFFB;

//=============================
// myAggBufFreeFunction
//=============================
GTI_RETURN myAggBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf)
{
    CStratAggregateInfo* aggre_info = (CStratAggregateInfo*)free_data;
    assert (aggre_info->num_in_use); //must be 1 or higher!
    aggre_info->num_in_use--;

    if (aggre_info->num_in_use == 0 && aggre_info->num_msgs_left == 0)
    {
        //aggregate completely processed!
        aggre_info->instance->notifyOfLastUserFinishedEmptyAggregate(aggre_info);
    }

    return GTI_SUCCESS;
}

//=============================
// myLongBufFreeFunction
//=============================
GTI_RETURN myLongBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratThreadedIntra
//=============================
CStratThreadedIntra::CStratThreadedIntra (const char* instanceName)
: ModuleBase<CStratThreadedIntra, CStratIntraQueue> (instanceName),
  CStratThreadedAggregator (&protocol),
  CStratAggregateReceiver (&protocol, BUF_LENGTH),
  myNumMsgsSent (0),
  myNumMsgsReceived (0),
  myReceivedUnexpectedMessages (),
  mySumDiffs (0),
  myNumUpdates (0),
  myChannelState (),
  myCommFinished (false),
  myAggregationAllowed (true)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A single protocol
    assert (subModInstances.size() == 1);

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up internal data

}

//=============================
// ~CStratThreadedIntra
//=============================
CStratThreadedIntra::~CStratThreadedIntra (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;

    //Free internal data

}

//=============================
// shutdown
//=============================
GTI_RETURN CStratThreadedIntra::shutdown (
        GTI_FLUSH_TYPE flush_behavior
)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==Flush
    if (flush_behavior == GTI_FLUSH)
        flush (true);

    // Remove protocol requests, to avoid sync msgs beeing catched by
    // outstanding requests !
    protocol->removeOutstandingRequests ();

    //Shutdown the protocols
    protocol->shutdown();

    return GTI_SUCCESS;
}

//=============================
// communicationFinished
//=============================
GTI_RETURN CStratThreadedIntra::communicationFinished (
        bool *pOutIsFinished
)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //==Bring everything that we didn't send yet on its way
    flush (false);

    //Ids & Channel size
    uint64_t numChannels;
    uint64_t thisPlaceId;
    protocol->getPlaceId(&thisPlaceId);
    protocol->getNumChannels(&numChannels);

    //Data for the messages
    uint64_t *buf = NULL;
    bool areWeFininshed = false;

    if (pOutIsFinished)
        *pOutIsFinished = false;

    //= A) SLAVE
    if (thisPlaceId != 0)
    {
        //== 1) Send an update to the master
        CStratAggregateInfo *info = get_free_aggregate ();
        buf = (uint64_t*) info->buf;
        buf[0] = myTokenUpdate;
        buf[1] = (uint64_t)(myNumMsgsSent-myNumMsgsReceived);
        protocol->ssend(buf, sizeof(uint64_t) * 2, 0);

        //== 2) Receive Acknowledgement
        uint64_t length;
        uint64_t channel = 0;
        bool gotAck = false;
        do
        {
            channel = 0;

            //Do we have an open request? If so we need to use that one
            if (myTestRequest != 0xFFFFFFFF)
            {
                protocol->wait_msg (myTestRequest, &length, &channel);
                myFreeAggregates.push_back(info);
                info = myTestAggregate;
                buf = (uint64_t*) info->buf; //The request used this buffer for receiving
                myTestAggregate = NULL;
                myTestRequest = 0xFFFFFFFF;
            }
            else
            {
                /*
                 * IMPORTANT: using RECV_ANY_CHANNEL may seem counter intuitive,
                 * as we want something from rank 0. BUT, we must allow all our partners
                 * to progress as to also join communicationFinished eventually!
                 */
                protocol->recv(buf, BUF_LENGTH, &length, RECV_ANY_CHANNEL, &channel);
            }

            //Was it possibly a message token instead?
            if (buf[0] == myTokenMessage)
            {
                //Store information about the message in our unexpected messages
                CStratQueueItem toQueue;
                toQueue.buf = buf;
                toQueue.buf_free_function = NULL;
                toQueue.free_data = NULL;
                toQueue.num_bytes = length;
                toQueue.toChannel = channel;

                //Destroy the old aggregate (without killing the buffer we took) and make a new one
                info->buf = NULL;
                delete info;
                info = get_free_aggregate ();
                buf = (uint64_t*) info->buf;

                //Queue it for later retrieval
                myReceivedUnexpectedMessages[channel].push_back(toQueue);
            }
            //Is it a long message?
            else if (buf[0] == myTokenLongMsg)
            {
                //Receive the long message from this channel
                uint64_t longMsgLen = ((uint64_t*)myTestAggregate->buf)[1];
                char* longBuf = new char[longMsgLen];
                uint64_t realLength, realChannel;
                protocol->recv(longBuf, longMsgLen, &realLength, channel, &realChannel);
                assert (realLength == longMsgLen && realChannel == channel);

                CStratQueueItem toQueue;
                toQueue.buf = longBuf;
                toQueue.buf_free_function = NULL;
                toQueue.free_data = (void*)1; //Marks this as a long message
                toQueue.num_bytes = longMsgLen;
                toQueue.toChannel = channel;

                //Queue it for later retrieval
                myReceivedUnexpectedMessages[channel].push_back(toQueue);
            }
            //Is it the hoped for acknowledge?
            else if (buf[0] == myTokenAcknowledge)
            {
                gotAck = true;
                areWeFininshed = (bool)buf[1];
            }
            //Internal error?
            else
            {
                std::cerr << "Internal Error: received an unexpected message! " << __FILE__ << ":" << __LINE__ << std::endl;
                assert (0);
            }
        } while (!gotAck);

        myFreeAggregates.push_back(info);
    }
    //= B) MASTER
    else
    {
        //== 1) Prepare internal data
        mySumDiffs += myNumMsgsSent-myNumMsgsReceived;
        myNumUpdates += 1;

        if (myChannelState.empty())
            myChannelState.resize(numChannels, false);

        myChannelState[0] = true;
        int nextChannel = 1;

        CStratAggregateInfo* getUpdateInfo = get_free_aggregate ();

        //== 2) Wait for an update from everyone
        while (myNumUpdates != numChannels)
        {
            buf = (uint64_t*) getUpdateInfo->buf;

            bool gotUpdate = false;
            uint64_t channel = 0;
            uint64_t length;

            while (myChannelState[nextChannel] == true)
                nextChannel++;

            channel = nextChannel;

            do
            {
                //Do we have an open request? If so we need to use that one
                if (myTestRequest != 0xFFFFFFFF)
                {
                    protocol->wait_msg (myTestRequest, &length, &channel);
                    myFreeAggregates.push_back(getUpdateInfo);
                    getUpdateInfo = myTestAggregate;
                    buf = (uint64_t*) getUpdateInfo->buf;
                    myTestRequest = 0xFFFFFFFF;
                    myTestAggregate = NULL;
                }
                else
                {
                    /*
                     * IMPORTANT: using RECV_ANY_CHANNEL may seem counter intuitive,
                     * as we want something from rank "channel". BUT, we must allow all our partners
                     * to progress as to also join communicationFinished eventually!
                     */
                    protocol->recv(buf, BUF_LENGTH, &length, RECV_ANY_CHANNEL, &channel);
                }

                //Was it possibly a message token instead?
                if (buf[0] == myTokenMessage)
                {
                    //Prepare and receive the message
                    CStratQueueItem toQueue;
                    toQueue.buf = buf;
                    toQueue.buf_free_function = NULL;
                    toQueue.free_data = NULL;
                    toQueue.num_bytes = length;
                    toQueue.toChannel = channel;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);

                    //Destroy the old aggregate (without killing the buffer we took) and make a new one
                    getUpdateInfo->buf = NULL;
                    delete getUpdateInfo;
                    getUpdateInfo = get_free_aggregate ();
                    buf = (uint64_t*) getUpdateInfo->buf;
                }
                //Is it a long message token instead?
                else if (buf[0] == myTokenLongMsg)
                {
                    //Receive the long message from this channel
                    char* longBuf = new char[length];
                    uint64_t realLength, realChannel;
                    protocol->recv(longBuf, length, &realLength, channel, &realChannel);
                    assert (realLength == length && realChannel == channel);

                    CStratQueueItem toQueue;
                    toQueue.buf = longBuf;
                    toQueue.buf_free_function = NULL;
                    toQueue.free_data = (void*)1; //Marks this as a long message
                    toQueue.num_bytes = length;
                    toQueue.toChannel = channel;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);
                }
                //Is it the hoped for update
                else if (buf[0] == myTokenUpdate)
                {
                    gotUpdate = true;
                    mySumDiffs += (long)buf[1];
                    myChannelState[channel] = true;
                    myNumUpdates++;
                }
                //Internal error?
                else
                {
                    std::cerr << "Internal Error: received an unexpected message! " << __FILE__ << ":" << __LINE__ << " (token=" << buf[0] << ")" << std::endl;
                    assert (0);
                }
            } while (!gotUpdate);
        }

        //== 3) Analyse
        buf = (uint64_t*) getUpdateInfo->buf; //recycle the buf we used for getting the update
        buf[0] = myTokenAcknowledge;

        //We are happy if the diffs sum up to 0 (i.e. as many messages sent as received globaly)
        if (mySumDiffs == 0)
        {
            buf[1] = 1;
            areWeFininshed = true;
        }
        else
        {
            buf[1] = 0;
        }

        //IMPORTANT: reset internal data for updating
        mySumDiffs = 0;
        myNumUpdates = 0;
        myChannelState.clear();
        myChannelState.resize(numChannels, false);

        //== 4) Send the Acknowledgments
        for (nextChannel = 1; nextChannel < numChannels; nextChannel++)
            protocol->ssend(buf,sizeof(uint64_t) * 2, nextChannel);

        //Clear the aggregate info we used
        myFreeAggregates.push_back(getUpdateInfo);
    }

    if (areWeFininshed)
    {
        myCommFinished = true;

        if (pOutIsFinished)
            *pOutIsFinished = true;
    }

    return GTI_SUCCESS;
}

//=============================
// getNumPlaces
//=============================
GTI_RETURN CStratThreadedIntra::getNumPlaces (uint64_t *outNumPlaces)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==getNumClients
    if (outNumPlaces)
        protocol->getNumChannels(outNumPlaces);

    return GTI_SUCCESS;
}

//=============================
// getOwnPlaceId
//=============================
GTI_RETURN CStratThreadedIntra::getOwnPlaceId (uint64_t *outId)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==getNumClients
    if (outId)
        protocol->getPlaceId(outId);

    return GTI_SUCCESS;
}

//=============================
// send
//=============================
GTI_RETURN CStratThreadedIntra::send (
        uint64_t toPlace,
        void* buf,
        uint64_t numBytes,
        void* freeData,
        GTI_RETURN (*bufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
)
{
    if (myCommFinished)
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        AddToQueue (buf, numBytes, freeData, bufFreeFunction);
        return GTI_SUCCESS;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==send
    //Is it the first send for this channel?
    if (myCurAggregateBufs.empty())
    {
        uint64_t numChannels;
        protocol->getNumChannels(&numChannels);
        myCurAggregateBufs.resize(numChannels);
        myCommBufs.resize(numChannels);
        myCurrAggregateLens.resize(numChannels);

        for (int i = 0; i < numChannels; i++)
        {
            myCurAggregateBufs[i] = NULL;
            myCommBufs[i] = NULL;
            myCurrAggregateLens[i] = 0;
        }
    }

    //Do we have an aggregate buffer? (swap gives us a buffer)
    if (myCurAggregateBufs[toPlace] == NULL)
        swap (toPlace);

    //Remember the number of messages we have in total
    myNumMsgsSent++;

    //DEBUG
    //std::cout << "SEND: [" << num_bytes << "]" << "id=" << ((uint64_t*) buf)[0] << std::endl;
    //ENDDEBUG

    //Is it a long message ?
    if (numBytes + 3*sizeof (uint64_t) > BUF_LENGTH) //3 -> message token + num records + one record length
    {
        return send_long_message (toPlace, buf, numBytes, freeData, bufFreeFunction);
    }

    //Does it still fit into the aggregate buffer ?
    if (myCurrAggregateLens[toPlace] + numBytes + sizeof(uint64_t) > BUF_LENGTH)
    {
        //Its too long ...
        uint64_t comm_buf_length = myCurrAggregateLens[toPlace];
        /**
         * The swap operation may busy wait until some outstanding send completes.
         * It will receive any incoming message to avoid deadlock.
          */
        swap(toPlace); //finish up outstanding communication of last buffer and swap buffers

        //DEBUG
        //std::cout << "SEND AGGREGATE: [" << comm_buf_length << "]" << std::endl;
        //ENDDEBUG

        sendCommBuf (false, comm_buf_length, toPlace);
    }

    //Add the new message to the aggregate buffer
    uint64_t startIndex = (myCurrAggregateLens[toPlace]/sizeof(uint64_t));
    ((uint64_t*)myCurAggregateBufs[toPlace])[1] = ((uint64_t*)myCurAggregateBufs[toPlace])[1] + 1; //increase count of msgs
    ((uint64_t*)myCurAggregateBufs[toPlace])[startIndex] = numBytes;
    myCurrAggregateLens[toPlace] += sizeof (uint64_t);
    memmove (myCurAggregateBufs[toPlace]+myCurrAggregateLens[toPlace], buf, numBytes);
    myCurrAggregateLens[toPlace] += numBytes;

    //pad to a multiple of "sizeof(uint64_t)"
    if (myCurrAggregateLens[toPlace] % sizeof(uint64_t))
        myCurrAggregateLens[toPlace] += sizeof(uint64_t) - myCurrAggregateLens[toPlace] % sizeof(uint64_t);

    //remove padding if buffer becomes too long due to that
    if (myCurrAggregateLens[toPlace] > BUF_LENGTH)
        myCurrAggregateLens[toPlace] = BUF_LENGTH;

    //free given buffer
    bufFreeFunction (freeData, numBytes, buf);

    //If we aren't allowed to aggregate any more then send it out now
    if (!myAggregationAllowed)
        flush ();

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratThreadedIntra::test (
        int* outFlag,
        uint64_t *outFromPlace,
        uint64_t* outNumBytes,
        void** outBuf,
        void** outFreeData,
        GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
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

    //==test

    //**
    //** (1) we still have an open aggregate with remaining messages
    //**
    if (myOpenAggregate)
    {
        GTI_RETURN ret = msg_from_open_aggregate (outFlag, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);
        myNumMsgsReceived++;
        return ret;
    }

    //**
    //** (1b) Did we have any unexpected messages during communicationFininshed?
    //**
    if (handleUnexpectedMessagesForReceive(outFlag, outFromPlace, outNumBytes, outBuf, outFreeData, outBufFreeFunction))
        return GTI_SUCCESS;

    //**
    //** (2) Do we already have a request for test calls ?
    //**
    if (myTestRequest == 0xFFFFFFFF)
    {
        if (!myTestAggregate)
            myTestAggregate = get_free_aggregate();
        protocol->irecv(myTestAggregate->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
    }

    //**
    //** (3) Test for completion of the request
    //**
    int completed;
    uint64_t numbytes, channel;

    protocol->test_msg(myTestRequest,&completed,&numbytes,&channel);

    //**
    //** (4) Handle new aggregate if the test succeeded
    //**
    if (completed)
    {
        //==If we are the master, this may be an unexpected update
        if (((uint64_t*)myTestAggregate->buf)[0] == myTokenUpdate)
        {
            /*
             * Take care of the update
             * After that we recurse the function to give it a new try.
             */
            handleUnexpectedUpdate((uint64_t*) myTestAggregate->buf, channel);
            myFreeAggregates.push_back (myTestAggregate);
            myTestAggregate = NULL;
            myTestRequest = 0xFFFFFFFF;
            return test (outFlag, outFromPlace, outNumBytes, outBuf,  outFreeData, outBufFreeFunction);
        }

        //==Regular message handling
        assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
        assert (((uint64_t*)myTestAggregate->buf)[0] == myTokenMessage ||
                   ((uint64_t*)myTestAggregate->buf)[0] == myTokenLongMsg    ); //has to be a message

        //Save request as used up
        myTestRequest = 0xFFFFFFFF;

        if (((uint64_t*)myTestAggregate->buf)[0] == myTokenMessage)
        {
            //Received an aggregate
            assert (myOpenAggregate == NULL);
            myOpenAggregate = myTestAggregate;
            myOpenAggregate->current_position = 2*sizeof(uint64_t);
            myOpenAggregate->num_msgs_left = ((uint64_t*)myOpenAggregate->buf)[1];
            myOpenAggregate->num_in_use = 0;
            myOpenAggregate->channel = channel;
            myOpenAggregate->instance = this;
            myTestAggregate = NULL;

            //return a message from the new aggregate
            GTI_RETURN ret = msg_from_open_aggregate (outFlag, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);
            myNumMsgsReceived++;
            return ret;
        }
        else
        {
            //Received info for a long message
            GTI_RETURN ret = long_msg_from_info (
                    ((uint64_t*)myTestAggregate->buf)[1], channel,
                    outFlag, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);

            myFreeAggregates.push_back (myTestAggregate);
            myTestAggregate = NULL;
            myNumMsgsReceived++;
            return ret;
        }
    }
    else
    {
        //No message received
        if (outFlag)
            *outFlag = 0;
    }

    return GTI_SUCCESS;
}

//=============================
// wait
//=============================
GTI_RETURN CStratThreadedIntra::wait (
        uint64_t* outFromPlace,
        uint64_t* outNumBytes,
        void** outBuf,
        void** outFreeData,
        GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
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

    //==wait

    //**
    //** (1) we still have an open aggregate with remaining messages
    //**
    if (myOpenAggregate)
    {
        GTI_RETURN ret = msg_from_open_aggregate (NULL, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);
        myNumMsgsReceived++;
        return ret;
    }

    //**
    //** (1b) Did we have any unexpected messages during communicationFininshed?
    //**
    if (handleUnexpectedMessagesForReceive(NULL, outFromPlace, outNumBytes, outBuf, outFreeData, outBufFreeFunction))
        return GTI_SUCCESS;

    //**
    //** (2) Either wait or use the existing test request if present
    //**
    uint64_t numbytes, channel;
    if (myTestRequest != 0xFFFFFFFF)
    {
        protocol->wait_msg(myTestRequest,&numbytes,&channel);
    }
    else
    {
        if (!myTestAggregate)
            myTestAggregate = get_free_aggregate();
        protocol->recv(myTestAggregate->buf, BUF_LENGTH, &numbytes, RECV_ANY_CHANNEL, &channel);
    }

    //**
    //** (4) Handle new message
    //**
    //==If we are the master, this may be an unexpected update
    if (((uint64_t*)myTestAggregate->buf)[0] == myTokenUpdate)
    {
        /*
         * Take care of the update
         * After that we recurse the function to give it a new try.
         */
        handleUnexpectedUpdate((uint64_t*) myTestAggregate->buf, channel);
        myFreeAggregates.push_back (myTestAggregate);
        myTestAggregate = NULL;
        return wait (outFromPlace, outNumBytes, outBuf,  outFreeData, outBufFreeFunction);
    }

    //==Regular message handling
    assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
    assert (((uint64_t*)myTestAggregate->buf)[0] == myTokenMessage ||
            ((uint64_t*)myTestAggregate->buf)[0] == myTokenLongMsg    ); //has to be a message

    //Save request as used up
    myTestRequest = 0xFFFFFFFF;

    if (((uint64_t*)myTestAggregate->buf)[0] == myTokenMessage)
    {
        //Received an aggregate
        assert (myOpenAggregate == NULL);
        myOpenAggregate = myTestAggregate;
        myOpenAggregate->current_position = 2*sizeof(uint64_t);
        myOpenAggregate->num_msgs_left = ((uint64_t*)myOpenAggregate->buf)[1];
        myOpenAggregate->num_in_use = 0;
        myOpenAggregate->channel = channel;
        myOpenAggregate->instance = this;
        myTestAggregate = NULL;

        //return a message from the new aggregate
        GTI_RETURN ret = msg_from_open_aggregate (NULL, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);
        myNumMsgsReceived++;
        return ret;
    }
    else
    {
        //Received info for a long message
        GTI_RETURN ret = long_msg_from_info (
                ((uint64_t*)myTestAggregate->buf)[1], channel,
                NULL, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);

        myFreeAggregates.push_back (myTestAggregate);
        myTestAggregate = NULL;
        myNumMsgsReceived++;
        return ret;
    }

    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratThreadedIntra::flush (
        void
)
{
    return flush (false);
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CStratThreadedIntra::flushAndSetImmediate (void)
{
    //1) Flush
    flush ();

    //2) Set immediate
    myAggregationAllowed = false;

    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratThreadedIntra::flush (
        bool block
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
    int completed;

    //Test for completion of existing send operations
    for (int i = 0; i < myCurrAggregateLens.size(); i++)
    {
        uint64_t old_len = myCurrAggregateLens[i];
        if (old_len > 2*sizeof(uint64_t))
        {
            swap(i); //finish sending old buffer and swap buffers
            sendCommBuf (false, old_len, i);
        }
    }

    //Do we want to block until all communications are done?
    if (block)
    {
        while (!myRequests.empty())
            completeOutstandingSendRequest(true, 0);
    }//We need to block till all requests are completed

    return GTI_SUCCESS;
}

//=============================
// handleUnexpectedMessagesForReceive
//=============================
bool CStratThreadedIntra::handleUnexpectedMessagesForReceive (
                int* outFlag,
                uint64_t *outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        )
{
    std::map <int, std::list<CStratQueueItem> >::iterator fromIter;

    for (fromIter = myReceivedUnexpectedMessages.begin(); fromIter != myReceivedUnexpectedMessages.end(); fromIter++)
    {
        std::list<CStratQueueItem>::iterator queueIter;

        for (queueIter = fromIter->second.begin(); queueIter != fromIter->second.end(); queueIter++)
        {
            CStratQueueItem item = *queueIter;
            fromIter->second.erase (queueIter);

            //What type of message is it: Aggregate OR long message
            if (item.free_data == NULL)
            {
                //->Aggregate
                assert (myOpenAggregate == NULL); //Otherwise this should not have been called
                myOpenAggregate = new CStratAggregateInfo ((char*)item.buf);
                assert (((uint64_t*)myOpenAggregate->buf)[0] == myTokenMessage);
                myOpenAggregate->current_position = 2*sizeof(uint64_t);
                myOpenAggregate->num_msgs_left = ((uint64_t*)myOpenAggregate->buf)[1];
                myOpenAggregate->num_in_use = 0;
                myOpenAggregate->channel = fromIter->first;
                myOpenAggregate->instance = this;

                GTI_RETURN ret = msg_from_open_aggregate (outFlag, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);
                assert (ret == GTI_SUCCESS);
            }
            else
            {
                //->Long message
                if (outFlag)
                    *outFlag = 1;
                if (outFromPlace)
                    *outFromPlace = fromIter->first;
                if (outNumBytes)
                    *outNumBytes = item.num_bytes;
                if (outBuf)
                    *outBuf = item.buf;
                if (outFreeData)
                    *outFreeData = NULL;
                if (outBufFreeFunction)
                    *outBufFreeFunction = longMsgBufFreeFunction;
            }

            myNumMsgsReceived++;

            return true;
        }
    }

    if (outFlag)
        *outFlag = 0;

    return false;
}

//=============================
// handleUnexpectedUpdate
//=============================
void CStratThreadedIntra::handleUnexpectedUpdate (uint64_t* buf, uint64_t channel)
{
    //Save request as used up
    myTestRequest = 0xFFFFFFFF;

    uint64_t numChannels;
    protocol->getNumChannels(&numChannels);

    myNumUpdates++;
    if (myChannelState.empty())
        myChannelState.resize(numChannels, false);
    myChannelState[channel] = true;

    mySumDiffs += (long) buf[1];
}

//=============================
// completeOutstandingSendRequest
//=============================
void CStratThreadedIntra::completeOutstandingSendRequest (bool useMyRequests, unsigned int request)
{
    std::list<AggRequestInfo>::iterator cur = myRequests.end();
    int completed = false;

    while (!completed)
    {
        uint64_t requestToUse;

        if (useMyRequests)
        {
            if (cur != myRequests.end()) cur++;
            if (cur == myRequests.end())
                cur = myRequests.begin();
            requestToUse = cur->request;
        }
        else
        {
            requestToUse = request;
        }

        protocol->test_msg(requestToUse, &completed, NULL,NULL);

        if (!completed)
        {
            if (myTestRequest == 0xFFFFFFFF)
            {
                if (!myTestAggregate)
                    myTestAggregate = get_free_aggregate();
                protocol->irecv(myTestAggregate->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
            }

            int gotSomething;
            uint64_t numbytes, channel;

            protocol->test_msg(myTestRequest,&gotSomething,&numbytes,&channel);

            if (gotSomething)
            {
                myTestRequest = 0xFFFFFFFF;

                //Was it an update (may happen on master)?
                if (((uint64_t*)myTestAggregate->buf)[0] == myTokenUpdate)
                {
                    handleUnexpectedUpdate((uint64_t*) myTestAggregate->buf, channel);
                    myFreeAggregates.push_back (myTestAggregate);
                    myTestAggregate = NULL;
                }
                else
                //Was it possibly a message token instead?
                if (((uint64_t*)myTestAggregate->buf)[0] == myTokenMessage)
                {
                    //Store information about the message in our unexpected messages
                    CStratQueueItem toQueue;
                    toQueue.buf = myTestAggregate->buf;
                    toQueue.buf_free_function = NULL;
                    toQueue.free_data = NULL;
                    toQueue.num_bytes = numbytes;
                    toQueue.toChannel = channel;

                    //Destroy the old aggregate (without killing the buffer we took) and make a new one
                    myTestAggregate->buf = NULL;
                    delete myTestAggregate;
                    myTestAggregate = NULL;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);
                }
                //Is it a long message?
                else if (((uint64_t*)myTestAggregate->buf)[0] == myTokenLongMsg)
                {
                    //Receive the long message from this channel
                    uint64_t longMsgLen = ((uint64_t*)myTestAggregate->buf)[1];
                    char* longBuf = new char[longMsgLen];
                    uint64_t realLength, realChannel;
                    protocol->recv(longBuf, longMsgLen, &realLength, channel, &realChannel);
                    assert (realLength == longMsgLen && realChannel == channel);

                    CStratQueueItem toQueue;
                    toQueue.buf = longBuf;
                    toQueue.buf_free_function = NULL;
                    toQueue.free_data = (void*)1; //Marks this as a long message
                    toQueue.num_bytes = longMsgLen;
                    toQueue.toChannel = channel;

                    myFreeAggregates.push_back (myTestAggregate);
                    myTestAggregate = NULL;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);
                }
                //Unknown token?
                else
                {
                    std::cerr << "Internal GTI ERROR: check CStratThreadedIntra " << __FILE__ << ":" << __LINE__ << std::endl;
                    assert (0); //Internal error
                }
            }//Got some message?
        }//Completed a send request?
        else if (useMyRequests)//We competed a request
        {
            myFreeBufs.push_back(cur->buf);
            myRequests.erase(cur);
            cur = myRequests.end();
        }
    }//While nothing completed
}

/*EOF*/
