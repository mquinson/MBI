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
 * @file CStratPRecv.cpp
 *        @see gti::CStratPRecv.
 *
 * Based on CStratThreaded, this is a modified copy!
 *
 * @author Tobias Hilbrich
 * @date 13.03.2013
 */

#include "CStratPRecv.h"

#include <assert.h>
#include <string.h>

using namespace gti;

uint64_t CStratPRecv::BUF_LENGTH 			    = 10*1024; //10KiByte
/**
 * One would think MAX_NUM_MSGS should be like 100 or 1000, but NO!
 * 2 is very good, along with the MPI_Issend fix (mod 50) in CProtMPISplited.
 *
 * I could reproduce this now with MVAPICH, MVAPICH2, OpenMPI.
 */
uint64_t CStratPRecv::MAX_NUM_MSGS 			    = 2;
const uint64_t CStratPRecv::myTokenShutdownSync = 0xFFFFFFFF;
const uint64_t CStratPRecv::myTokenMessage 		= 0xFFFFFFFE;
const uint64_t CStratPRecv::myTokenLongMsg 		= 0xFFFFFFFD;

//=============================
// returnedBufBufFreeFunction
//=============================
GTI_RETURN gti::returnedBufBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf)
{
    CStratPRecvBufInfo* aggre_info = (CStratPRecvBufInfo*)free_data;
    aggre_info->instance->notifyOfLastUserFinishedEmptyBuf(aggre_info);
    return GTI_SUCCESS;
}

//=============================
// longMsgBufFreeFunction
//=============================
GTI_RETURN gti::longMsgBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] (uint64_t*) buf;
    return GTI_SUCCESS;
}

//=============================
// CStratPRecv
//=============================
CStratPRecv::CStratPRecv ()
{
	//Nothing to do
}

//=============================
// ~CStratPRecv
//=============================
CStratPRecv::~CStratPRecv ()
{
	//Nothing to do
}

//=============================
// CStratPRecvBufInfo
//=============================
CStratPRecvBufInfo::CStratPRecvBufInfo(uint64_t buf_size)
{
    buf = new char[buf_size]();
    channel = 0;
    instance=NULL;
}

CStratPRecvBufInfo::CStratPRecvBufInfo(char* buf)
 : buf (buf)
{
    channel = 0;
    instance=NULL;
}

//=============================
// ~CStratPRecvBufInfo
//=============================
CStratPRecvBufInfo::~CStratPRecvBufInfo()
{
    delete [] buf;
}

//=============================
// CStratPRecvSender
//=============================
CStratPRecvSender::CStratPRecvSender (I_CommProtocol **managedProtocol)
 : myRequests(),
   myManagedProtocol (managedProtocol)
{
    myMaxNumReqs = 0;
}

//=============================
// ~CStratPRecvSender
//=============================
CStratPRecvSender::~CStratPRecvSender (void)
{

}

//=============================
// completeOutstandingSendRequest
//=============================
void CStratPRecvSender::completeOutstandingSendRequest (bool useMyRequests, CStratIsendRequest request)
{
    if (useMyRequests)
    {
        CStratIsendRequest req = myRequests.front();

        assert (*myManagedProtocol != NULL);
        (*myManagedProtocol)->wait_msg(req.my_request,NULL,NULL);

        myRequests.pop_front();
        req.free_buffer();
    }
    else
    {
        assert (*myManagedProtocol != NULL);
        (*myManagedProtocol)->wait_msg(request.my_request,NULL,NULL);
        request.free_buffer();
    }
}

//=============================
// send_long_message
//=============================
GTI_RETURN CStratPRecvSender::send_long_message (
        uint64_t toPlace,
        void* buf,
        uint64_t num_bytes,
        void* free_data,
        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    uint64_t *temp_buf = new uint64_t[2];
    temp_buf[0] = myTokenLongMsg;
    temp_buf[1] = num_bytes;

    /**
     * @todo if we use CProtSharedMemory, we need to check whether both messeges went out
     */

    sendCommBuf (temp_buf, false, 2*sizeof(uint64_t), toPlace, NULL, longMsgBufFreeFunction);
    sendCommBuf (buf, false, num_bytes, toPlace, free_data, buf_free_function);

    return GTI_SUCCESS;
}

//=============================
// sendCommBuf
//=============================
void CStratPRecvSender::sendCommBuf (
        void *buf,
        bool sendSynchronizing,
        uint64_t len,
        uint64_t channel,
        void* free_data ,
        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf))
{
    int completed=myRequests.size();

    if (completed > myMaxNumReqs)
        myMaxNumReqs = completed;

    while (completed)
    {
        CStratIsendRequest req = myRequests.front();

        if (completed < MAX_NUM_MSGS)
        {
            assert (*myManagedProtocol != NULL);
            (*myManagedProtocol)->test_msg(req.my_request,&completed,NULL,NULL);

            if (completed)
            {
                myRequests.pop_front();
                completed = myRequests.size();
                req.free_buffer();
            }
        }
        else
        {
            //Too many requests enforce completion of some request
            completeOutstandingSendRequest(true,CStratIsendRequest());
            completed=1;
        }
    }

    if (buf)
    {
        if (!sendSynchronizing)
        {
            unsigned int request;
            assert (*myManagedProtocol != NULL);
            (*myManagedProtocol)->isend(buf, len, &request, channel);
            myRequests.push_back (CStratIsendRequest (buf, len, free_data, buf_free_function, request));
        }
        else
        {
            assert (*myManagedProtocol != NULL);
            (*myManagedProtocol)->ssend(buf, len, channel);
            if (buf_free_function)
                (*buf_free_function) (free_data, len, buf);
        }
    }
}

//=============================
// CStratBufReceiver::TestInfo::TestInfo
//=============================
CStratBufReceiver::TestInfo::TestInfo ()
 : request (0),
   buf (NULL)
{
}

//=============================
// CStratBufReceiver
//=============================
CStratBufReceiver::CStratBufReceiver (I_CommProtocol **managedProtocol, uint64_t bufSize)
 : myFreeBufs(),
   myTestBuf(NULL),
   myTests (),
   myNumNonWcTests (0),
   myManagedProtocol (managedProtocol),
   myBufSize (bufSize)
{
    myTestRequest = 0xFFFFFFFF;
}

//=============================
// ~CStratBufReceiver
//=============================
CStratBufReceiver::~CStratBufReceiver (void)
{
    while (!myFreeBufs.empty())
    {
        CStratPRecvBufInfo* a = myFreeBufs.front();
        delete (a);
        myFreeBufs.pop_front();
    }

    if (myTestBuf)
        delete (myTestBuf);
}

//=============================
// get_free_buf
//=============================
CStratPRecvBufInfo* CStratBufReceiver::get_free_buf (void)
{
    if (myFreeBufs.empty())
    {
        //create a new buffer
        CStratPRecvBufInfo* ret = new CStratPRecvBufInfo (myBufSize);
        ret->instance = this;
        ret->channel = 0;
        return ret;
    }

    CStratPRecvBufInfo* ret = myFreeBufs.front();
    myFreeBufs.pop_front();
    return ret;
}

//=============================
// long_msg_from_info
//=============================
GTI_RETURN CStratBufReceiver::long_msg_from_info (
        uint64_t in_length,
        uint64_t channel,
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
        uint64_t *outChannel
        )
{
    //Received info for a long message
    uint64_t* buf;
    uint64_t  buf_size, recv_size;

    buf_size = ((in_length+sizeof(uint64_t)-1)/sizeof(uint64_t));

    buf = new uint64_t[buf_size]();

    assert (*myManagedProtocol != NULL);
    (*myManagedProtocol)->recv(buf, in_length ,&recv_size, channel, NULL);

    assert (recv_size == in_length); //must be the size specified in first message

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
        *out_buf_free_function = longMsgBufFreeFunction;
    if (outChannel)
        *outChannel = channel;

    return GTI_SUCCESS;
}

//=============================
// notifyOfLastUserFinishedEmptyBuf
//=============================
void CStratBufReceiver::notifyOfLastUserFinishedEmptyBuf (CStratPRecvBufInfo *info)
{
    info->channel = 0;
    info->instance = this;
    myFreeBufs.push_back (info);
}

/*EOF*/
