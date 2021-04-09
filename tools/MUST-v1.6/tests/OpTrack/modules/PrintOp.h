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
 * @file PrintOp.h
 *       @see MUST::PrintOp.
 *
 *  @date 10.05.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_CreateMessage.h"

#include "I_PrintOp.h"

#ifndef PRINTOP_H
#define PRINTOP_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_PrintOp.
     */
    class PrintOp : public gti::ModuleBase<PrintOp, I_PrintOp>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintOp (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintOp (void);

    		/**
    		 * @see I_PrintOp::print.
    		 */
    		GTI_ANALYSIS_RETURN print (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustOpType op);

    protected:
    		I_CreateMessage* myLogger;
    		I_OpTrack* myOpTracker;
    		I_LocationAnalysis* myLocations;
    }; /*class PrintOp */
} /*namespace MUST*/

#endif /*PRINTOP_H*/
