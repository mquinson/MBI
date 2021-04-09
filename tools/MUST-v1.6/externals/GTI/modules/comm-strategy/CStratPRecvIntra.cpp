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
 * @file CStratPRecvIntra.cpp
 *        @see gti::CStratPRecvIntra
 *
 * Based on CStratThreaded, this is a modified copy!
 *
 * @author Tobias Hilbrich
 * @date 13.03.2013
 */

#include "GtiMacros.h"
#include "GtiEnums.h"

#include "CStratPRecvIntra.h"

#include <assert.h>
#include <stdio.h>
#include <mpi.h>
#include <pnmpimod.h>
#include <stdlib.h>
#include <string.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratPRecvIntra)
mFREE_INSTANCE_FUNCTION(CStratPRecvIntra)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratPRecvIntra)

const uint64_t gti::CStratPRecvIntra::myTokenUpdate = 0xFFFFFFFC;
const uint64_t gti::CStratPRecvIntra::myTokenAcknowledge = 0xFFFFFFFB;

//=============================
// CStratPRecvIntra
//=============================
CStratPRecvIntra::CStratPRecvIntra (const char* instanceName)
: ModuleBase<CStratPRecvIntra, CStratIntraQueue> (instanceName),
  CStratPRecvSender (&protocol),
  CStratBufReceiver (&protocol, BUF_LENGTH),
  myNumMsgsSent (0),
  myNumMsgsReceived (0),
  myReceivedUnexpectedMessages (),
  mySumDiffs (0),
  myNumUpdates (0),
  myChannelState (),
  myCommFinished (false)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A single protocol
    assert (subModInstances.size() == 1);

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up internal data

}

//=============================
// ~CStratPRecvIntra
//=============================
CStratPRecvIntra::~CStratPRecvIntra (void)
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
GTI_RETURN CStratPRecvIntra::shutdown (
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
GTI_RETURN CStratPRecvIntra::communicationFinished (
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
        CStratPRecvBufInfo *info = get_free_buf ();
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
                myFreeBufs.push_back(info);
                info = myTestBuf;
                buf = (uint64_t*) info->buf; //The request used this buffer for receiving
                myTestBuf = NULL;
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

                //Destroy the old buf (without killing the buffer we took) and make a new one
                info->buf = NULL;
                delete info;
                info = get_free_buf ();
                buf = (uint64_t*) info->buf;

                //Queue it for later retrieval
                myReceivedUnexpectedMessages[channel].push_back(toQueue);
            }
            //Is it a long message?
            else if (buf[0] == myTokenLongMsg)
            {
                //Receive the long message from this channel
                uint64_t longMsgLen = ((uint64_t*)myTestBuf->buf)[1];
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

        myFreeBufs.push_back(info);
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

        CStratPRecvBufInfo* getUpdateInfo = get_free_buf ();

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
                    myFreeBufs.push_back(getUpdateInfo);
                    getUpdateInfo = myTestBuf;
                    buf = (uint64_t*) getUpdateInfo->buf;
                    myTestRequest = 0xFFFFFFFF;
                    myTestBuf = NULL;
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

                    //Destroy the old buf (without killing the buffer we took) and make a new one
                    getUpdateInfo->buf = NULL;
                    delete getUpdateInfo;
                    getUpdateInfo = get_free_buf ();
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

        //Clear the buf info we used
        myFreeBufs.push_back(getUpdateInfo);
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
GTI_RETURN CStratPRecvIntra::getNumPlaces (uint64_t *outNumPlaces)
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
GTI_RETURN CStratPRecvIntra::getOwnPlaceId (uint64_t *outId)
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
GTI_RETURN CStratPRecvIntra::send (
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
    //Remember the number of messages we have in total
    myNumMsgsSent++;

    //Is it a long message ?
    if (numBytes + 2*sizeof (uint64_t) > BUF_LENGTH) //2 -> message token + length
    {
        return send_long_message (toPlace, buf, numBytes, freeData, bufFreeFunction);
    }

    char* ourBuf = new char[numBytes + 2*sizeof (uint64_t)];
    memcpy (ourBuf+2*sizeof (uint64_t), buf, numBytes);
    ((uint64_t*)ourBuf)[0] = myTokenMessage;
    ((uint64_t*)ourBuf)[1] = numBytes;

    sendCommBuf (ourBuf, false, numBytes+2*sizeof (uint64_t), toPlace, NULL, longMsgBufFreeFunction);
    bufFreeFunction (freeData, numBytes, buf);

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratPRecvIntra::test (
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
    //** (1b) Did we have any unexpected messages during communicationFininshed?
    //**
    if (handleUnexpectedMessagesForReceive(outFlag, outFromPlace, outNumBytes, outBuf, outFreeData, outBufFreeFunction))
        return GTI_SUCCESS;

    //**
    //** (2) Do we already have a request for test calls ?
    //**
    if (myTestRequest == 0xFFFFFFFF)
    {
        if (!myTestBuf)
            myTestBuf = get_free_buf();
        protocol->irecv(myTestBuf->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
    }

    //**
    //** (3) Test for completion of the request
    //**
    int completed;
    uint64_t numbytes, channel;

    protocol->test_msg(myTestRequest,&completed,&numbytes,&channel);

    //**
    //** (4) Handle new buf if the test succeeded
    //**
    if (completed)
    {
        //==If we are the master, this may be an unexpected update
        if (((uint64_t*)myTestBuf->buf)[0] == myTokenUpdate)
        {
            /*
             * Take care of the update
             * After that we recurse the function to give it a new try.
             */
            handleUnexpectedUpdate((uint64_t*) myTestBuf->buf, channel);
            myFreeBufs.push_back (myTestBuf);
            myTestBuf = NULL;
            myTestRequest = 0xFFFFFFFF;
            return test (outFlag, outFromPlace, outNumBytes, outBuf,  outFreeData, outBufFreeFunction);
        }

        //==Regular message handling
        assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
        assert (((uint64_t*)myTestBuf->buf)[0] == myTokenMessage ||
                   ((uint64_t*)myTestBuf->buf)[0] == myTokenLongMsg    ); //has to be a message

        //Save request as used up
        myTestRequest = 0xFFFFFFFF;

        if (((uint64_t*)myTestBuf->buf)[0] == myTokenMessage)
        {
            //Received an buf
            if (outFlag) *outFlag = 1;
            if (outNumBytes) *outNumBytes =  ((uint64_t*)myTestBuf->buf)[1];
            if (outBuf) *outBuf = myTestBuf->buf+ 2*sizeof(uint64_t);
            if (outFreeData) *outFreeData = myTestBuf;
            if (outBufFreeFunction) *outBufFreeFunction = returnedBufBufFreeFunction;
            if (outFromPlace) *outFromPlace = channel;

            myTestBuf = NULL;

            myNumMsgsReceived++;
            return GTI_SUCCESS;
        }
        else
        {
            //Received info for a long message
            GTI_RETURN ret = long_msg_from_info (
                    ((uint64_t*)myTestBuf->buf)[1], channel,
                    outFlag, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);

            myFreeBufs.push_back (myTestBuf);
            myTestBuf = NULL;
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
GTI_RETURN CStratPRecvIntra::wait (
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
        if (!myTestBuf)
            myTestBuf = get_free_buf();
        protocol->recv(myTestBuf->buf, BUF_LENGTH, &numbytes, RECV_ANY_CHANNEL, &channel);
    }

    //**
    //** (4) Handle new message
    //**
    //==If we are the master, this may be an unexpected update
    if (((uint64_t*)myTestBuf->buf)[0] == myTokenUpdate)
    {
        /*
         * Take care of the update
         * After that we recurse the function to give it a new try.
         */
        handleUnexpectedUpdate((uint64_t*) myTestBuf->buf, channel);
        myFreeBufs.push_back (myTestBuf);
        myTestBuf = NULL;
        return wait (outFromPlace, outNumBytes, outBuf,  outFreeData, outBufFreeFunction);
    }

    //==Regular message handling
    assert (numbytes >= sizeof(uint64_t)*2); //each message has at least two entries
    assert (((uint64_t*)myTestBuf->buf)[0] == myTokenMessage ||
            ((uint64_t*)myTestBuf->buf)[0] == myTokenLongMsg    ); //has to be a message

    //Save request as used up
    myTestRequest = 0xFFFFFFFF;

    if (((uint64_t*)myTestBuf->buf)[0] == myTokenMessage)
    {
        //Received an buf
        if (outNumBytes) *outNumBytes =  ((uint64_t*)myTestBuf->buf)[1];
        if (outBuf) *outBuf = myTestBuf->buf+ 2*sizeof(uint64_t);
        if (outFreeData) *outFreeData = myTestBuf;
        if (outBufFreeFunction) *outBufFreeFunction = returnedBufBufFreeFunction;
        if (outFromPlace) *outFromPlace = channel;

        myTestBuf = NULL;

        myNumMsgsReceived++;
        return GTI_SUCCESS;
    }
    else
    {
        //Received info for a long message
        GTI_RETURN ret = long_msg_from_info (
                ((uint64_t*)myTestBuf->buf)[1], channel,
                NULL, outNumBytes, outBuf, outFreeData, outBufFreeFunction, outFromPlace);

        myFreeBufs.push_back (myTestBuf);
        myTestBuf = NULL;
        myNumMsgsReceived++;
        return ret;
    }

    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratPRecvIntra::flush (
        void
)
{
    return flush (false);
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CStratPRecvIntra::flushAndSetImmediate (void)
{
    //1) Flush
    flush ();

    //2) Set immediate

    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratPRecvIntra::flush (
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
    //We cann do that with a "sendCommBuf", if it gets a buffer=NULL, it just tests for completion
    sendCommBuf (
            NULL,
            false,
            0,
            0,
            NULL,
            NULL);

    //Do we want to block until all communications are done?
    if (block)
    {
        while (!myRequests.empty())
            completeOutstandingSendRequest(true, CStratIsendRequest());
    }//We need to block till all requests are completed

    return GTI_SUCCESS;
}

//=============================
// handleUnexpectedMessagesForReceive
//=============================
bool CStratPRecvIntra::handleUnexpectedMessagesForReceive (
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

            //What type of message is it: Buf OR long message
            if (item.free_data == NULL)
            {
                //->Buf
                CStratPRecvBufInfo* tempBuf = new CStratPRecvBufInfo ((char*)item.buf);
                tempBuf->instance = this;

                if (outFlag) *outFlag = 1;
                if (outNumBytes) *outNumBytes =  ((uint64_t*)tempBuf->buf)[1];
                if (outBuf) *outBuf = tempBuf->buf+ 2*sizeof(uint64_t);
                if (outFreeData) *outFreeData = tempBuf;
                if (outBufFreeFunction) *outBufFreeFunction = returnedBufBufFreeFunction;
                if (outFromPlace) *outFromPlace = fromIter->first;;
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
void CStratPRecvIntra::handleUnexpectedUpdate (uint64_t* buf, uint64_t channel)
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
void CStratPRecvIntra::completeOutstandingSendRequest (bool useMyRequests, CStratIsendRequest request)
{
    std::list<CStratIsendRequest>::iterator cur = myRequests.end();
    int completed = false;

    while (!completed)
    {
        CStratIsendRequest requestToUse;

        if (useMyRequests)
        {
            if (cur != myRequests.end()) cur++;
            if (cur == myRequests.end())
                cur = myRequests.begin();
            requestToUse = *cur;
        }
        else
        {
            requestToUse = request;
        }

        protocol->test_msg(requestToUse.my_request, &completed, NULL,NULL);

        if (!completed)
        {
            if (myTestRequest == 0xFFFFFFFF)
            {
                if (!myTestBuf)
                    myTestBuf = get_free_buf();
                protocol->irecv(myTestBuf->buf, BUF_LENGTH, &myTestRequest, RECV_ANY_CHANNEL);
            }

            int gotSomething;
            uint64_t numbytes, channel;

            protocol->test_msg(myTestRequest,&gotSomething,&numbytes,&channel);

            if (gotSomething)
            {
                myTestRequest = 0xFFFFFFFF;

                //Was it an update (may happen on master)?
                if (((uint64_t*)myTestBuf->buf)[0] == myTokenUpdate)
                {
                    handleUnexpectedUpdate((uint64_t*) myTestBuf->buf, channel);
                    myFreeBufs.push_back (myTestBuf);
                    myTestBuf = NULL;
                }
                else
                //Was it possibly a message token instead?
                if (((uint64_t*)myTestBuf->buf)[0] == myTokenMessage)
                {
                    //Store information about the message in our unexpected messages
                    CStratQueueItem toQueue;
                    toQueue.buf = myTestBuf->buf;
                    toQueue.buf_free_function = NULL;
                    toQueue.free_data = NULL;
                    toQueue.num_bytes = numbytes;
                    toQueue.toChannel = channel;

                    //Destroy the old buf (without killing the buffer we took) and make a new one
                    myTestBuf->buf = NULL;
                    delete myTestBuf;
                    myTestBuf = NULL;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);
                }
                //Is it a long message?
                else if (((uint64_t*)myTestBuf->buf)[0] == myTokenLongMsg)
                {
                    //Receive the long message from this channel
                    uint64_t longMsgLen = ((uint64_t*)myTestBuf->buf)[1];
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

                    myFreeBufs.push_back (myTestBuf);
                    myTestBuf = NULL;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);
                }
                //Unknown token?
                else
                {
                    std::cerr << "Internal GTI ERROR: check CStratPRecvIntra " << __FILE__ << ":" << __LINE__ << std::endl;
                    assert (0); //Internal error
                }
            }//Got some message?
        }//Completed a send request?
        else
        {
            requestToUse.free_buffer();

            if (useMyRequests)//We competed a request
            {
                myRequests.erase(cur);
                cur = myRequests.end();
            }
        }
    }//While nothing completed
}

/*EOF*/
