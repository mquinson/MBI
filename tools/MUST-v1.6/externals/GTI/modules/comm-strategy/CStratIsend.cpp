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

#include "CStratIsend.h"
#include <cstdlib>

using namespace gti;

#define MAX_NUM_REQUESTS_DEFAULT 100

const uint64_t CStratIsend::myTokenShutdownSync = 0xFFFFFFFF;
const uint64_t CStratIsend::myTokenMessage = 0xFFFFFFFE;

//=============================
// CStratIsendRequest
//=============================
CStratIsendRequest::CStratIsendRequest (void)
	: my_buf (NULL), my_num_bytes (0), my_free_data (NULL), my_buf_free_function(NULL), my_request (0)
{
	//Nothing to do
}

//=============================
// CStratIsendRequest
//=============================
CStratIsendRequest::CStratIsendRequest (void* buf, uint64_t num_bytes, void* free_data , GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf), unsigned int request)
	: my_buf (buf), my_num_bytes (num_bytes), my_free_data (free_data), my_buf_free_function (buf_free_function), my_request (request)
{
	//Nothing to do
}

//=============================
// ~CStratIsendRequest
//=============================
CStratIsendRequest::~CStratIsendRequest (void)
{
	//Nothing to do
}

//=============================
// get_request
//=============================
unsigned int CStratIsendRequest::get_request (void)
{
	return my_request;
}

//=============================
// free_buffer
//=============================
void CStratIsendRequest::free_buffer (void)
{
	/*Free the buffer*/
	if (my_buf_free_function)
		my_buf_free_function (my_free_data, my_num_bytes, my_buf);
	my_buf_free_function = NULL;
}

//=============================
// CStratIsend
//=============================
CStratIsend::CStratIsend ()
	: myRequests (), myMaxNumRequests (MAX_NUM_REQUESTS_DEFAULT)
{
    //Nothing to do
}

//=============================
// ~CStratIsend
//=============================
CStratIsend::~CStratIsend ()
{
	//Nothing to do
}

/*EOF*/
