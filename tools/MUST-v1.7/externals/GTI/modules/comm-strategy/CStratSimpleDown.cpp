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
 * @file CStratSimpleDown.cpp
 *       Simple communication strategy.
 *
 * Sends all messages immediately and blocking.
 */

#include <assert.h>
#include <stdio.h>
#include <pnmpimod.h>
#include "CStratSimpleDown.h"
#include "GtiMacros.h"
#include <stdlib.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CStratSimpleDown)
mFREE_INSTANCE_FUNCTION(CStratSimpleDown)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CStratSimpleDown)

const uint64_t CStratSimpleDown::myTokenShutdownSync = 0xFFFFFFFF;
const uint64_t CStratSimpleDown::myTokenMessage = 0xFFFFFFFE;
const uint64_t CStratSimpleDown::myTokenAck = 0xFFFFFFFD;

//=============================
// buf_free_function
//=============================
GTI_RETURN buf_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratSimpleDown
//=============================
CStratSimpleDown::CStratSimpleDown (const char* instanceName)
: ModuleBase<CStratSimpleDown, CStratDownQueue> (instanceName),
  myBcastReqs ()
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
// ~CStratSimpleDown
//=============================
CStratSimpleDown::~CStratSimpleDown (void)
{
    /*Destroy sub modules*/
    if (protocol)
        destroySubModuleInstance ((I_Module*)protocol);
    protocol = NULL;
}

//=============================
// shutdown
//=============================
GTI_RETURN CStratSimpleDown::shutdown (
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

    //==Flush
    if (flush_behavior == GTI_FLUSH)
        flush ();

    //Finish pending broadcasts
    completeBcastRequests (true);

    // Remove protocol requests, to avoid sync msgs beeing catched by
    // outstanding requests !
    protocol->removeOutstandingRequests ();

    if (sync_behavior == GTI_SYNC)
    {
        //Do the sync
        uint64_t num,i;
        protocol->getNumClients(&num);
        for (i = 0; i < num; i++)
        {
            uint64_t buf[2] = {myTokenShutdownSync,0};
            uint64_t length;

            //Send the ping
            protocol->ssend(buf,sizeof(uint64_t)*2,i);

            //receive the pong
            do
            {
                protocol->recv(buf,sizeof(uint64_t)*2,&length,i, NULL);
                assert (length == sizeof(uint64_t)*2);
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
                    protocol->recv(temp_buf,buf[1],&length,i, NULL);
                    delete [] temp_buf;
                }
            } while (buf[0] != myTokenShutdownSync);
        }
    }

    //Shutdown the protocols
    protocol->shutdown();

    return GTI_SUCCESS;
}

//=============================
// broadcast
//=============================
GTI_RETURN CStratSimpleDown::broadcast (
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
    SimpleDownRequestInfo msgInfo;
    msgInfo.request = 0; /*not used*/
    msgInfo.buf_free_function = buf_free_function;
    msgInfo.buf = buf;
    msgInfo.free_data = free_data;
    msgInfo.num_bytes = num_bytes;

    uint64_t num,i;
    unsigned int request1, request2;
    protocol->getNumChannels(&num);
    for (i = 0; i < num; i++)
    {
        uint64_t *temp_buf = new uint64_t[2];
        temp_buf[0] = myTokenMessage;
        temp_buf[1] = num_bytes;
        protocol->isend(temp_buf,sizeof(uint64_t)*2, &request1, i);
        protocol->isend(buf,num_bytes, &request2,i);

        SimpleDownRequestInfo reqInfo1;
        reqInfo1.request = request1;
        reqInfo1.buf = temp_buf;
        reqInfo1.buf_free_function = NULL;
        msgInfo.requests.push_back (request2);


        myBcastReqs.push_back (reqInfo1);
    }
    myBcastReqs.push_back (msgInfo);

    //==Test for completion of some requests
    completeBcastRequests (false);

    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CStratSimpleDown::getPlaceId (uint64_t* outPlaceId)
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
GTI_RETURN CStratSimpleDown::getNumClients (uint64_t *outNumClients)
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
GTI_RETURN CStratSimpleDown::test (
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

#if 0
        if (test_buff[1] % sizeof(uint64_t) != 0)
            buf_size = (test_buff[1]/sizeof(uint64_t))+1;
        else
            buf_size = (test_buff[1]/sizeof(uint64_t));
#else
        buf_size = ((test_buff[1] - 1) / sizeof(uint64_t)) + 1;
#endif

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
GTI_RETURN CStratSimpleDown::wait (
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

#if 0
    if (test_buff[1] % sizeof(uint64_t) != 0)
        buf_size = (test_buff[1]/sizeof(uint64_t))+1;
    else
        buf_size = (test_buff[1]/sizeof(uint64_t));
#else
    buf_size = ((test_buff[1] - 1) / sizeof(uint64_t)) + 1;
#endif

    buf = new uint64_t[buf_size];

    protocol->recv(buf,test_buff[1],&recv_size,channel, NULL);

    assert (recv_size == test_buff[1]); //must be the size specified in first message

    //Save request as used up
    test_request = 0xFFFFFFFF;

    //save values
    if (out_num_bytes)
    		*out_num_bytes =	 recv_size;
    if (out_buf)
    		*out_buf = buf;
    if (out_free_data)
    		*out_free_data = NULL;
    if (out_buf_free_function)
    		*out_buf_free_function = buf_free_function;
    if (outChannel)
    		*outChannel = channel;
    return GTI_SUCCESS;
}

//=============================
// acknowledge
//=============================
GTI_RETURN CStratSimpleDown::acknowledge (
		uint64_t channel
        )
{
	if (!protocol->isConnected())
	{
	    	return GTI_ERROR_NOT_INITIALIZED;
	}

	uint64_t *temp_buf = new uint64_t[2];
	temp_buf[0] = myTokenAck;
	temp_buf[1] = 0;
	unsigned int request;
	protocol->isend(temp_buf,sizeof(uint64_t)*2,&request,channel);

	SimpleDownRequestInfo info;
	info.request = request;
	info.buf = temp_buf;
	info.buf_free_function = NULL;

	myBcastReqs.push_back(info);

	completeBcastRequests (false);
	return GTI_SUCCESS;
}

//=============================
// flush
//=============================
GTI_RETURN CStratSimpleDown::flush (
        void
        )
{

    //Check whether there are queued sends, process if so
    if (hasQueueEntries())
    	ProcessQueue ();

    //flush
    //No action needed
    //->broadcast uses ssend

    return GTI_SUCCESS;
}

//=============================
// completeBcastRequests
//=============================
void CStratSimpleDown::completeBcastRequests (bool block)
{
    int completed = false;

    while (!myBcastReqs.empty())
    {
        //What request (single one or one from a list?)
        unsigned int req = myBcastReqs.front().request;

        if (!myBcastReqs.front().requests.empty())
            req = myBcastReqs.front().requests.front();

        //Complete it (or try to)
        if (block)
        {
            protocol->wait_msg(req, NULL, NULL);
            completed = true;
        }
        else
        {
            protocol->test_msg(req, &completed, NULL, NULL);
        }

        //If not completed abort this loop
        if (!completed)
            break;

        //If we have multiple requests store that the first one is finished
        if (!myBcastReqs.front().requests.empty())
            myBcastReqs.front().requests.pop_front();

        //Check wether this was the last request in the info, if so kill it
        if (myBcastReqs.front().requests.empty())
        {
            if (myBcastReqs.front().buf_free_function)
                (*(myBcastReqs.front().buf_free_function)) (myBcastReqs.front().free_data, myBcastReqs.front().num_bytes, myBcastReqs.front().buf);
            else if (myBcastReqs.front().buf)
                delete [] (uint64_t*) myBcastReqs.front().buf;

            myBcastReqs.pop_front();
        }
    }
}

/*EOF*/
