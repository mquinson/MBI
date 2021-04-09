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
 * @file CStratSimpleIntra.cpp
 *       @see I_CommStrategyIntra.
 *
 * @author Tobias Hilbrich
 * @date 16.01.2012
 */

#include "CStratIsendIntra.h"
#include "GtiMacros.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratIsendIntra)
mFREE_INSTANCE_FUNCTION(CStratIsendIntra)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratIsendIntra)

const uint64_t gti::CStratIsendIntra::myTokenUpdate = 0xFFFFFFFD;
const uint64_t gti::CStratIsendIntra::myTokenAcknowledge = 0xFFFFFFFC;

//=============================
// myBufFreeFunction
//=============================
GTI_RETURN myBufFreeFunction (void* freeData, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratIsendIntra
//=============================
CStratIsendIntra::CStratIsendIntra (const char* instanceName)
    : ModuleBase<CStratIsendIntra, CStratIntraQueue> (instanceName),
      CStratIsend (),
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

    //An intra Comm Strategy uses a single communication protocol
    assert (subModInstances.size() == 1);

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up own data
    myRequest = 0xFFFFFFFF;
    myBuf[0] = myBuf[1] = 0; //of no importance
}

//=============================
// ~CStratIsendIntra
//=============================
CStratIsendIntra::~CStratIsendIntra (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;
}

//=============================
// shutdown
//=============================
GTI_RETURN CStratIsendIntra::shutdown (GTI_FLUSH_TYPE flush_behavior)
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
GTI_RETURN CStratIsendIntra::communicationFinished (bool *pOutIsFinished)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //Ids & Channel size
    uint64_t numChannels;
    uint64_t thisPlaceId;
    protocol->getPlaceId(&thisPlaceId);
    protocol->getNumChannels(&numChannels);

    //Data for the messages
    uint64_t buf[2];
    bool areWeFininshed = false;

    if (pOutIsFinished)
        *pOutIsFinished = false;

    //= A) SLAVE
    if (thisPlaceId != 0)
    {
        //== 1) Send an update to the master
        unsigned int sendRequest;
        buf[0] = myTokenUpdate;
        buf[1] = (uint64_t)(myNumMsgsSent-myNumMsgsReceived);
        //protocol->ssend(buf, sizeof(uint64_t) * 2, 0);
        protocol->isend(buf, sizeof(uint64_t) * 2, &sendRequest, 0);
        int completed;
        bool gotAck = false;

        //Loop until we got rid of the token update, in between we receive anything that comes in!
        do
        {
            //Test on send
            protocol->test_msg(sendRequest, &completed, NULL, NULL);

            //If send not done test for other messages
            if (!completed)
            {
                //Open up request if necessary
                if (myRequest == 0xFFFFFFFF)
                {
                    protocol->irecv (myBuf, sizeof(uint64_t)*2, &myRequest, RECV_ANY_CHANNEL);
                }

                //Test on the receive request
                int completedR;
                uint64_t numbytes, channel;

                protocol->test_msg (myRequest ,&completedR, &numbytes, &channel);

                if (completedR)
                {
                    if (myBuf[0] == myTokenMessage)
                    {
                        //Prepare and receive the message
                        CStratQueueItem toQueue;
                        toQueue.toChannel = channel;

                        handleReceivedMessageToken (
                                buf,
                                channel,
                                &toQueue.num_bytes,
                                &toQueue.buf,
                                &toQueue.free_data,
                                &toQueue.buf_free_function
                        );

                        //Queue it for later retrieval
                        myReceivedUnexpectedMessages[channel].push_back(toQueue);
                    }
                    else if (buf[0] == myTokenAcknowledge)
                    {
                        gotAck = true;
                    }
                }
            }
        } while (!completed);

        //== 2) Receive Acknowledgement
        uint64_t length;
        uint64_t channel = 0;

        while (!gotAck)
        {
            channel = 0;

            //Do we have an open request? If so we need to use that one
            if (myRequest != 0xFFFFFFFF)
            {
                protocol->wait_msg (myRequest, &length, &channel);
                buf[0] = myBuf[0];
                buf[1] = myBuf[1];
                myRequest = 0xFFFFFFFF;
            }
            else
            {
                /*
                 * IMPORTANT: using RECV_ANY_CHANNEL may seem counter intuitive,
                 * as we want something from rank 0. BUT, we must allow all our partners
                 * to progress as to also join communicationFinished eventually!
                 */
                protocol->recv(buf, sizeof(uint64_t) * 2, &length, RECV_ANY_CHANNEL, &channel);
            }

            //Was it possibly a message token instead?
            if (buf[0] == myTokenMessage)
            {
                //Prepare and receive the message
                CStratQueueItem toQueue;
                toQueue.toChannel = channel;

                handleReceivedMessageToken (
                                buf,
                                channel,
                                &toQueue.num_bytes,
                                &toQueue.buf,
                                &toQueue.free_data,
                                &toQueue.buf_free_function
                                );

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
        }
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

        //== 2) Wait for an update from everyone
        while (myNumUpdates != numChannels)
        {
            bool gotUpdate = false;
            uint64_t channel = 0;
            uint64_t length;

            while (myChannelState[nextChannel] == true)
                nextChannel++;

            channel = nextChannel;

            do
            {
                //Do we have an open request? If so we need to use that one
                if (myRequest != 0xFFFFFFFF)
                {
                    protocol->wait_msg (myRequest, &length, &channel);
                    buf[0] = myBuf[0];
                    buf[1] = myBuf[1];
                    myRequest = 0xFFFFFFFF;
                }
                else
                {
                    /*
                     * IMPORTANT: using RECV_ANY_CHANNEL may seem counter intuitive,
                     * as we want something from rank "channel". BUT, we must allow all our partners
                     * to progress as to also join communicationFinished eventually!
                     */
                    protocol->recv(buf, sizeof(uint64_t) * 2, &length, RECV_ANY_CHANNEL, &channel);
                }

                //Was it possibly a message token instead?
                if (buf[0] == myTokenMessage)
                {
                    //Prepare and receive the message
                    CStratQueueItem toQueue;
                    toQueue.toChannel = channel;

                    handleReceivedMessageToken (
                            buf,
                            channel,
                            &toQueue.num_bytes,
                            &toQueue.buf,
                            &toQueue.free_data,
                            &toQueue.buf_free_function
                    );

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
    }

    if (areWeFininshed)
    {
        if (pOutIsFinished)
            *pOutIsFinished = true;

        myCommFinished = true;
    }

    return GTI_SUCCESS;
}

//=============================
// getNumPlaces
//=============================
GTI_RETURN CStratIsendIntra::getNumPlaces (uint64_t *outNumPlaces)
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
GTI_RETURN CStratIsendIntra::getOwnPlaceId (uint64_t *outId)
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
GTI_RETURN CStratIsendIntra::send (
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
        AddToQueue (buf, numBytes, freeData, bufFreeFunction, toPlace);
        return GTI_SUCCESS;
    }

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==send
    uint64_t *temp_buf = new uint64_t[2];
    temp_buf[0] = myTokenMessage;
    temp_buf[1] = numBytes;
    unsigned int req1, req2;
    int completed = true;

    //Test for completion of existing send operations
    while (!myRequests.empty() && completed)
    {
        protocol->test_msg (
                myRequests.front().get_request(),
                &completed,
                NULL,
                NULL
        );

        if (completed)
        {
            myRequests.front().free_buffer();
            myRequests.pop_front ();
        }
    }

    //Do we have too many requests, if so enforce completion!
    while (myRequests.size() >= myMaxNumRequests)
        finishFirstSendRequest ();

    myNumMsgsSent++;
    protocol->isend(temp_buf,sizeof(uint64_t)*2, &req1, toPlace);
    protocol->isend(buf,numBytes, &req2, toPlace);

    myRequests.push_back(CStratIsendRequest (temp_buf, sizeof(uint64_t)*2, NULL , myBufFreeFunction, req1));
    myRequests.push_back(CStratIsendRequest (buf, numBytes, freeData , bufFreeFunction, req2));

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratIsendIntra::test (
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
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==Check whether we got unexpected messages during communicationFininshed
    if (handleUnexpectedMessagesForReceive(outFlag, outFromPlace, outNumBytes, outBuf, outFreeData, outBufFreeFunction))
        return GTI_SUCCESS;

    //==test
    if (myRequest == 0xFFFFFFFF)
    {
        protocol->irecv (myBuf, sizeof(uint64_t)*2, &myRequest, RECV_ANY_CHANNEL);
    }

    int completed;
    uint64_t numbytes, channel;

    protocol->test_msg (myRequest ,&completed, &numbytes, &channel);

    if (completed)
    {
        //If we are the master, this may be a unexpected update
        if (myBuf[0] == myTokenUpdate)
        {
            handleUnexpectedUpdate(myBuf, channel);
            return test (outFlag, outFromPlace, outNumBytes, outBuf,  outFreeData, outBufFreeFunction);
        }

        //Regular message
        myNumMsgsReceived++;

        //Received a message
        if (outFlag)
            *outFlag = 1;

        assert (numbytes == sizeof(uint64_t)*2); //has to be a token, length message
        return handleReceivedMessageToken (
                        myBuf,
                        channel,
                        outNumBytes,
                        outBuf,
                        outFreeData,
                        outBufFreeFunction
                        );
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
GTI_RETURN CStratIsendIntra::wait (
        uint64_t* outFromPlace,
        uint64_t* outNumBytes,
        void** outBuf,
        void** outFreeData,
        GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==Check whether we got unexpected messages during communicationFininshed
    if (handleUnexpectedMessagesForReceive(NULL, outFromPlace, outNumBytes, outBuf, outFreeData, outBufFreeFunction))
        return GTI_SUCCESS;

    //==Wait
    uint64_t numbytes, channel;

    if (myRequest != 0xFFFFFFFF)
    {
        protocol->wait_msg (myRequest, &numbytes, &channel);
    }
    else
    {
        protocol->recv(myBuf, sizeof(uint64_t)*2, &numbytes, RECV_ANY_CHANNEL, &channel);
    }

    //If we are the master, this may be a unexpected update
    if (myBuf[0] == myTokenUpdate)
    {
        handleUnexpectedUpdate(myBuf, channel);
        return wait (outFromPlace, outNumBytes, outBuf,  outFreeData, outBufFreeFunction);
    }

    //Handle an actual message
    myNumMsgsReceived++;
    assert (numbytes == sizeof(uint64_t)*2); //has to be a token, length message
    return handleReceivedMessageToken (
                            myBuf,
                            channel,
                            outNumBytes,
                            outBuf,
                            outFreeData,
                            outBufFreeFunction
                            );
}

//=============================
// handleUnexpectedUpdate
//=============================
void CStratIsendIntra::handleUnexpectedUpdate (uint64_t* buf, uint64_t channel)
{
    //Save request as used up
    myRequest = 0xFFFFFFFF;

    uint64_t numChannels;
    protocol->getNumChannels(&numChannels);

    myNumUpdates++;
    if (myChannelState.empty())
        myChannelState.resize(numChannels, false);
    myChannelState[channel] = true;

    mySumDiffs += (long) buf[1];
}

//=============================
// handleUnexpectedMessagesForReceive
//=============================
bool CStratIsendIntra::handleUnexpectedMessagesForReceive (
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

            if (outFlag)
                *outFlag = 1;
            if (outFromPlace)
                *outFromPlace = fromIter->first;
            if (outNumBytes)
                *outNumBytes = item.num_bytes;
            if (outBuf)
                *outBuf = item.buf;
            if (outFreeData)
                *outFreeData = item.free_data;
            if (outBufFreeFunction)
                *outBufFreeFunction = item.buf_free_function;

            fromIter->second.erase (queueIter);
            myNumMsgsReceived++;

            return true;
        }
    }

    if (outFlag)
        *outFlag = 0;

    return false;
}

//=============================
// flush
//=============================
GTI_RETURN CStratIsendIntra::flush (void)
{
    return flush (false);
}

//=============================
// flush
//=============================
GTI_RETURN CStratIsendIntra::flush (bool block)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
        return GTI_ERROR_NOT_INITIALIZED;

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==flush
    //Test for completion of existing send operations
    //Blocking till everything arrives appears to be not so good an idea
    if (block)
    {
        while (!myRequests.empty())
            finishFirstSendRequest ();
    }

    return GTI_SUCCESS;
}

//=============================
// handleReceivedMessageToken
//=============================
GTI_RETURN CStratIsendIntra::handleReceivedMessageToken (
                uint64_t* tokenMessage,
                uint64_t channel,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
                )
{
    //Save request as used up
    myRequest = 0xFFFFFFFF;

    assert (tokenMessage[0] == myTokenMessage); //has to be a message

    uint64_t* buf;
    uint64_t  bufSize, recvSize;

    if (tokenMessage[1] % sizeof(uint64_t) != 0)
        bufSize = (tokenMessage[1]/sizeof(uint64_t))+1;
    else
        bufSize = (tokenMessage[1]/sizeof(uint64_t));

    buf = new uint64_t[bufSize];

    protocol->recv (buf, tokenMessage[1], &recvSize, channel, NULL);

    assert (recvSize == tokenMessage[1]); //must be the size specified in first message

    //save values
    if (outNumBytes)
        *outNumBytes = recvSize;
    if (outBuf)
        *outBuf = buf;
    if (outFreeData)
        *outFreeData = NULL;
    if (outBufFreeFunction)
        *outBufFreeFunction = myBufFreeFunction;

    return GTI_SUCCESS;
}

//=============================
// finishFirstSendRequest
//=============================
void CStratIsendIntra::finishFirstSendRequest (void)
{
    int completed = false;
    std::list<CStratIsendRequest>::iterator cur = myRequests.end();

    //Test for completion of existing send operations
    while (!myRequests.empty() && !completed)
    {
        if (cur == myRequests.end())
            cur = myRequests.begin();
        else
            cur++;

        if (cur == myRequests.end())
            cur = myRequests.begin();

        protocol->test_msg (
                cur->get_request(),
                &completed,
                NULL,
                NULL
        );

        if (!completed)
        {
            /*
             * We have to wait for some completion while apparently none is available right now.
             * If we busy wait till we have a completion we may deadlock as all tasks may
             * collectively end in the same situation. THUS, we must also check for incoming messages
             * and receive them, with that we can satisfy the requests of our beloved colleagues.
             * This should allow us to avoid deadlock, but may be heavy on our internal buffering ...
             */
            //==test
            if (myRequest == 0xFFFFFFFF)
            {
                protocol->irecv (myBuf, sizeof(uint64_t)*2, &myRequest, RECV_ANY_CHANNEL);
            }

            int gotSomething;
            uint64_t numbytes, channel;

            protocol->test_msg (myRequest ,&gotSomething, &numbytes, &channel);

            if (gotSomething)
            {
                //Save request as used up
                myRequest = 0xFFFFFFFF;

                //If we are the master, this may be an unexpected update
                if (myBuf[0] == myTokenUpdate)
                {
                    handleUnexpectedUpdate(myBuf, channel);
                }
                else
                if (myBuf[0] == myTokenMessage)
                {
                    //If we got a message we receive it completely and put it into the unexpected messages for later retrieval
                    CStratQueueItem toQueue;
                    toQueue.toChannel = channel;

                    handleReceivedMessageToken (
                            myBuf,
                            channel,
                            &toQueue.num_bytes,
                            &toQueue.buf,
                            &toQueue.free_data,
                            &toQueue.buf_free_function
                    );

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages[channel].push_back(toQueue);
                }
                else
                {
                    //Internal error
                    assert(0);
                }
            }/*Did we catch some incoming message?*/
        }
        else /*did we complete an outstanding send request?*/
        {
            cur->free_buffer();
            myRequests.erase(cur);
        }
    }
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CStratIsendIntra::flushAndSetImmediate ()
{
    //1) Flush
    flush ();

    //2) Set immediate
    /*Nothing to do, always immediate*/

    return GTI_SUCCESS;
}

/*EOF*/
