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
 * @file I_PrintCommGroup.h
 *       @see I_PrintCommGroup.
 *
 *  @date 06.03.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h" //TODO Needs to be renamed to GTI enums
#include "BaseIds.h"
#include "I_CommTrack.h"
#include "I_GroupTrack.h"
#include "I_LocationAnalysis.h"

#ifndef I_PRINTCOMMGROUP_H
#define I_PRINTCOMMGROUP_H

/**
 * Interface for a module that prints all available
 * information on a communicator or group,
 * using I_CommTrack and I_GroupTrack.
 *
 * Dependencies:
 *  1) LocationAnalysis
 *  2) CreateMessage
 *  3) CommTrack
 *  4) GroupTrack
 */
class I_PrintCommGroup : public gti::I_Module
{
public:
	/**
	 * Prints information on the given communicator as log events.
	 * @param pId parallel Id of the print call.
	 * @param lId location Id of the print call.
	 * @param comm to print.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN printComm (
    		MustParallelId pId,
    		MustLocationId lId,
    		MustCommType request) = 0;

    /**
     * Prints information on the given group as log events.
     * @param pId parallel Id of the print call.
     * @param lId location Id of the print call.
     * @param group to print.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN printGroup (
    		MustParallelId pId,
    		MustLocationId lId,
    		MustGroupType group) = 0;

}; /*class I_PrintCommGroup*/

#endif /*I_PRINTCOMMGROUP_H*/
