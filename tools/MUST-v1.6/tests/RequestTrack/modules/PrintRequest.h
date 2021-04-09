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
 * @file PrintRequest.h
 *       @see MUST::PrintRequest.
 *
 *  @date 04.02.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_PrintRequest.h"
#include "I_CreateMessage.h"

#ifndef PRINTREQUEST_H
#define PRINTREQUEST_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_PrintRequest.
     */
    class PrintRequest : public gti::ModuleBase<PrintRequest, I_PrintRequest>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintRequest (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintRequest (void);

    		/**
    		 * @see I_PrintRequest::print.
    		 */
    		GTI_ANALYSIS_RETURN print (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    protected:
    		I_CreateMessage* myLogger;
    		I_RequestTrack* myRTracker;
    		I_LocationAnalysis* myLocations;
    }; /*class PrintRequest */
} /*namespace MUST*/

#endif /*PRINTREQUEST_H*/
