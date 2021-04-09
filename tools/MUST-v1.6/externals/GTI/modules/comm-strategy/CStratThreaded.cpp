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
 * @file CStratThreaded.cpp
 *        Application phase aware communication strategy.
 *
 * Provides the interface functionality with an implementation
 * that tries to avoid any comm. during phases in which the
 * application is communicating.
 *
 * @author Tobias Hilbrich
 * @date 25.08.2009
 */

#include "CStratThreaded.h"

#include <assert.h>
#include <string.h>

using namespace gti;

uint64_t CStratThreaded::BUF_LENGTH 			    = 100 *1024; //100KB
uint64_t CStratThreaded::MAX_NUM_MSGS 			    = 1000;
const uint64_t CStratThreaded::myTokenShutdownSync = 0xFFFFFFFF;
const uint64_t CStratThreaded::myTokenMessage 		= 0xFFFFFFFE;
const uint64_t CStratThreaded::myTokenLongMsg 		= 0xFFFFFFFD;

//=============================
// returnedAggregateBufFreeFunction
//=============================
GTI_RETURN gti::returnedAggregateBufFreeFunction (void* free_data, uint64_t num_bytes, void* buf)
{
    CStratAggregateInfo* aggre_info = (CStratAggregateInfo*)free_data;
    assert (aggre_info->num_in_use); //must be 1 or higher!
    aggre_info->num_in_use--;

    if (aggre_info->num_in_use == 0 && aggre_info->num_msgs_left == 0)
    {
        //aggregate completely processed!
        aggre_info->instance->notifyOfLastUserFinishedEmptyAggregate(aggre_info);
    }

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
// CStratThreaded
//=============================
CStratThreaded::CStratThreaded ()
{
	//Nothing to do
}

//=============================
// ~CStratThreaded
//=============================
CStratThreaded::~CStratThreaded ()
{
	//Nothing to do
}

//=============================
// CStratAggregateInfo
//=============================
CStratAggregateInfo::CStratAggregateInfo(uint64_t buf_size)
{
    buf = new char[buf_size]();
    // pattern not found by gdb
    // (gdb) b CStratThreadedDown.cpp:322
    // 322                             ((uint64_t*)successfulAggregate->buf)[0] == myTokenLongMsg    ); //has to be a message
    // (gdb) python buff=gdb.parse_and_eval("((unsigned char*)successfulAggregate->buf)"); print [i for i in range(gdb.parse_and_eval("numbytes")) if buff[i]==0x96]
//    memset(buf,0x69,buf_size);
    current_position = 0;
    num_msgs_left = 0;
    num_in_use = 0;
    channel = 0;
    instance=NULL;
}

CStratAggregateInfo::CStratAggregateInfo(char* buf)
 : buf (buf)
{
    current_position = 0;
    num_msgs_left = 0;
    num_in_use = 0;
    channel = 0;
    instance=NULL;
}

//=============================
// ~CStratAggregateInfo
//=============================
CStratAggregateInfo::~CStratAggregateInfo()
{
    delete [] buf;
}

//=============================
// CStratThreadedAggregator
//=============================
CStratThreadedAggregator::CStratThreadedAggregator (I_CommProtocol **managedProtocol)
 : myRequests(),
   myFreeBufs(),
   myCommBufs(),
   myCurAggregateBufs(),
   myCurrAggregateLens (),
   myManagedProtocol (managedProtocol)
{
    myMaxNumReqs = 0;
}

//=============================
// ~CStratThreadedAggregator
//=============================
CStratThreadedAggregator::~CStratThreadedAggregator (void)
{
    for (int i = 0; i < myCurAggregateBufs.size(); i++)
    {
        if (myCurAggregateBufs[i])
            delete [] myCurAggregateBufs[i];
    }
    myCurAggregateBufs.clear();

    for (int i = 0; i < myCommBufs.size(); i++)
    {
        if (myCommBufs[i])
            delete [] myCommBufs[i];
    }
    myCommBufs.clear();

    while (!myFreeBufs.empty())
    {
        delete [] myFreeBufs.front();
        myFreeBufs.pop_front();
    }
}

//=============================
// swap
//=============================
void CStratThreadedAggregator::swap (uint64_t channel)
{
    /*
     * - wait till curr comm buffer is received (if necessary)
     * - switch buffers
     * - prepare the new aggregate buf (set len = 0)
     */
    assert (!myCurAggregateBufs.empty()); //Must be initialized beforehand

    myCommBufs[channel] = myCurAggregateBufs[channel];
    myCurAggregateBufs[channel] = NULL;

    int completed=myRequests.size();

    if (completed > myMaxNumReqs)
        myMaxNumReqs = completed;

    while (completed)
    {
        AggRequestInfo req = myRequests.front();

        if (completed < MAX_NUM_MSGS)
        {
            assert (*myManagedProtocol != NULL);
            (*myManagedProtocol)->test_msg(req.request,&completed,NULL,NULL);

            if (completed)
            {
                if (!myCurAggregateBufs[channel])
                    myCurAggregateBufs[channel] = req.buf;
                else
                    myFreeBufs.push_back(req.buf);

                myRequests.pop_front();

                completed = myRequests.size();
            }
        }
        else
        {
            //Too many requests enforce completion of some request
            completeOutstandingSendRequest(true,0);
            completed=1;
        }
    }

    if (!myCurAggregateBufs[channel])
    {
        if (!myFreeBufs.empty())
        {
            myCurAggregateBufs[channel] = myFreeBufs.front();
            myFreeBufs.pop_front();
        }
        else
        {
            myCurAggregateBufs[channel] = new char [BUF_LENGTH]();
        }
    }

    prepareAggregateBuffer(channel);
}

//=============================
// completeOutstandingSendRequest
//=============================
void CStratThreadedAggregator::completeOutstandingSendRequest (bool useMyRequests, unsigned int request)
{
    if (useMyRequests)
    {
        AggRequestInfo req = myRequests.front();

        assert (*myManagedProtocol != NULL);
        (*myManagedProtocol)->wait_msg(req.request,NULL,NULL);

        myFreeBufs.push_back(req.buf);
        myRequests.pop_front();
    }
    else
    {
        assert (*myManagedProtocol != NULL);
        (*myManagedProtocol)->wait_msg(request,NULL,NULL);
    }
}

//=============================
// prepareAggregateBuffer
//=============================
void CStratThreadedAggregator::prepareAggregateBuffer (uint64_t channel)
{
    assert (!myCurAggregateBufs.empty()); //Must be initialized beforehand

    //Prepare the aggregate buffer if it is still fresh
    myCurrAggregateLens[channel] = 2*sizeof (uint64_t);
    ((uint64_t*)myCurAggregateBufs[channel])[0] = myTokenMessage;
    ((uint64_t*)myCurAggregateBufs[channel])[1] = 0; //curr num messages
}

//=============================
// send_long_message
//=============================
GTI_RETURN CStratThreadedAggregator::send_long_message (
        uint64_t toPlace,
        void* buf,
        uint64_t num_bytes,
        void* free_data,
        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)
        )
{
    /*
     * - send old aggregate (swap and isend)
     * - finish sending current aggregate
     * - send ping (overlong message token + length)
     * - send the long message as one buffer (ssend)
     * - wait till sending is finished
     */
    assert (!myCurAggregateBufs.empty()); //Must be initialized beforehand

    uint64_t old_len = myCurrAggregateLens[toPlace];
    if (old_len > 2*sizeof(uint64_t))
    {
        swap(toPlace); //finish sending old buffer and swap buffers
        /**
         * @todo see CStratThreadedUp and its comments on sendCommBuf (where it uses it the first time)
         */
        sendCommBuf (false, BUF_LENGTH, toPlace);
    }

    uint64_t temp_buf[2];
    unsigned int req1, req2;
    temp_buf[0] = myTokenLongMsg;
    temp_buf[1] = num_bytes;
    assert (*myManagedProtocol != NULL);
    (*myManagedProtocol)->isend(temp_buf, 2*sizeof(uint64_t), &req1,toPlace);

    assert (*myManagedProtocol != NULL);
    (*myManagedProtocol)->isend(buf, num_bytes, &req2, toPlace);

    //free given buffer
    buf_free_function (buf, num_bytes, free_data);

    completeOutstandingSendRequest (false, req1);
    completeOutstandingSendRequest (false, req2);;

    return GTI_SUCCESS;
}

//=============================
// sendCommBuf
//=============================
void CStratThreadedAggregator::sendCommBuf (bool sendSynchronizing, uint64_t len, uint64_t channel)
{
    assert (!myCurAggregateBufs.empty()); //Must be initialized beforehand

    if (!sendSynchronizing)
    {
        unsigned int request;
        assert (*myManagedProtocol != NULL);
        (*myManagedProtocol)->isend(myCommBufs[channel], len, &request, channel);
        myRequests.push_back (AggRequestInfo(myCommBufs[channel],request));
    }
    else
    {
        assert (*myManagedProtocol != NULL);
        (*myManagedProtocol)->ssend(myCommBufs[channel], len, channel);
        myFreeBufs.push_back(myCommBufs[channel]);
    }

    myCommBufs[channel] = NULL;
}

//=============================
// CStratAggregateReceiver::TestInfo::TestInfo
//=============================
CStratAggregateReceiver::TestInfo::TestInfo ()
 : request (0),
   aggregate (NULL)
{
}

//=============================
// CStratAggregateReceiver
//=============================
CStratAggregateReceiver::CStratAggregateReceiver (I_CommProtocol **managedProtocol, uint64_t aggregateSize)
 : myFreeAggregates(),
   myOpenAggregate(NULL),
//    myAggregatesInUse(),
   myTestAggregate(NULL),
   myTests (),
   myNumNonWcTests (0),
   myManagedProtocol (managedProtocol),
   myAggregateSize (aggregateSize)
{
    myTestRequest = 0xFFFFFFFF;
}

//=============================
// ~CStratAggregateReceiver
//=============================
CStratAggregateReceiver::~CStratAggregateReceiver (void)
{
//     std::map<CStratAggregateInfo*, int>::iterator iter;
//     for (iter = myAggregatesInUse.begin(); iter != myAggregatesInUse.end (); iter++)
//         delete (iter->first);
//     myAggregatesInUse.clear();

    while (!myFreeAggregates.empty())
    {
        CStratAggregateInfo* a = myFreeAggregates.front();
        delete (a);
        myFreeAggregates.pop_front();
    }

    if (myOpenAggregate)
        delete (myOpenAggregate);

    if (myTestAggregate)
        delete (myTestAggregate);
}

//=============================
// get_free_aggregate
//=============================
CStratAggregateInfo* CStratAggregateReceiver::get_free_aggregate (void)
{
    if (myFreeAggregates.empty())
    {
        //create a new buffer
        return new CStratAggregateInfo (myAggregateSize);
    }

    CStratAggregateInfo* ret = myFreeAggregates.front();
    myFreeAggregates.pop_front();
    return ret;
}

//=============================
// msg_from_open_aggregate
//=============================
GTI_RETURN CStratAggregateReceiver::msg_from_open_aggregate (
        int* out_flag,
        uint64_t* out_num_bytes,
        void** out_buf,
        void** out_free_data,
        GTI_RETURN (**out_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
        uint64_t *outChannel
        )
{
    uint64_t msg_length = ((uint64_t*)myOpenAggregate->buf)[myOpenAggregate->current_position/sizeof(uint64_t)];
    void *buffer_start = myOpenAggregate->buf + myOpenAggregate->current_position + sizeof(uint64_t);
    void *free_data =  myOpenAggregate;

    if (out_flag)
        *out_flag = 1;
    if (out_num_bytes)
        *out_num_bytes = msg_length;
    if (out_buf)
        *out_buf = buffer_start;
    if (out_free_data)
        *out_free_data = free_data;
    if (out_buf_free_function)
        *out_buf_free_function = returnedAggregateBufFreeFunction;
    if (outChannel)
        *outChannel = myOpenAggregate->channel;

    myOpenAggregate->num_in_use++;
    myOpenAggregate->num_msgs_left--;

    if (myOpenAggregate->num_msgs_left == 0)
    {
        //last message from aggregate -> add it to used
//         myAggregatesInUse.insert (std::make_pair(myOpenAggregate,0));
        myOpenAggregate = NULL;
    }
    else
    {
        //set position to next message in aggregate
        myOpenAggregate->current_position +=  sizeof(uint64_t) + msg_length;

        if (myOpenAggregate->current_position % sizeof(uint64_t))
            myOpenAggregate->current_position += sizeof(uint64_t) - myOpenAggregate->current_position % sizeof(uint64_t);
    }

    //DEBUG
    //std::cout << "RECV: [" << *out_num_bytes << "]" << "id=" << ((uint64_t*) *out_buf)[0] << std::endl;
    //ENDDEBUG

    return GTI_SUCCESS;
}

//=============================
// long_msg_from_info
//=============================
GTI_RETURN CStratAggregateReceiver::long_msg_from_info (
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

//     if (in_length % sizeof(uint64_t) != 0)
//         buf_size = (in_length/sizeof(uint64_t))+1;
//     else
//         buf_size = (in_length/sizeof(uint64_t));
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

    //DEBUG
    //std::cout << "LONG MSG RECV: [" << *out_num_bytes << "]" << "id=" << ((uint64_t*) *out_buf)[0] << std::endl;
    //ENDDEBUG

    return GTI_SUCCESS;
}

//=============================
// notifyOfLastUserFinishedEmptyAggregate
//=============================
void CStratAggregateReceiver::notifyOfLastUserFinishedEmptyAggregate (CStratAggregateInfo *info)
{
    info->channel = 0;
    info->current_position = 0;
    info->instance = this;
    info->num_in_use = 0;
    info->num_msgs_left = 0;
//     myAggregatesInUse.erase(info);
    myFreeAggregates.push_back (info);
}

/*EOF*/
