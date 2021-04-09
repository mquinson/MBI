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
 * @file CStratQueue.cxx
 *        Queue for sends and broadcasts of communication strategies that
 *        can't be handled immediately, e.g. due to an uninitialized comm
 *        protocol.
 *
 * @author Tobias Hilbrich
 * @date 13.10.2010
 */

#include "CStratQueue.h"

using namespace gti;

//=============================
// CStratQueue
//=============================
CStratQueue::CStratQueue (void)
	: myQueue ()
{

}

//=============================
// ~CStratQueue
//=============================
CStratQueue::~CStratQueue (void)
{
	/*
	 * Clean up any remaining queue items.
	 * In a normal situation there should be none.
	 * We try to free the buffers to transfer here,
	 * this might be somewhat dangerous though.
	 */
	std::list <CStratQueueItem>::iterator i;
	for (i = myQueue.begin(); i != myQueue.end(); i++)
	{
		CStratQueueItem item = *i;
		if (item.buf_free_function)
			item.buf_free_function (item.free_data, item.num_bytes, item.buf);
	}
}

//=============================
// AddToQueue
//=============================
GTI_RETURN CStratQueue::AddToQueue (
		void* buf,
		uint64_t num_bytes,
		void* free_data,
		GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
		uint64_t toChannel
		)
{
	CStratQueueItem item;

	item.toChannel = toChannel;
	item.buf = buf;
	item.buf_free_function = buf_free_function;
	item.free_data = free_data;
	item.num_bytes = num_bytes;

	myQueue.push_back (item);

	return GTI_SUCCESS;
}

//=============================
// hasQueueEntries
//=============================
bool CStratQueue::hasQueueEntries (void)
{
	return !myQueue.empty();
}

//=============================
// CStratUpQueue
//=============================
CStratUpQueue::CStratUpQueue (void)
	: CStratQueue ()
{
	/*nothing to do*/
}

//=============================
// ~CStratUpQueue
//=============================
CStratUpQueue::~CStratUpQueue (void)
{
	/*nothing to do*/
}

//=============================
// ProcessQueue
//=============================
GTI_RETURN CStratUpQueue::ProcessQueue (void)
{
	// Loop over all queue items and call the
	// actual send function for each
	//
	// Create a copy first!
	// Rational:
	//     If something goes wrong the queue
	//     items may be jumping back into the
	//     queue immediately, thus creating a
	//     spin lock.
    // swap eliminates excessive copying
    std::list <CStratQueueItem> copy;
    std::swap (myQueue, copy);

	std::list <CStratQueueItem>::iterator i;
	for (i = copy.begin(); i != copy.end(); i++)
	{
		CStratQueueItem item = *i;
		send (item.buf, item.num_bytes, item.free_data, item.buf_free_function);
	}

	return GTI_SUCCESS;
}

//=============================
// CStratDownQueue
//=============================
CStratDownQueue::CStratDownQueue (void)
	: CStratQueue ()
{
	/*nothing to do*/
}

//=============================
// ~CStratDownQueue
//=============================
CStratDownQueue::~CStratDownQueue (void)
{
	/*nothing to do*/
}

//=============================
// ProcessQueue
//=============================
GTI_RETURN CStratDownQueue::ProcessQueue (void)
{
	// Loop over all queue items and call the
	// actual send function for each
	//
	// Create a copy first!
	// Rational:
	//     If something goes wrong the queue
	//     items may be jumping back into the
	//     queue immediately, thus creating a
	//     spin lock.
	// swap eliminates excessive copying
	std::list <CStratQueueItem> copy;
	std::swap (myQueue, copy);

	std::list <CStratQueueItem>::iterator i;
	for (i = copy.begin(); i != copy.end(); i++)
	{
		CStratQueueItem item = *i;
		broadcast (item.buf, item.num_bytes, item.free_data, item.buf_free_function);
	}

	return GTI_SUCCESS;
}

//=============================
// CStratIntraQueue
//=============================
CStratIntraQueue::CStratIntraQueue (void)
    : CStratQueue ()
{
    /*nothing to do*/
}

//=============================
// ~CStratIntraQueue
//=============================
CStratIntraQueue::~CStratIntraQueue (void)
{
    /*nothing to do*/
}

//=============================
// ProcessQueue
//=============================
GTI_RETURN CStratIntraQueue::ProcessQueue (void)
{
    // Loop over all queue items and call the
    // actual send function for each
    //
    // Create a copy first!
    // Rational:
    //     If something goes wrong the queue
    //     items may be jumping back into the
    //     queue immediately, thus creating a
    //     spin lock.
    std::list <CStratQueueItem> copy = myQueue;
    myQueue.clear ();

    std::list <CStratQueueItem>::iterator i;
    for (i = copy.begin(); i != copy.end(); i++)
    {
        CStratQueueItem item = *i;
        send (item.toChannel, item.buf, item.num_bytes, item.free_data, item.buf_free_function);
    }

    return GTI_SUCCESS;
}

/*EOF*/
