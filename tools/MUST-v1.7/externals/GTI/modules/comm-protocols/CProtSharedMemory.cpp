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
 * @file CProtSharedMemory.cpp
 *       A shared memory implementation of the communication protocol interface.
 *
 *  This implementation may be used to communicate between processes that share
 *  memory.
 *
 * @author Joachim Protze
 * @date 15.03.2012
 *
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>
#include <algorithm>
#include <utility>

#include "CProtSharedMemory.h"
#include "GtiMacros.h"


#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>


using namespace gti;

mGET_INSTANCE_FUNCTION(CommProtSharedMemory)
mFREE_INSTANCE_FUNCTION(CommProtSharedMemory)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommProtSharedMemory)

SMQueue * CommProtSharedMemory::helloQueue;
SMSyncPoint CommProtSharedMemory::entrySyncPoint;
SMSyncPoint CommProtSharedMemory::exitSyncPoint;


void CommProtSharedMemory::initModule()
{
    helloQueue = new SMQueue();
}

struct hello_msg{
  SMQueue* inQueue;
  SMQueue* outQueue;
};


void __attribute__ ((noinline)) CommProtSharedMemory::connect()
{
  assert(!myIsIntra);
  if (isTop)
  {
    maxNumPartners = remoteTierSize / tierSize;
    numPartners=0;

  }
  else
  {
    numPartners=1;
    hello_msg hm;

    hm.inQueue=new SMQueue();
    hm.outQueue=new SMQueue();

    outQueues.push_back(hm.outQueue);
    hm.outQueue->outChannel = 0;

    inQueues.push_back(hm.inQueue);
    hm.inQueue->inChannel = 0;


    recvQueues.push_back(new std::queue<SMRequest*>());

    SMRequest* msg = new SMRequest(&hm, sizeof(hm), 0);
    {
      std::unique_lock<std::mutex> mLock(msg->SMRMutex);
      helloQueue->push(msg);
      while(!msg->finished){
        assert(mLock.owns_lock());
        msg->SMRCondVar.wait(mLock);
      }
    }
    delete msg;
    recv(&myPlaceId, sizeof(myPlaceId), NULL, 0, NULL);
    myPlaceId += (hm.inQueue->outChannel+1) << 32;
  }
}

void CommProtSharedMemory::reconnect()
{
  while (isTop && !helloQueue->empty())
  {
    SMRequest* msg = helloQueue->try_pop();
    if (msg==nullptr)
      return;
    hello_msg* hm = (hello_msg*) msg->data;
    outQueues.push_back(hm->inQueue);
    inQueues.push_back(hm->outQueue);
    recvQueues.push_back(new std::queue<SMRequest*>());
    hm->outQueue->inChannel = numPartners;
    hm->inQueue->outChannel = numPartners;
    numPartners++;
    // notify the place about the new client
    newClientCallback();
    assert(numPartners < maxNumPartners);
    {
      std::unique_lock<std::mutex> lock(msg->SMRMutex);
      msg->finished=true;
      msg->SMRCondVar.notify_one();
    }
    ssend(&myPlaceId, sizeof(myPlaceId), numPartners-1);
  }
}


//=============================
// CommProtSharedMemory
//=============================
CommProtSharedMemory::CommProtSharedMemory (const char* instanceName)
: ModuleBase<CommProtSharedMemory, I_CommProtocol> (instanceName),
requestId(1), myIsIntra (false), isTop(true)
{

    char string[128];
    static std::once_flag key_once;
    std::call_once(key_once, CommProtSharedMemory::initModule);

    SMRequest::dp = new DataPool<SMRequest, 8>();

    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A Comm Protocol likely needs no sub modules
    assert (subModInstances.empty());

    // === (1) Read data ===
    //General protocol data
    std::map<std::string, std::string> data = getData ();
    std::map<std::string, std::string>::iterator i;

    i=data.find("comm_id");
    assert (i != data.end ());
    commId = atoi (i->second.c_str());

    i=data.find("is_intra");
    if (i != data.end())
    {
        if (i->second.c_str()[0] != '0' && i->second.c_str()[0] != '1')
        {
            std::cerr << "Error: Invalid specification for \"is_intra\" module data field in " << __FILE__ << ":" << __LINE__ << std::endl;
            assert (0);
        }

        if (i->second.c_str()[0] == '1')
            myIsIntra = true;
    }
    i=data.find("side");
    assert (myIsIntra || i != data.end ());
    assert (myIsIntra || i->second.length() == 1); //just one letter !
    if(!myIsIntra)
    {
        commSide = i->second.c_str()[0];
        assert ((commSide == 't') || (commSide == 'b')); //either "t" or "b"
        if (commSide == 't')
            isTop = true;
        else
            isTop = false;
    }

    i=data.find("tier_size");
    assert (i != data.end ());
    tierSize = atol (i->second.c_str());

    i=data.find("target_tier_size");
    assert (myIsIntra || i != data.end ());
    if(!myIsIntra)
        remoteTierSize = atol (i->second.c_str());

    i=data.find("id");
    assert (i != data.end ());
    myPlaceId = atol (i->second.c_str());

    i=data.find("gti_own_level");
    assert (i != data.end ());
    gtiOwnLevel = atol (i->second.c_str());

    connect();
    initialized=true;
    finalized=false;
}

//=============================
// ~CommProtSharedMemory
//=============================
CommProtSharedMemory::~CommProtSharedMemory (void)
{
    //TODO
}

//=============================
// isConnected
//=============================
bool CommProtSharedMemory::isConnected (void)
{
	return isInitialized() && !isFinalized();
}

//=============================
// isInitialized
//=============================
bool CommProtSharedMemory::isInitialized (void)
{
	return initialized;
}

//=============================
// isFinalized
//=============================
bool CommProtSharedMemory::isFinalized (void)
{
	return finalized;
}

//=============================
// shutdown
//=============================
GTI_RETURN CommProtSharedMemory::shutdown (void)
{
//    for (int i =0; i< numPartners; i++)
    finalized = true;
    return GTI_SUCCESS;
}

//=============================
// getNumChannels
//=============================
GTI_RETURN CommProtSharedMemory::getNumChannels (
        uint64_t* out_numChannels)
{
    reconnect();
    *out_numChannels = numPartners;
    return GTI_SUCCESS;
}

//=============================
// getNumClients
//=============================
GTI_RETURN CommProtSharedMemory::getNumClients (
        uint64_t* out_numClients)
{
    reconnect();
    *out_numClients = 1;
    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CommProtSharedMemory::getPlaceId (uint64_t* outPlaceId)
{
    if (!initialized)
            return GTI_ERROR_NOT_INITIALIZED;

    if (outPlaceId)
            *outPlaceId = myPlaceId;
    return GTI_SUCCESS;
}

//=============================
// removeOutstandingRequests
//=============================
GTI_RETURN CommProtSharedMemory::removeOutstandingRequests (void)
{
    entrySyncPoint.visit();


 /*   for (std::map<int, SMRequest*>::iterator i = requestMap.begin(); i != requestMap.end();)
    {
      if(! i->second->send){
        delete i->second;
        requestMap.erase(i++);
      }else{
        i++;
      }
    }*/
    
    for (auto i : requestMap){
      if(! i.second->send)
        delete i.second;
    }
    requestMap.clear();

    for (auto i : outQueues)
      while(auto req = i->try_pop())
        delete req;

    exitSyncPoint.visit();

    return GTI_SUCCESS;
}

// int pipe_send(pipe_t p, const char* buf, int bytes, long source);

//=============================
// ssend
//=============================
GTI_RETURN CommProtSharedMemory::ssend (
        void* buf,
        uint64_t num_bytes,
        uint64_t channel
        )
{
    reconnect();
    SMQueue* queue = outQueues[channel];
    SMRequest* msg = new SMRequest(buf, num_bytes, 0);
    {
      std::unique_lock<std::mutex> mLock(msg->SMRMutex);
      queue->push(msg);
      while(!msg->finished)
        msg->SMRCondVar.wait(mLock);
      TsanHappensAfter(&(msg->finished));
    }
    delete msg;
    return GTI_SUCCESS;
}

//=============================
// isend
//=============================
GTI_RETURN CommProtSharedMemory::isend (
        void* buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
    reconnect();
    SMQueue* queue = outQueues[channel];
    SMRequest* msg = new SMRequest(buf, num_bytes, requestId++);
    requestMap[msg->request] = msg;
    queue->push(msg);
    if (out_request)
      *out_request = msg->request;
    return GTI_SUCCESS;
}

uint64_t handle_recv(SMRequest* msg,
        void* out_buf,
        uint64_t num_bytes)
{
  assert(num_bytes >= msg->size);
  std::unique_lock<std::mutex> mLock(msg->SMRMutex);
  memmove(out_buf, msg->data, (size_t) msg->size);
  TsanHappensBefore(&(msg->finished));
  msg->finished = true;
  if (msg->request == 0)
    msg->SMRCondVar.notify_one();
  return msg->size;
}

uint64_t handle_recv(
        SMRequest* send,
        SMRequest* recv)
{
  recv->finished = true;
  return recv->inSize = handle_recv(send, recv->data, recv->size);
}



//=============================
// recv
//=============================
GTI_RETURN CommProtSharedMemory::recv (
        void* out_buf,
        uint64_t num_bytes,
        uint64_t* out_length,
        uint64_t channel,
        uint64_t* out_channel
        )
{
    reconnect();
    while (channel==RECV_ANY_CHANNEL)
    {
      for (;next_channel<numPartners;next_channel++)
      {
        if (!inQueues[next_channel]->empty())
        {
          channel = next_channel;
          break;
        }
      }
      next_channel=0;
    }
    SMRequest* msg = inQueues[channel]->wait_pop();
    if (out_length)
      *out_length = msg->size;
    if (out_channel)
      *out_channel = channel;
    handle_recv(msg, out_buf, num_bytes);
    return GTI_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_RETURN CommProtSharedMemory::irecv (
        void* out_buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
  reconnect();
  SMRequest* msg = new SMRequest(out_buf, num_bytes, channel, requestId++);
  requestMap[msg->request] = msg;
  if (channel != RECV_ANY_CHANNEL)
    recvQueues[channel]->push(msg);
  if (out_request)
    *out_request = msg->request;
  return GTI_SUCCESS;
}


/*
 * match queued receives with arrived messages
 * until we could match our request
 */

bool finishReceives(
        SMRequest* req,
        std::queue<SMRequest*>* requests,
        SMQueue* inQueue,
        bool test)
{
  SMRequest* inmsg, *recv;
  if (req->finished)
    return true;
  assert(req->channel == inQueue->inChannel);
  while(!requests->empty()){
    if(test)
      inmsg = inQueue->try_pop();
    else
      inmsg = inQueue->wait_pop();
    if (inmsg == nullptr)
      return false;
    recv = requests->front();
    requests->pop();
    recv->inSize = handle_recv(inmsg, recv);
    if (recv->request == req->request)
      return req->finished;
  }
  return false;
}


void CommProtSharedMemory::handle_test(
        unsigned int request,
        int* out_completed,
        uint64_t* out_receive_length,
        uint64_t* out_channel,
        bool test)
{
    reconnect();
    if (out_completed)
        *out_completed = false;
    std::map<int, SMRequest*>::iterator iter = requestMap.find(request);
    assert(iter != requestMap.end());
    SMRequest * req = iter->second;

    if (!req->finished)
    {
      if(req->send){
        assert(req->channel != RECV_ANY_CHANNEL);
        if(!test)
        {
          std::unique_lock<std::mutex> mLock(req->SMRMutex);
          req->request = 0;
          while(!req->finished)
            req->SMRCondVar.wait(mLock);
        }
      } else {
        if (req->channel == RECV_ANY_CHANNEL)
        {
          if(test){
            int i=next_channel;
            for (;next_channel<numPartners;next_channel++)
            {
              if (!inQueues[next_channel]->empty())
              {
                req->channel = next_channel;
                break;
              }
            }
            for (next_channel=0; next_channel<i && req->channel!=RECV_ANY_CHANNEL; next_channel++)
            {
              if (!inQueues[next_channel]->empty())
              {
                req->channel = next_channel;
                break;
              }
            }
          } else {
            while (req->channel==RECV_ANY_CHANNEL)
            {
              for (;next_channel<numPartners;next_channel++)
              {
                if (!inQueues[next_channel]->empty())
                {
                  req->channel = next_channel;
                  break;
                }
              }
              next_channel=0;
            }
          }
          if (req->channel != RECV_ANY_CHANNEL) {
            SMRequest* inmsg;
            assert(req->channel == inQueues[req->channel]->inChannel);
            inmsg = inQueues[req->channel]->wait_pop();
            req->inSize = handle_recv(inmsg, req);
          }
        } else {
          finishReceives(req, recvQueues[req->channel], inQueues[req->channel], test);
        }
      }
    }
    assert (test || req->finished);
    TsanHappensAfter(&(req->finished));
    if (req->finished)
    {
      if (out_completed != NULL)
          *out_completed = true;
      if (out_receive_length != NULL)
          *out_receive_length = req->inSize;
      if (out_channel != NULL)
          *out_channel = req->channel;
      delete req;
      requestMap.erase(iter);
    }
}



//=============================
// test_msg
//=============================
GTI_RETURN CommProtSharedMemory::test_msg (
        unsigned int request,
        int* out_completed,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
  handle_test(request, out_completed, out_receive_length, out_channel, true);
   return GTI_SUCCESS;
}

//=============================
// wait_msg
//=============================
GTI_RETURN CommProtSharedMemory::wait_msg (
        unsigned int request,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
  handle_test(request, nullptr, out_receive_length, out_channel, false);
  return GTI_SUCCESS;
}

/*EOF*/
