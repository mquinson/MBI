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
 * @file CStratSimpleUp.cpp
 *        Simple implementation for the I_CommStrategyUp interface.
 *
 * Provides the interface functionality with a simple implementation.
 *
 * @author Tobias Hilbrich
 * @date 16.04.2009
 */

#include "CStratSimpleUp.h"
#include "GtiMacros.h"
#include "GtiApi.h"

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>

using namespace gti;

const uint64_t CStratSimpleUp::myTokenShutdownSync = 0xFFFFFFFF;
const uint64_t CStratSimpleUp::myTokenMessage = 0xFFFFFFFE;
const uint64_t CStratSimpleUp::myTokenAck = 0xFFFFFFFD;

mGET_INSTANCE_FUNCTION(CStratSimpleUp)
mFREE_INSTANCE_FUNCTION(CStratSimpleUp)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratSimpleUp)
mCOMM_STRATEGY_UP_RAISE_PANIC(CStratSimpleUp)

//=============================
// buf_free_function
//=============================
GTI_RETURN buf_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratSimpleUp
//=============================
CStratSimpleUp::CStratSimpleUp (const char* instanceName)
    : ModuleBase<CStratSimpleUp, CStratUpQueue> (instanceName),
      myMsgQueue()
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A Comm Strategy either uses a regular communication protocol or a regular and a
    //fall-back one.
    assert ((subModInstances.size() == 1) || (subModInstances.size() == 2));

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up own data
    myRequest = 0xFFFFFFFF;
    myBuf[0] = myBuf[1] = 0; //of no importance
    myGotPing = false;
}

//=============================
// ~CStratSimpleUp
//=============================
CStratSimpleUp::~CStratSimpleUp (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;
}

//=============================
// shutdown
//=============================
GTI_RETURN CStratSimpleUp::shutdown (
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
        flush ();

    // Remove protocol requests, to avoid sync msgs beeing catched by
    // outstanding requests !
    protocol->removeOutstandingRequests ();

    if (sync_behavior == GTI_SYNC)
    {
        uint64_t buf[2] = {0,0};
        uint64_t length;

        //Receive the ping (if not already received)
        while ((buf[0] != myTokenShutdownSync) && (!myGotPing))
        {
            protocol->recv(buf,sizeof(uint64_t)*2,&length,0,NULL);
            assert(length == sizeof(uint64_t)*2);

            if (buf[0] != myTokenShutdownSync)
            {
                //TODO give a warning here, received some other outstanding
                //     message even though this strategy is being closed
                //Receive the message, and discard it
                std::cerr << "WARNING: In shutdown sync (StrategyUp) "
                          << "received an outstanding message!"
                          << std::endl;
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

    //Shutdown the protocols
    protocol->shutdown();

    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CStratSimpleUp::getPlaceId (uint64_t* outPlaceId)
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
GTI_RETURN CStratSimpleUp::send (
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
    uint64_t temp_buf[2] = {myTokenMessage, num_bytes};
    protocol->ssend(temp_buf,sizeof(uint64_t)*2,0);
    protocol->ssend(buf,num_bytes,0);

    //Wait for the acknowledgment
    bool hadAck = false;
    while (!hadAck)
    {
        uint64_t length;

        if (myRequest == 0xFFFFFFFF)
        {
            protocol->recv(temp_buf, sizeof(uint64_t)*2, &length, 0, NULL);
        }
        else
        {
            protocol->wait_msg(myRequest,&length, NULL);
            temp_buf[0] = myBuf[0];
            temp_buf[1] = myBuf[1];
            myRequest = 0xFFFFFFFF;
        }

        if (temp_buf[0] == myTokenShutdownSync)
        {
            //We got a shutdown ping instead
            assert (myGotPing == false);
            myGotPing = true;
        }
        else if (temp_buf[0] == myTokenMessage)
        {
            //We got a regular message instead
            uint64_t buf_size, recv_size;
            if (temp_buf[1] % sizeof(uint64_t) != 0)
                buf_size = (temp_buf[1]/sizeof(uint64_t))+1;
            else
                buf_size = (temp_buf[1]/sizeof(uint64_t));

            uint64_t* tempBuf = new uint64_t[buf_size];

            protocol->recv(tempBuf,temp_buf[1],&recv_size,0, NULL);

            assert (recv_size == temp_buf[1]); //must be the size specified in first message

            myMsgQueue.push_back(std::make_pair(recv_size, tempBuf));
        }
        else
        {
            //We got the acknowledgement
            assert (temp_buf[0] == myTokenAck);
            hadAck = true;
        }
    }

    if (buf_free_function)
        (*buf_free_function) (free_data,num_bytes,buf);

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratSimpleUp::test (
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    //Check whether a connection exists
    if (!protocol->isConnected())
    {
    	return GTI_ERROR_NOT_INITIALIZED;
    }

    //Check whether there are queued sends, process if so
    if (hasQueueEntries())
    	ProcessQueue ();

    //Did we receive messages earlier on (when waiting for an ack)
    if (!myMsgQueue.empty())
    {
		*out_flag = 1;
		*out_num_bytes = myMsgQueue.front().first;
		*out_buf = myMsgQueue.front().second;
		*out_free_data = NULL;
		*out_buf_free_function = buf_free_function;

		myMsgQueue.pop_front();

		return GTI_SUCCESS;
    }

    //test
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
        *out_buf_free_function = buf_free_function;
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
GTI_RETURN CStratSimpleUp::wait (
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    //Check whether a connection exists
    if (!protocol->isConnected())
    {
    	return GTI_ERROR_NOT_INITIALIZED;
    }

    //Check whether there are queued sends, process if so
    if (hasQueueEntries())
    	ProcessQueue ();

    //Did we receive messages earlier on (when waiting for an ack)
    if (!myMsgQueue.empty())
    {
			*out_num_bytes = myMsgQueue.front().first;
			*out_buf = myMsgQueue.front().second;
			*out_free_data = NULL;
			*out_buf_free_function = buf_free_function;

			myMsgQueue.pop_front();
			return GTI_SUCCESS;
    }

    //wait
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
    *out_buf_free_function = buf_free_function;

    return GTI_SUCCESS;
}

//=============================
// raisePanic
//=============================
GTI_RETURN CStratSimpleUp::raisePanic (void)
{
    //1) Flush
    flush ();

    //2) Disable future aggregation
    /*Nothing to do*/

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
GTI_RETURN CStratSimpleUp::flush (
        void
        )
{

    //Check whether there are queued sends, process if so
    if (hasQueueEntries())
    	ProcessQueue ();

    //Flush
    //No action needed
    //->send uses ssend

    return GTI_SUCCESS;
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CStratSimpleUp::flushAndSetImmediate (void)
{
    //1) Flush
    flush ();

    //2) Set immediate
    /*Nothing to do, always immediate*/

    return GTI_SUCCESS;
}

/*EOF*/
