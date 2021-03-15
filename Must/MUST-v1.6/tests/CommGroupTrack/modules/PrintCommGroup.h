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
 * @file PrintCommGroup.h
 *       @see MUST::PrintCommGroup.
 *
 *  @date 06.03.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_PrintCommGroup.h"
#include "I_CreateMessage.h"

#ifndef PRINTCOMMGROUP_H
#define PRINTCOMMGROUP_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_PrintCommGroup.
     */
    class PrintCommGroup : public gti::ModuleBase<PrintCommGroup, I_PrintCommGroup>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintCommGroup (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintCommGroup (void);

    		/**
    		 * @see I_PrintCommGroup::printComm.
    		 */
    		GTI_ANALYSIS_RETURN printComm (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm);

    		/**
    		 * @see I_PrintCommGroup::printGroup.
    		 */
    		GTI_ANALYSIS_RETURN printGroup (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType group);

    protected:
    		I_LocationAnalysis* myLocations;
    		I_CreateMessage* myLogger;
    		I_CommTrack* myCTracker;
    		I_GroupTrack* myGTracker;

    		/**
    		 * Adds information for the mapping of the given group to the
    		 * stream.
    		 *
    		 * @param group to print.
    		 * @param stream to print to.
    		 */
    		void addGroupInfoToStream (I_GroupTable* group, std::stringstream &stream);
    }; /*class PrintCommGroup */
} /*namespace MUST*/

#endif /*PRINTCOMMGROUP_H*/
