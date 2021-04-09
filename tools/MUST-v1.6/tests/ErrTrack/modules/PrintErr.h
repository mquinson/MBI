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
 * @file PrintErr.h
 *       @see MUST::PrintErr.
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_CreateMessage.h"

#include "I_PrintErr.h"

#ifndef PRINTERR_H
#define PRINTERR_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_PrintErr.
     */
    class PrintErr : public gti::ModuleBase<PrintErr, I_PrintErr>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintErr (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintErr (void);

    		/**
    		 * @see I_PrintErr::print.
    		 */
    		GTI_ANALYSIS_RETURN print (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustErrType err);

    protected:
    		I_CreateMessage* myLogger;
    		I_ErrTrack* myErrTracker;
    		I_LocationAnalysis* myLocations;
    }; /*class PrintErr */
} /*namespace MUST*/

#endif /*PRINTERR_H*/
