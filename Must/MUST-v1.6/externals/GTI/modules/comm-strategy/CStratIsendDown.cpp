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
 * @file CommStrategyDownTemplate.cpp
 *       Non-blocking send based implementation for the I_CommStrategyDown interface.
 *
 * Provides the interface functionality with a simple implementation,
 * which uses non-blocking sends.
 *
 * @author Tobias Hilbrich
 * @date 03.08.2009
 */

#include <assert.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>
#include "CStratIsendDown.h"
#include "GtiMacros.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratIsendDown)
mFREE_INSTANCE_FUNCTION(CStratIsendDown)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratIsendDown)

//=============================
// buf_free_function
//=============================
GTI_RETURN buf_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratIsendDown
//=============================
CStratIsendDown::CStratIsendDown (const char* instanceName)
: ModuleBase<CStratIsendDown, CStratDownQueue> (instanceName)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Either uses a regular comm protocol or a regular and a
    //fall-back one.
    assert ((subModInstances.size() == 1) || (subModInstances.size() == 2));

    protocol = (I_CommProtocol*) subModInstances[0];

    //Set up internal data
    test_request = 0xFFFFFFFF;
    test_buff[0] = test_buff[1] = 0; /*Of no importance*/
}

//=============================
// ~CStratIsendDown
//=============================
CStratIsendDown::~CStratIsendDown (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;
}


//=============================
// shutdown
//=============================
GTI_RETURN CStratIsendDown::shutdown (
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
        uint64_t buf[2];

        for (i = 0; i < num; i++)
        {
            buf[0] = myTokenShutdownSync;
            buf[1] = 0;

            //Send the ping
            protocol->ssend(buf,sizeof(uint64_t)*2,i);
        }

        int pongCount = 0;
        while (pongCount < num)
        {
            uint64_t length;
            uint64_t channel;

            //receive the pong
            if (test_request != 0xFFFFFFFF)
            {
                protocol->wait_msg(test_request,&length,&channel);
                buf[0] = test_buff[0];
                buf[1] = test_buff[1];
                test_request = 0xFFFFFFFF;
            }
            else
            {
                protocol->recv(buf,sizeof(uint64_t)*2,&length,RECV_ANY_CHANNEL,&channel);
            }

            assert (length == sizeof(uint64_t)*2);
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
                protocol->recv(temp_buf,buf[1],&length,channel, NULL);
                delete [] temp_buf;
            }
            else
            {
                pongCount++;
            }
        }
    }

    // Remove anything that remains
    protocol->removeOutstandingRequests ();

    //Shutdown the protocols
    protocol->shutdown();
    return GTI_SUCCESS;
}

//=============================
// broadcast
//=============================
GTI_RETURN CStratIsendDown::broadcast (
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
// getPlaceId
//=============================
GTI_RETURN CStratIsendDown::getPlaceId (uint64_t* outPlaceId)
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
GTI_RETURN CStratIsendDown::getNumClients (uint64_t *outNumClients)
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
GTI_RETURN CStratIsendDown::test (
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

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==test
    if (test_request == 0xFFFFFFFF)
    {
        protocol->irecv(test_buff, sizeof(uint64_t)*2, &test_request, RECV_ANY_CHANNEL);
    }

    int completed;
    uint64_t numbytes, channel;

    protocol->test_msg(test_request,&completed,&numbytes,&channel);

    if (completed)
    {
        assert (numbytes == sizeof(uint64_t)*2); //has to be a token,length message
        assert (test_buff[0] == myTokenMessage); //has to be a message

        uint64_t* buf;
        uint64_t  buf_size, recv_size;

        if (test_buff[1] % sizeof(uint64_t) != 0)
            buf_size = (test_buff[1]/sizeof(uint64_t))+1;
        else
            buf_size = (test_buff[1]/sizeof(uint64_t));

        buf = new uint64_t[buf_size];

        protocol->recv(buf,test_buff[1],&recv_size,channel, NULL);

        assert (recv_size == test_buff[1]); //must be the size specified in first message

        //Save request as used up
        test_request = 0xFFFFFFFF;

        //save values
        if (out_flag)
                *out_flag = 1;
        if (out_num_bytes)
                *out_num_bytes = recv_size;
        if (out_buf)
                *out_buf = buf;
        if (out_free_data)
                *out_free_data = NULL;
        if (out_buf_free_function)
                *out_buf_free_function = buf_free_function;
        if (outChannel)
                *outChannel = channel;
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
GTI_RETURN CStratIsendDown::wait (
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

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==wait
    uint64_t numbytes, channel;

    if (test_request != 0xFFFFFFFF)
    {
        protocol->wait_msg(test_request,&numbytes,&channel);
    }
    else
    {
        protocol->recv(test_buff,sizeof(uint64_t)*2,&numbytes,RECV_ANY_CHANNEL,&channel);
    }

    assert (numbytes == sizeof(uint64_t)*2); //has to be a token,length message
    assert (test_buff[0] == myTokenMessage); //has to be a message

    uint64_t* buf;
    uint64_t  buf_size, recv_size;

    if (test_buff[1] % sizeof(uint64_t) != 0)
        buf_size = (test_buff[1]/sizeof(uint64_t))+1;
    else
        buf_size = (test_buff[1]/sizeof(uint64_t));

    buf = new uint64_t[buf_size];

    protocol->recv(buf,test_buff[1],&recv_size,channel, NULL);

    assert (recv_size == test_buff[1]); //must be the size specified in first message

    //Save request as used up
    test_request = 0xFFFFFFFF;

    //save values
    if (out_num_bytes)
            *out_num_bytes = recv_size;
    if (out_buf)
            *out_buf = buf;
    if (out_free_data)
            *out_free_data = NULL;
    if  (out_buf_free_function)
            *out_buf_free_function = buf_free_function;
    if (outChannel)
            *outChannel = channel;

    return GTI_SUCCESS;
}

//=============================
// acknowledge
//=============================
GTI_RETURN CStratIsendDown::acknowledge (
        uint64_t channel
        )
{
    /*Nothing to do*/
    return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratIsendDown::flush (
        void
        )
{

    //==Check whether there are queued sends, process if so
    if (hasQueueEntries())
        ProcessQueue ();

    //==flush
    //No action needed
    //->broadcast uses ssend

    return GTI_SUCCESS;
}

/*EOF*/
