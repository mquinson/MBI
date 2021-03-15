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
 * @file CStratIsendUp.h
 *        Non-blocking send based implementation for the I_CommStrategyUp interface.
 *
 * Basis functionality for both up and down.
 *
 * @author Tobias Hilbrich
 * @data 03.08.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include <list>

#ifndef CSTRAT_ISEND_H
#define CSTRAT_ISEND_H

namespace gti
{
	/**
	 * Class used for outstanding send requests.
	 */
	class CStratIsendRequest
	{
	protected:
		void* 			my_buf;
		uint64_t 	my_num_bytes;
		void* 			my_free_data;
		GTI_RETURN 	(*my_buf_free_function) (void* free_data, uint64_t num_bytes, void* buf);

	public:
		unsigned int 	my_request;

		/**
		 * Default constructor.
		 */
		CStratIsendRequest (void);

		/**
		 * Constructor with values.
		 */
		CStratIsendRequest (void* buf, uint64_t num_bytes, void* free_data , GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf), unsigned int request);

		/**
		 * Destructor.
		 */
		~CStratIsendRequest (void);

		/**
		 * Returns this request.
		 * @return request
		 */
		unsigned int get_request (void);

		/**
		 * Frees the buffer with the function provided to the constructor.
		 */
		void free_buffer (void);
	};

	/**
	 * Base class for the non-blocking send based communication
	 * strategy.
	 */
	class CStratIsend
	{
	protected:
		std::list<CStratIsendRequest> myRequests;
		unsigned int 				  myMaxNumRequests;

		static const uint64_t 	  myTokenShutdownSync;
		static const uint64_t 	  myTokenMessage;

	public:
		CStratIsend ();
		virtual ~CStratIsend ();
	};
}

#endif /*CSTRAT_ISEND_H*/
