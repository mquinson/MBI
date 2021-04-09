/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <pnmpi.h>
#include <pnmpimod.h>
#include "GtiMacros.h"
#include "GtiDefines.h"

#include <fstream>
#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include "CProtSMQueue.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(CProtSMQueue)
mFREE_INSTANCE_FUNCTION(CProtSMQueue)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CProtSMQueue)

bool CProtSMQueue::m_Terminate = false;
pthread_mutex_t CProtSMQueue::m_TIBLock;
std::vector<SMQueueTIB*> CProtSMQueue::m_TIBs;

pthread_key_t CProtSMQueue::m_KeyBuffer;
pthread_once_t CProtSMQueue::key_once = PTHREAD_ONCE_INIT;
unsigned int CProtSMQueue::lastRequestId = 0;
unsigned int CProtSMQueue::lastThreadId = 0;

void CProtSMQueue::make_key()
{
    pthread_key_create(&(CProtSMQueue::m_KeyBuffer), NULL);
    pthread_mutex_init(&m_TIBLock, NULL);
}

__attribute__((constructor))
void ModuleStartUp()
{
    pthread_once(&CProtSMQueue::key_once, CProtSMQueue::make_key);
}

/*__attribute__((constructor))
void CProtSMQueue::ModuleStartUp()
{
    pthread_once(&key_once, make_key);
}*/

CProtSMQueue::CProtSMQueue( const char* instanceName ) : ModuleBase<CProtSMQueue, I_CommProtocol>(instanceName) {
    //add the id to all sub modules
    char temp[64];
    sprintf (temp,"%" PRIu64, buildLayer());
    addDataToSubmodules ("id", temp);

    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    SMQueueTIB *tib = getTIB();

    //save sub modules
    //assert (subModInstances.size() >= 1);
}

CProtSMQueue::~CProtSMQueue() {
}

/**
* @see gti::I_CommProtocol::isConnected
*/
bool CProtSMQueue::isConnected (void)
{
    return isInitialized() && !isFinalized();
}

/**
* @see gti::I_CommProtocol::isInitialized
*/
bool CProtSMQueue::isInitialized (void)
{
    return true;
}

/**
* @see gti::I_CommProtocol::isFinalized
*/
bool CProtSMQueue::isFinalized (void)
{
    return m_Terminate;
}

/**
* @see gti::I_CommProtocol::getNumChannels
*/
GTI_RETURN CProtSMQueue::getNumChannels (uint64_t* out_numChannels)
{
    if( out_numChannels == NULL )
        return GTI_SUCCESS;

    // There are as many channels, as threads are registered up to now
    SMQueueTIB *c = getTIB();
    if( c->toolthread )
    {
        int numClients = 0;
        pthread_mutex_lock(&m_TIBLock);
        for( int i = 0; i < m_TIBs.size(); i++ )
            if( m_TIBs[i]->toolthread == false )
                numClients++;

        numClients = (numClients > 1) ? numClients : 1;
        *out_numChannels = 1;
        pthread_mutex_unlock(&m_TIBLock);
    }
    else // every thread only knows of one channel (the one to talk to the tool-thread)
    {
        *out_numChannels = 1;
    }
    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::getPlaceId
*/
GTI_RETURN CProtSMQueue::getPlaceId (uint64_t* outPlaceId)
{
    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::shutdown
*/
GTI_RETURN CProtSMQueue::shutdown (void)
{
    m_Terminate = true;
    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::removeOutstandingRequests
*/
GTI_RETURN CProtSMQueue::removeOutstandingRequests (void)
{
    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::ssend
*/
GTI_RETURN CProtSMQueue::ssend (
       void* buf,
       uint64_t num_bytes,
       uint64_t channel
       )
{
    unsigned int request;
    // Map it to isend + wait_msg
    isend(buf, num_bytes, &request, channel);
    wait_msg(request, NULL, NULL);
    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::isend
*/
GTI_RETURN CProtSMQueue::isend (
       void* buf,
       uint64_t num_bytes,
       unsigned int *out_request,
       uint64_t channel
       )
{
    SMQueueTIB *tib = getTIB();

    BYTE *position = NULL;

    // the request is stored on the thread that issued the send
    SMQueueRequest *req = pushRequest(channel, NULL, num_bytes, true);
    if( out_request != NULL )
        *out_request = req->requestid;

    // store data
    if( tib->toolthread == false )
    {
        tib->channel->getSendBuffer()->pushData((BYTE*)buf, num_bytes, req->requestid);
    }
    else
    {
        SMQueueTIB *apptib = getAppTIB(channel);
        assert( apptib );
        apptib->channel->getRecvBuffer()->pushData((BYTE*)buf, num_bytes, req->requestid);
    }

    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::recv
*/
GTI_RETURN CProtSMQueue::recv (
       void* out_buf,
       uint64_t num_bytes,
       uint64_t* out_length,
       uint64_t channel,
       uint64_t *out_channel
       )
{
    unsigned int request;
    // Map it to irecv + wait_msg
    irecv( out_buf, num_bytes, &request, channel );
    wait_msg( request, out_length, out_channel );

    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::irecv
*/
GTI_RETURN CProtSMQueue::irecv (
       void* out_buf,
       uint64_t num_bytes,
       unsigned int *out_request,
       uint64_t channel
       )
{
    SMQueueRequest *req = pushRequest(channel, out_buf, num_bytes, false);
    if ( out_request != NULL )
        *out_request = req->requestid;

    return GTI_SUCCESS;
}

/**
* @see gti::I_CommProtocol::test_msg
*/
GTI_RETURN CProtSMQueue::test_msg (
       unsigned int request,
       int* out_completed,
       uint64_t* out_receive_length,
       uint64_t* out_channel
       )
{
    if (out_completed != NULL)
        *out_completed = 0;

    SMQueueRequest *req = findRequestLocal(request, false);
    if ( req == NULL)
        return GTI_ERROR;

    // send requests are finished immediately
    if( req->send )
    {
        if( req->completed )
        {
            if( out_completed != NULL )
                *out_completed = 1;
            if( out_receive_length != NULL)
                *out_receive_length = req->size;
            if( out_channel != NULL )
                *out_channel = req->channel;

            findRequestLocal(request, true);
            delete req;
        }
        return GTI_SUCCESS;
    }

    return WaitMessageInternal(request, out_receive_length, out_channel, out_completed, true);
}

/**
* @see gti::I_CommProtocol::wait_msg
*/
GTI_RETURN CProtSMQueue::wait_msg (
       unsigned int request,
       uint64_t* out_receive_length,
       uint64_t* out_channel
       )
{
    SMQueueRequest *req = findRequestLocal(request, false);
    if ( req == NULL)
        return GTI_ERROR;

    // send requests are finished immediately
    if( req->send )
    {
        if( out_receive_length != NULL)
            *out_receive_length = req->size;
        if( out_channel != NULL )
            *out_channel = req->channel;

        while(!req->completed) { sched_yield(); }

        findRequestLocal(request, true);
        delete req;
        return GTI_SUCCESS;
    }
    return WaitMessageInternal(request, out_receive_length, out_channel, NULL, false);
}

GTI_RETURN CProtSMQueue::WaitMessageInternal(
       unsigned int request,
       uint64_t* out_receive_length,
       uint64_t* out_channel,
       int* out_completed,
       bool test_only
)
{
    if (out_completed != NULL)
        *out_completed = 0;

    SMQueueTIB *tib = getTIB();

    // Iterate through it's send requests to see if we find one for us
    SMQueueRequest *req = findRequestLocal(request, false);
    while(req == NULL)
    {
        // Try again
        req = findRequestLocal(request, false);
#if 0
        // This will break at wait_msg if there is a thread
        // looking for a nonexistant request.
        assert( req != NULL );
#endif
        sched_yield();
    }

    // If we are waiting for a receive request we
    // move the data from the send/receive buffer here
    // We have individual RecvBuffers for each thread
    if(req->send == false)
    {
        GTI_RETURN retVal = RecvInternal(req, out_receive_length, out_channel, out_completed, test_only);
        return retVal;
    }
    else // Send requests
    {
        assert(0);
    }
    return GTI_SUCCESS;
}

GTI_RETURN CProtSMQueue::RecvInternal(SMQueueRequest *req,
       uint64_t* out_receive_length,
       uint64_t* out_channel,
       int* out_completed,
       bool test_only)
{
    assert(!req->send);
    SMQueueTIB *tib = getTIB();

    if( req->channel == RECV_ANY_CHANNEL )
    {
        if( tib->toolthread )
        {
            bool gotMessage = false;
            do
            {
                // We must search for the first TIB which has something
                // in it's send buffer
                pthread_mutex_lock(&m_TIBLock);
                unsigned int iChannel = 0;
                for( std::vector<SMQueueTIB*>::iterator it = m_TIBs.begin(); it != m_TIBs.end(); ++it )
                {
                    if( !((*it)->toolthread) )
                    {
                        if (!((*it)->channel->getSendBuffer()->IsEmpty()) )
                        {
                            gotMessage = true;
                            req->channel = iChannel;
                            break;
                        }
                        iChannel++;
                    }
                }
                pthread_mutex_unlock(&m_TIBLock);
            }
            while(!test_only && !gotMessage);

            if(test_only && !gotMessage)
            {
                if( out_channel != NULL )
                    *out_channel = req->channel;

                if( out_receive_length != NULL )
                    *out_receive_length = -1;
                return GTI_SUCCESS;
            }
        }
        else
            req->channel = 0;
    }

    BIOBuffer *buffer = NULL;
    if( tib->toolthread == false )
    {
        buffer = tib->channel->getRecvBuffer();
    }
    else
    {
        SMQueueTIB *apptib = getAppTIB(req->channel);
        buffer = apptib->channel->getSendBuffer();
    }

    // If we selected a buffer, do the receiving here
    if( buffer != NULL )
    {
        unsigned int recvlen = 0;
        unsigned int remote_req;
        for(;;)
        {
            recvlen = buffer->readData(req->ptr, req->size, &remote_req);
            // If we come here when testing for messages
            // we may exit immediately after the first try fails!
            if( recvlen <= 0 )
            {
                if( test_only )
                    return GTI_SUCCESS; // there is no message here!
                sched_yield(); // yield..
            }
            else
            {
                break; // if we received something, exit here!
            }
        }

        if (out_completed != NULL)
            *out_completed = 1;

        if( out_channel != NULL )
            *out_channel = req->channel;

        if( out_receive_length != NULL )
            *out_receive_length = recvlen;

        // Finally remove the request. if we come here the message was received
        signalRemoteRequest(req->channel, remote_req);
        findRequestLocal(req->requestid, true);
        delete req;
    }
    return GTI_SUCCESS;
}

SMQueueRequest *CProtSMQueue::signalRemoteRequest( int channel, unsigned int req )
{
    SMQueueRequest *retval = NULL;
    SMQueueTIB *tib = getTIB();
    assert(tib != NULL);
    SMQueueTIB *rtib;
    if(tib->toolthread)
    {
        rtib = getAppTIB(channel);
    }
    else
    {
        rtib = getToolTIB();
    }
    assert(rtib!=NULL);
    pthread_mutex_lock(&(rtib->m_RequestLock));
    std::vector<SMQueueRequest*>::iterator it;
    bool set = false;
    for( it = (rtib->m_Requests.begin()); it != (rtib->m_Requests.end()); ++it )
    {
        if((*it)->send == false)
            continue;

        if ((*it)->requestid == req )
        {
            (*it)->completed = true;
            set = true;
            break;
        }
    }
    pthread_mutex_unlock(&(rtib->m_RequestLock));
    //assert(set==true);
    return retval;
}

SMQueueRequest *CProtSMQueue::findRequestLocal( unsigned int request, bool remove )
{
    SMQueueRequest *retval = NULL;
    SMQueueTIB *tib = getTIB();
    assert( tib != NULL );
    pthread_mutex_lock(&(tib->m_RequestLock));
    if( tib->m_Requests.size() )
    {
        std::vector<SMQueueRequest*>::iterator it;
        for( it = (tib->m_Requests.begin()); it != (tib->m_Requests.end()); ++it )
        {
            if ((*it)->requestid == request )
            {
                retval = *it;
                break;
            }
        }
        // Only remove if we actually found something
        if( retval != NULL && remove )
        {
            tib->m_Requests.erase(it);
        }
    }
    pthread_mutex_unlock(&(tib->m_RequestLock));
    return retval;
}

unsigned int CProtSMQueue::getNextRequestId()
{
    return __sync_fetch_and_add(&lastRequestId,1);
}

SMQueueTIB *CProtSMQueue::getTIB()
{
    void *tls = pthread_getspecific(m_KeyBuffer);
    if( tls == NULL )
    {
        printf("Creating new Thread Info Block and Channel!\n");

        pthread_mutex_lock(&m_TIBLock);
        ThreadChannel *chan = new ThreadChannel;
        SMQueueTIB *TIB = new SMQueueTIB;

        {
            // Put this thread on layer 2 for now
            char buf[] = "level_1";
            PNMPI_modHandle_t stack;
            int err=PNMPI_Service_GetStackByName(buf,&stack);
            assert (err==PNMPI_SUCCESS);
            // If we are level 1 let's assume we are the tool thread
            PNMPI_modHandle_t h;
            PNMPI_Service_GetModuleSelf(&h);
            if(h == stack)
                TIB->toolthread = true;
        }

        TIB->thread_id = lastThreadId++;
        TIB->channel = chan;
        m_TIBs.push_back( TIB );
        pthread_setspecific(m_KeyBuffer, (void*)TIB);
        pthread_mutex_unlock(&m_TIBLock);

        return TIB;
    }
    return (SMQueueTIB*)tls;
}

SMQueueTIB *CProtSMQueue::getToolTIB()
{
    pthread_mutex_lock(&m_TIBLock);
    SMQueueTIB *tib = NULL;
    for( std::vector<SMQueueTIB*>::iterator it = m_TIBs.begin(); it != m_TIBs.end(); ++it )
        if( (*it)->toolthread )
        {
            tib = *it;
            break;
        }
    pthread_mutex_unlock(&m_TIBLock);

    return tib;
}

SMQueueTIB *CProtSMQueue::getAppTIB( int channel )
{
     SMQueueTIB *tib = NULL;
     while(tib == NULL) {
        pthread_mutex_lock(&m_TIBLock);
        unsigned int iChannel = 0;
        for( std::vector<SMQueueTIB*>::iterator it = m_TIBs.begin(); it != m_TIBs.end(); ++it )
        {
            if( !((*it)->toolthread) )
            {
                if( iChannel == channel )
                {
                    tib = *it;
                    break;
                }
                iChannel++;
            }
        }
        pthread_mutex_unlock(&m_TIBLock);
        if(tib == NULL)
            sched_yield();
    }
    return tib;
}

bool CProtSMQueue::freeTIB()
{
    SMQueueTIB *TIB = (SMQueueTIB*)pthread_getspecific(m_KeyBuffer);
    if( TIB != NULL )
    {
        printf("Deleting Thread Info Block and Channel!\n");

        pthread_mutex_lock(&m_TIBLock);
        for( std::vector<SMQueueTIB*>::iterator it = m_TIBs.begin(); it != m_TIBs.end(); ++it )
        {
            SMQueueTIB *ptib = *it;
            if( ptib->thread_id == TIB->thread_id )
            {
                m_TIBs.erase(it);
                pthread_setspecific(m_KeyBuffer, NULL);
                pthread_mutex_unlock(&m_TIBLock);
                // Free the channel!
                delete ptib->channel;
                delete ptib;
                return true;
                break;
            }
        }
        pthread_mutex_unlock(&m_TIBLock);
        return false;
    }
    return false;
}

SMQueueRequest *CProtSMQueue::pushRequest(
        unsigned int channel,
        void* buf_pos,
        uint64_t num_bytes,
        bool send
    )
{
    SMQueueTIB *tib = getTIB();

    // create a new request in this threads request queue
    unsigned int id = getNextRequestId();
    SMQueueRequest *req = new SMQueueRequest();
    req->requestid = id;
    req->ptr = (BYTE*)buf_pos;
    req->size = num_bytes;
    req->channel = channel;
    req->send = send;

    pthread_mutex_lock(&(tib->m_RequestLock));
    tib->m_Requests.push_back(req);
    pthread_mutex_unlock(&(tib->m_RequestLock));

    return req;
}


