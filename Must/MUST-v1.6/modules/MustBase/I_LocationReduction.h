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
 * @file I_LocationReduction.h
 *       @see I_LocationReduction
 *
 *  @date 11.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "I_Reduction.h"
#include "I_ChannelId.h"
#include "GtiEnums.h"
#include "mustConfig.h"

#include "BaseIds.h"

#include <list>

#ifndef I_LOCATIONREDUCTION_H
#define I_LOCATIONREDUCTION_H

/**
 * Interface for a reduction that removes
 * unnecessary location records.
 * @see MUST::LocationImpl
 */
class I_LocationReduction : public gti::I_Module, public gti::I_Reduction
{
public:
	/**
	 * Performs the reduction.
	 * @see I_LocationAnalysis::registerLocation
	 * @see gti::I_Reduction
	 */
    virtual gti::GTI_ANALYSIS_RETURN reduce (
    		MustParallelId pId,
    		MustLocationId lId,
    		char* callName,
    		int callNameLen,
#ifdef USE_CALLPATH
    		int numStackLevels,
    		int stackInfosLength,
    		int indicesLength,
    		int* infoIndices,
    		char* stackInfos,
#endif
    		gti::I_ChannelId *thisChannel,
    		std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

}; /*class I_LocationReduction*/

#endif /*I_LOCATIONREDUCTION_H*/
