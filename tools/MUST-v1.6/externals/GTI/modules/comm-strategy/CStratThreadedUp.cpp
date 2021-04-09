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
 * @file CStratThreadedUp.cpp
 *        Application phase aware communication strategy.
 *
 * Provides the interface functionality with an implementation
 * that tries to avoid any communication. during phases in which the
 * application is communicating.
 *
 * @author Tobias Hilbrich
 * @date 24.08.2009
 */

#include "CStratThreadedUp.h"
#include "GtiMacros.h"
#include "GtiApi.h"

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>
#include <string.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratThreadedUp)
mFREE_INSTANCE_FUNCTION(CStratThreadedUp)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratThreadedUp)
mCOMM_STRATEGY_UP_RAISE_PANIC(CStratThreadedUp)

//=============================
// my_buf_free_function
//=============================
GTI_RETURN my_buf_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
	delete [] (uint64_t*) buf;
	return GTI_SUCCESS;
}

//=============================
// CStratThreadedUp
//=============================
CStratThreadedUp::CStratThreadedUp (const char* instanceName)
    : ModuleBase<CStratThreadedUp, CStratUpQueue> (instanceName),
      CStratThreadedAggregator (&protocol),
      myAggregationAllowed (true)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A Comm Strategy either uses a regular communication protocol or a regular and a
    //fall-back one.
    assert ((subModInstances.size() == 1) || (subModInstances.size() == 2));

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up own data
    myRequest = 0xFFFFFFFF;
    myGotPing = false;

    //Set up buffers immediately for our single communication channel
    myCurAggregateBufs.resize(1);
    myCommBufs.resize(1);
    myCurrAggregateLens.resize(1);
    myCurAggregateBufs[0] = NULL;
    myCommBufs[0] = NULL;
    myCurrAggregateLens[0] = 0;
    swap(0);
}

//=============================
// ~CStratThreadedUp
//=============================
CStratThreadedUp::~CStratThreadedUp (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;

#ifdef GTI_DEBUG
    //DEBUG OUT
    std::cout << "MaxReqs: " << myMaxNumReqs << std::endl;
#endif
}

//=============================
// shutdown
//=============================
GTI_RETURN CStratThreadedUp::shutdown (
        GTI_FLUSH_TYPE flush_behavior,
        GTI_SYNC_TYPE sync_behavior)
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
    {
        //start sending all we have
        flush ();

        //finish all sends
        while (!myRequests.empty())
            completeOutstandingSendRequest(true, 0);

        //remove outstanding requests
        while (!myRequests.empty())
        {
            AggRequestInfo req = myRequests.front();
            protocol->wait_msg(req.request,NULL,NULL);
            delete []req.buf;
            myRequests.pop_front();
        }
    }

    if (sync_behavior == GTI_SYNC)
    {
        uint64_t buf[2] = {0,0};
        uint64_t length;

        //Receive the ping (if not already received)
        while ((buf[0] != myTokenShutdownSync) && (!myGotPing))
        {
            if (myRequest == 0xFFFFFFFF)
            {
                protocol->recv(buf,sizeof(uint64_t)*2,&length,0,NULL);
            }
            else
            {
                protocol->wait_msg(myRequest,&length,NULL);
                buf[0] = myBuf[0];
                buf[1] = myBuf[1];
            }
            assert(length == sizeof(uint64_t)*2);
            myRequest = 0xFFFFFFFF;

            if (buf[0] != myTokenShutdownSync)
            {
                //@note we can give a warning here: received some other outstanding
                //     message even though this strategy is being closed
                //     Current decision: only give the warning in Debug mode
                //Receive the message, and discard it
#ifdef GTI_DEBUG
                std::cerr << "WARNING: In shutdown sync (StrategyUp) "
                        << "received an outstanding message!"
                        << std::endl;
#endif
                uint64_t *temp_buf;
                length = buf[1];
                if (length % sizeof(uint64_t)) length = ((length/sizeof(uint64_t))+1)*sizeof(uint64_t);
                temp_buf = new uint64_t[length/sizeof(uint64_t)];
                protocol->recv(temp_buf,buf[1],&length,0, NULL);
                delete [] temp_buf;
            }
        }

        //Send the pong
        buf[0] = myTokenShutdownSync;
        buf[1] = 0; //of no importance
        protocol->ssend(buf,sizeof(uint64_t)*2,0);
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
GTI_RETURN CStratThreadedUp::getPlaceId (uint64_t* outPlaceId)
{
    //==Check whether a connection exists
    if (!protocol->isConnected())
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    //==getPlaceId
    if (outPlaceId)
        protocol->getPlaceId(outPlaceId);

    return GTI_SUCCESS;
}

//=============================
// send
//=============================
GTI_RETURN CStratThreadedUp::send (
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

    //==send

    //DEBUG
    //std::cout << "SEND: [" << num_bytes << "]" << "id=" << ((uint64_t*) buf)[0] << std::endl;
    //ENDDEBUG

    //Is it a long message ?
    if (num_bytes + 3*sizeof (uint64_t) > BUF_LENGTH) //3 -> message token + num records + one record length
    {
        return send_long_message (0, buf, num_bytes, free_data, buf_free_function);
    }

    //Does it still fit into the aggregate buffer ?
    if (myCurrAggregateLens[0] + num_bytes + sizeof(uint64_t) > BUF_LENGTH)
    {
        //Its too long ...
        uint64_t comm_buf_length = myCurrAggregateLens[0];
        swap(0); //finish up outstanding communication of last buffer and swap buffers

        //DEBUG
        //std::cout << "SEND AGGREGATE: [" << comm_buf_length << "]" << std::endl;
        //ENDDEBUG

        /**
         * @todo beforehand we sent "comm_buf_length" as length,
         *            this only works (and saves performance) if the protocol automatically supports partial receives.
         *            Otherwise (e.g. TCP) we must send the full buffer length, some part potentially unused!
         *            This should make its way to the trunk at some point.
         */
        sendCommBuf (false, BUF_LENGTH, 0);
    }

    //Add the new message to the aggregate buffer
    uint64_t startIndex = (myCurrAggregateLens[0]/sizeof(uint64_t));
    ((uint64_t*)myCurAggregateBufs[0])[1] = ((uint64_t*)myCurAggregateBufs[0])[1] + 1; //increase count of msgs
    ((uint64_t*)myCurAggregateBufs[0])[startIndex] = num_bytes;
    myCurrAggregateLens[0] += sizeof (uint64_t);
    memmove (myCurAggregateBufs[0]+myCurrAggregateLens[0], buf, num_bytes);
    myCurrAggregateLens[0] += num_bytes;

    //pad to a multiple of "sizeof(uint64_t)"
    if (myCurrAggregateLens[0] % sizeof(uint64_t))
        myCurrAggregateLens[0] += sizeof(uint64_t) - myCurrAggregateLens[0] % sizeof(uint64_t);

    //remove padding if buffer becomes too long due to that
    if (myCurrAggregateLens[0] > BUF_LENGTH)
        myCurrAggregateLens[0] = BUF_LENGTH;

    //free given buffer
    buf_free_function (free_data, num_bytes, buf);

    //Are we still allowed to aggregate?
    if (!myAggregationAllowed)
        flush ();

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratThreadedUp::test (
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
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

    //**
    //** (1b) Did we have any unexpected messages during a swap?
    //**
    if (handleUnexpectedMessagesForReceive(out_flag, NULL, out_num_bytes, out_buf, out_free_data, out_buf_free_function))
        return GTI_SUCCESS;

    //==test
    if (myRequest == 0xFFFFFFFF)
    {
        protocol->irecv(myBuf,sizeof(uint64_t)*2,&myRequest,0);
    }

    int completed;
    uint64_t numbytes, channel;

    protocol->test_msg(myRequest,&completed,&numbytes,&channel);

    if (completed)
    {
        //Save request as used up
        myRequest = 0xFFFFFFFF;

        assert (numbytes == sizeof(uint64_t)*2); //has to be a token,length message

        //Did we receive the ping used for ping-pong shutdown sync ?
        if (myBuf[0] == myTokenShutdownSync)
        {
            assert (myGotPing == false);
            myGotPing = true;
            //Retry, maybe another message available ...
            return test (out_flag, out_num_bytes, out_buf, out_free_data, out_buf_free_function);
        }

        assert (myBuf[0] == myTokenMessage); //has to be a message

        uint64_t* buf;
        uint64_t  buf_size, recv_size;

        if (myBuf[1] % sizeof(uint64_t) != 0)
            buf_size = (myBuf[1]/sizeof(uint64_t))+1;
        else
            buf_size = (myBuf[1]/sizeof(uint64_t));

        buf = new uint64_t[buf_size];

        protocol->recv(buf,myBuf[1],&recv_size,channel,NULL);

        assert (recv_size == myBuf[1]); //must be the size specified in first message

        //save values
        *out_flag = 1;
        *out_num_bytes = recv_size;
        *out_buf = buf;
        *out_free_data = NULL;
        *out_buf_free_function = my_buf_free_function;
    }
    else
    {
        //No message received
        *out_flag = 0;
    }

    return GTI_SUCCESS;
}

//=============================
// wait
//=============================
GTI_RETURN CStratThreadedUp::wait (
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
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

    //**
    //** (1b) Did we have any unexpected messages during a swap?
    //**
    if (handleUnexpectedMessagesForReceive(NULL, NULL, out_num_bytes, out_buf, out_free_data, out_buf_free_function))
        return GTI_SUCCESS;

    //==wait
    uint64_t numbytes, channel;

    if (myRequest != 0xFFFFFFFF)
    {
        protocol->wait_msg(myRequest,&numbytes,&channel);
    }
    else
    {
        protocol->recv(myBuf,sizeof(uint64_t)*2,&numbytes,0,&channel);
    }

    //Save request as used up
    myRequest = 0xFFFFFFFF;

    //Sanity
    assert (numbytes == sizeof(uint64_t)*2); //has to be a token,length message

    //Did we receive the ping used for ping-pong shutdown sync ?
    if (myBuf[0] == myTokenShutdownSync)
    {
        assert (myGotPing == false);
        myGotPing = true;
        //Retry, maybe another message available ...
        return wait (out_num_bytes, out_buf, out_free_data, out_buf_free_function);
    }

    //Regular message
    assert (myBuf[0] == myTokenMessage); //has to be a message

    uint64_t* buf;
    uint64_t  buf_size, recv_size;

    if (myBuf[1] % sizeof(uint64_t) != 0)
        buf_size = (myBuf[1]/sizeof(uint64_t))+1;
    else
        buf_size = (myBuf[1]/sizeof(uint64_t));

    buf = new uint64_t[buf_size];

    protocol->recv(buf,myBuf[1],&recv_size,channel, NULL);

    assert (recv_size == myBuf[1]); //must be the size specified in first message

    //save values
    *out_num_bytes = recv_size;
    *out_buf = buf;
    *out_free_data = NULL;
    *out_buf_free_function = my_buf_free_function;

    return GTI_SUCCESS;
}

//=============================
// raisePanic
//=============================
GTI_RETURN CStratThreadedUp::raisePanic (void)
{
    //1) Flush
    flush ();

    //2) Disable future aggregation
    myAggregationAllowed = false;

    //3) Create the GTI internal panic event
    gtiRaisePanicP f;
    if (getWrapperFunction("gtiRaisePanic", (GTI_Fct_t*) &f) == GTI_SUCCESS)
    {
        //Raise the panic
        (*f) ();
    }
    else
    {
        std::cerr << "MUST internal error: could not find the creation function for the GTI internal event \"gtiRaisePanic\", this should never happen (" << __FILE__ << ":" << __LINE__ << ")." << std::endl;
        return GTI_ERROR;
    }

    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratThreadedUp::flush (
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
    int completed;

    //Test for completion of existing send operations
    uint64_t old_len = myCurrAggregateLens[0];
    if (old_len > 2*sizeof(uint64_t))
    {
        swap(0); //finish sending old buffer and swap buffers

        //DEBUG
        //std::cout << "SEND SHUTDOWN AGGREGATE: [" << old_len << "]" << std::endl;
        //ENDDEBUG

        /**
         * @todo see the other sendCommBuf above and its comment!
         */
        sendCommBuf (false, BUF_LENGTH, 0);
    }

    return GTI_SUCCESS;
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CStratThreadedUp::flushAndSetImmediate (void)
{
    //1) Flush
    flush ();

    //2) Set immediate
    myAggregationAllowed = false;

    return GTI_SUCCESS;
}

//=============================
// completeOutstandingSendRequest
//=============================
void CStratThreadedUp::completeOutstandingSendRequest (bool useMyRequests, unsigned int request)
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
            if (myRequest == 0xFFFFFFFF)
            {
                protocol->irecv(myBuf,sizeof(uint64_t)*2,&myRequest,0);
            }

            int gotSomething;
            uint64_t numbytes, channel;

            protocol->test_msg(myRequest,&gotSomething,&numbytes,&channel);

            if (gotSomething)
            {
                myRequest = 0xFFFFFFFF;

                assert (numbytes == sizeof(uint64_t)*2); //has to be a token,length message

                //Did we receive the ping used for ping-pong shutdown sync ?
                if (myBuf[0] == myTokenShutdownSync)
                {
                    assert (myGotPing == false);
                    myGotPing = true;
                }
                else
                {
                    //-- Receive the actual message
                    assert (myBuf[0] == myTokenMessage); //has to be a message

                    uint64_t* buf;
                    uint64_t  buf_size, recv_size;

                    if (myBuf[1] % sizeof(uint64_t) != 0)
                        buf_size = (myBuf[1]/sizeof(uint64_t))+1;
                    else
                        buf_size = (myBuf[1]/sizeof(uint64_t));

                    buf = new uint64_t[buf_size];

                    protocol->recv(buf,myBuf[1],&recv_size,channel,NULL);

                    assert (recv_size == myBuf[1]); //must be the size specified in first message

                    //-- Queue the new message
                    CStratQueueItem toQueue;
                    toQueue.buf = buf;
                    toQueue.buf_free_function = my_buf_free_function;
                    toQueue.free_data = NULL;
                    toQueue.num_bytes = recv_size;
                    toQueue.toChannel = 0;

                    //Queue it for later retrieval
                    myReceivedUnexpectedMessages.push_back(toQueue);
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

//=============================
// handleUnexpectedMessagesForReceive
//=============================
bool CStratThreadedUp::handleUnexpectedMessagesForReceive (
                int* outFlag,
                uint64_t *outFromPlace,
                uint64_t* outNumBytes,
                void** outBuf,
                void** outFreeData,
                GTI_RETURN (**outBufFreeFunction) (void* freeData, uint64_t numBytes, void* buf)
        )
{
    std::list<CStratQueueItem>::iterator queueIter;

    for (queueIter = myReceivedUnexpectedMessages.begin(); queueIter != myReceivedUnexpectedMessages.end(); queueIter++)
    {
        CStratQueueItem item = *queueIter;
        myReceivedUnexpectedMessages.erase (queueIter);

        if (outFlag)
            *outFlag = 1;
        if (outFromPlace)
            *outFromPlace = item.toChannel;
        if (outNumBytes)
            *outNumBytes = item.num_bytes;
        if (outBuf)
            *outBuf = item.buf;
        if (outFreeData)
            *outFreeData = NULL;
        if (outBufFreeFunction)
            *outBufFreeFunction = item.buf_free_function;

        return true;
    }

    if (outFlag)
        *outFlag = 0;

    return false;
}

/*EOF*/
