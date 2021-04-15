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
 * @file CStratIsendUp.cpp
 *        Non-blocking send based implementation for the I_CommStrategyUp interface.
 *
 * Provides the interface functionality with a simple implementation,
 * which uses non-blocking sends.
 *
 * @author Tobias Hilbrich
 * @date 03.08.2009
 */

#include "CStratIsendUp.h"
#include "GtiMacros.h"
#include "GtiApi.h"

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>


using namespace gti;

mGET_INSTANCE_FUNCTION(CStratIsendUp)
mFREE_INSTANCE_FUNCTION(CStratIsendUp)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratIsendUp)
mCOMM_STRATEGY_UP_RAISE_PANIC(CStratIsendUp)

//=============================
// my_buf_free_function
//=============================
GTI_RETURN my_buf_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratIsendUp
//=============================
CStratIsendUp::CStratIsendUp (const char* instanceName)
    : ModuleBase<CStratIsendUp, CStratUpQueue> (instanceName),
      myReceivedUnexpectedMessages ()
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
// ~CStratIsendUp
//=============================
CStratIsendUp::~CStratIsendUp (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;
}


//=============================
// shutdown
//=============================
GTI_RETURN CStratIsendUp::shutdown (
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
                myRequest = 0xFFFFFFFF;
            }

            myRequest = 0xFFFFFFFF;
            assert(length == sizeof(uint64_t)*2);

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
                delete[] temp_buf;
            }
        }

        //Send the pong
        buf[0] = myTokenShutdownSync;
        buf[1] = 0; //of no importance
        protocol->ssend(buf,sizeof(uint64_t)*2,0);
    }

    // Remove anything that remains
    protocol->removeOutstandingRequests ();

    //Shutdown the protocols
    protocol->shutdown();
    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CStratIsendUp::getPlaceId (uint64_t* outPlaceId)
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
GTI_RETURN CStratIsendUp::send (
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
    uint64_t *temp_buf = new uint64_t[2];
    temp_buf[0] = myTokenMessage;
    temp_buf[1] = num_bytes;
    unsigned int req1, req2;
    int completed = myRequests.size();
    GTI_RETURN ret;
    bool doneMsg1 = false, doneMsg2 = false;

    /*
     * We now loop until we send out the message, in the loop we do:
     * - try to finish existing send requests (possibly blocking if there are too many of these)
     * - create an isend for the message length
     * - create an isend for the actuall message
     */
    while (!doneMsg1 || !doneMsg2)
    {
        //Test for completion of existing send operations
        while (!myRequests.empty() && (completed || myRequests.size() >= myMaxNumRequests))
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
            else if (myRequests.size() >= myMaxNumRequests)
            {
                checkIncomingMessage();
            }
        }

        if (!doneMsg1)
        {
            ret = protocol->isend(temp_buf,sizeof(uint64_t)*2, &req1, 0);
            if (ret == GTI_ERROR)
            {
                return ret;
            }
            else if (ret == GTI_ERROR_OUTSTANDING_LIMIT)
            {
                doneMsg1 = false;
            }
            else
            {
                doneMsg1 = true;
                myRequests.push_back(CStratIsendRequest (temp_buf, sizeof(uint64_t)*2, NULL , my_buf_free_function, req1));
            }
        }

        if (doneMsg1 && !doneMsg2)
        {
            ret = protocol->isend(buf,num_bytes, &req2, 0);
            if (ret == GTI_ERROR)
            {
                return ret;
            }
            else if (ret == GTI_ERROR_OUTSTANDING_LIMIT)
            {
                doneMsg2 = false;
            }
            else
            {
                doneMsg2 = true;
                myRequests.push_back(CStratIsendRequest (buf, num_bytes, free_data , buf_free_function, req2));
            }
        }
    }

    return GTI_SUCCESS;
}

//=============================
// test
//=============================
GTI_RETURN CStratIsendUp::test (
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

    //==Did we got an unexpected message?
    if (!myReceivedUnexpectedMessages.empty())
    {
        CStratQueueItem item = myReceivedUnexpectedMessages.front();
        myReceivedUnexpectedMessages.pop_front();
        if (out_flag) *out_flag = 1;
        if (out_num_bytes) *out_num_bytes = item.num_bytes;
        if (out_buf) *out_buf = item.buf;
        if (out_free_data) *out_free_data = item.free_data;
        if (out_buf_free_function) *out_buf_free_function = item.buf_free_function;
        return GTI_SUCCESS;
    }

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
GTI_RETURN CStratIsendUp::wait (
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

    //==Did we got an unexpected message?
    if (!myReceivedUnexpectedMessages.empty())
    {
        CStratQueueItem item = myReceivedUnexpectedMessages.front();
        myReceivedUnexpectedMessages.pop_front();
        if (out_num_bytes) *out_num_bytes = item.num_bytes;
        if (out_buf) *out_buf = item.buf;
        if (out_free_data) *out_free_data = item.free_data;
        if (out_buf_free_function) *out_buf_free_function = item.buf_free_function;
        return GTI_SUCCESS;
    }

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
GTI_RETURN CStratIsendUp::raisePanic (void)
{
    //1) Flush
    flush ();

    //2) Disable future aggregation
    /*nothing to do*/

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
GTI_RETURN CStratIsendUp::flush (
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
    while (!myRequests.empty())
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
        else
        {
            checkIncomingMessage();
        }
    }

    return GTI_SUCCESS;
}

//=============================
// flushAndSetImmediate
//=============================
GTI_RETURN CStratIsendUp::flushAndSetImmediate (void)
{
    //1) Flush
    flush ();

    //2) Set immediate
    /*Nothing to do, always immediate*/

    return GTI_SUCCESS;
}

//=============================
// checkIncomingMessage
//=============================
void CStratIsendUp::checkIncomingMessage (void)
{
    if (myRequest == 0xFFFFFFFF)
    {
        protocol->irecv(myBuf,sizeof(uint64_t)*2,&myRequest,0);
    }

    int rcompleted = 0;
    uint64_t numbytes, channel;

    protocol->test_msg(myRequest,&rcompleted,&numbytes,&channel);

    if (rcompleted)
    {
        //Save request as used up
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

            //save values and add to unexpected messages
            CStratQueueItem item;

            item.num_bytes = recv_size;
            item.buf = buf;
            item.free_data = NULL;
            item.toChannel = 0;
            item.buf_free_function = my_buf_free_function;

            myReceivedUnexpectedMessages.push_back(item);
        }
    }
}

/*EOF*/
