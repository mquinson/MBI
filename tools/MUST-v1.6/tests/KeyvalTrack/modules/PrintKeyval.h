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
 * @file PrintKeyval.h
 *       @see MUST::PrintKeyval.
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_CreateMessage.h"

#include "I_PrintKeyval.h"

#ifndef PRINTKEYVAL_H
#define PRINTKEYVAL_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_PrintKeyval.
     */
    class PrintKeyval : public gti::ModuleBase<PrintKeyval, I_PrintKeyval>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintKeyval (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintKeyval (void);

    		/**
    		 * @see I_PrintKeyval::print.
    		 */
    		GTI_ANALYSIS_RETURN print (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustKeyvalType keyval);

    protected:
    		I_CreateMessage* myLogger;
    		I_KeyvalTrack* myKeyvalTracker;
    		I_LocationAnalysis* myLocations;
    }; /*class PrintKeyval */
} /*namespace MUST*/

#endif /*PRINTKEYVAL_H*/
