/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file I_PrintRequest.h
 *       @see I_PrintRequest.
 *
 *  @date 04.02.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h" //TODO Needs to be renamed to GTI enums
#include "BaseIds.h"
#include "I_RequestTrack.h"
#include "I_LocationAnalysis.h"

#ifndef I_PRINTREQUEST_H
#define I_PRINTREQUEST_H

/**
 * Interface for a module that prints all available
 * information on a request, using I_RequestTrack.
 *
 * Dependencies:
 *  1) LocationAnalysis
 *  2) CreateMessage
 *  3) RequestTrack
 */
class I_PrintRequest : public gti::I_Module
{
public:
	/**
	 * Prints information on the given request as log events.
	 * @param pId parallel Id of the print call.
	 * @param lId location Id of the print call.
	 * @param request request to print.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN print (
    		MustParallelId pId,
    		MustLocationId lId,
    		MustRequestType request) = 0;

}; /*class I_PrintRequest*/

#endif /*I_PRINTREQUEST_H*/
