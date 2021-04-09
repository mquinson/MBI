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
 * @file LocationReduction.h
 *       @see LocationReduction
 *
 *  @date 11.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_LocationReduction.h"
#include "I_LocationAnalysis.h"

#ifndef LOCATIONREDUCTION_H
#define LOCATIONREDUCTION_H

using namespace gti;

namespace must
{
	/**
	 * Class for a reduction that removes
	 * unnecessary location records.
	 * @see MUST::LocationImpl
	 */
	class LocationReduction : public gti::ModuleBase<LocationReduction, I_LocationReduction>
	{
	public:
		/**
		 * Constructor.
		 * @param instanceName name of this module instance.
		 */
		LocationReduction (const char* instanceName);

		/**
		 * Destructor.
		 */
		virtual ~LocationReduction (void);

		/**
		 * @see I_LocationReduction::reduce
		 */
		gti::GTI_ANALYSIS_RETURN reduce (
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
				std::list<gti::I_ChannelId*> *outFinishedChannels);

		/**
		 * @see I_Reduction::timeout.
		 */
		void timeout (void);

	protected:
		I_LocationAnalysis *myLocationModule;
	}; /*class LocationReduction*/
} /*namespace MUST*/

#endif /*LOCATIONREDUCTION_H*/
