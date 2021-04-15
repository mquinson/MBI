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
 * @file CStratQueue.h
 *        Queue for sends and broadcasts of communication strategies that
 *        can't be handled immediately, e.g. due to an uninitialized comm
 *        protocol.
 *
 * @author Tobias Hilbrich
 * @date 13.10.2010
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_CommStrategyDown.h"
#include "I_CommStrategyUp.h"
#include "I_CommStrategyIntra.h"

#include <list>

#ifndef CSTRAT_QUEUE_H
#define CSTRAT_QUEUE_H

namespace gti
{
	/**
	 * Information on a single queued send or broadcast.
	 * @see gti::I_CommStrategyUp::send
	 * @see gti::I_CommStrategyDown::broadcast
	 */
	struct CStratQueueItem
	{
	    uint64_t toChannel; //Channel to send to, only relevant for intra communication
		void* buf;
		uint64_t num_bytes;
		void* free_data;
		GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf);
	};

	/**
	 * Queue of delayed sends or broadcasts.
	 * Used if the communication protocol of a communication strategy
	 * is not initialized before sends or broadcasts are issued to it.
	 */
	class CStratQueue
	{
	protected:
		std::list <CStratQueueItem> myQueue; /**< Queue used to store sends/broadcasts.*/

	public:
		/**
		 * Proper constructor.
		 */
		CStratQueue (void);

		/**
		 * Destructor.
		 */
		virtual ~CStratQueue (void);

		/**
		 * Adds information on a new send/broadcast to the queue.
		 * @see gti::I_CommStrategyUp::send
		 * @see gti::I_CommStrategyDown::broadcast
		 * @return GTI_SUCCESS if successful.
		 */
		GTI_RETURN AddToQueue (
                void* buf,
                uint64_t num_bytes,
                void* free_data,
                GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),
                uint64_t toChannel = 0
                );

		/**
		 * Returns true if this queue is non-empty
		 * and false otherwise.
		 * @return true iff non-empty.
		 */
		bool hasQueueEntries (void);

		/**
		 * Processes all items in the queue to send them.
		 * @return GTI_SUCCESS if successful.
		 */
		virtual GTI_RETURN ProcessQueue (void) = 0;
	};

	/**
	 * Queue for an upwards directed communication strategy.
	 * @see gti::CStratQueue
	 */
	class CStratUpQueue : public CStratQueue, public I_CommStrategyUp
	{
	public:

		/**
		 * Constructor.
		 */
		CStratUpQueue (void);

		/**
		 * Destructor.
		 */
		virtual ~CStratUpQueue (void);

		/**
		 * Processes all items in the queue to send them.
		 * @return GTI_SUCCESS if successful.
		 */
		GTI_RETURN ProcessQueue (void);
	};

	/**
	 * Queue for a downwards directed communication strategy.
	 * @see gti::CStratQueue
	 */
	class CStratDownQueue : public CStratQueue, public I_CommStrategyDown
	{
	public:
		/**
		 * Constructor.
		 */
		CStratDownQueue (void);

		/**
		 * Destructor.
		 */
		virtual ~CStratDownQueue (void);

		/**
		 * Processes all items in the queue to send them.
		 * @return GTI_SUCCESS if successful.
		 */
		GTI_RETURN ProcessQueue (void);
	};

	/**
	 * Queue for a intra communication strategy.
	 * @see gti::CStratQueue
	 */
	class CStratIntraQueue : public CStratQueue, public I_CommStrategyIntra
	{
	public:
	    /**
	     * Constructor.
	     */
	    CStratIntraQueue (void);

	    /**
	     * Destructor.
	     */
	    virtual ~CStratIntraQueue (void);

	    /**
	     * Processes all items in the queue to send them.
	     * @return GTI_SUCCESS if successful.
	     */
	    GTI_RETURN ProcessQueue (void);
	};
} //namespace gti

#endif /*CSTRAT_QUEUE_H*/
