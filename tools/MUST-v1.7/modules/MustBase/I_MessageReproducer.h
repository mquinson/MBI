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
 * @file I_MessageReproducer.h
 *       @see I_MessageReproducer.
 *
 *  @date 26.05.2014
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_CreateMessage.h"
#include "I_ChannelId.h"

#include "BaseIds.h"

#ifndef I_MESSAGEREPRODUCER_H
#define I_MESSAGEREPRODUCER_H

/**
 * Interface for replaying a log of MUST messages during an application execution.
 * Targets use-cases such as hitting a breakpoint when the application reaches an MPI call that caused an error/warning in a previous
 * execution.
 *
 * Dependencies:
 * - I_ParallelIdAnalysis
 * - I_LocationAnalysis
 * - I_CreateMessage
 */
class I_MessageReproducer : public gti::I_Module
{
public:
	/**
	 * Checks whether the given pId and lId matches a call from the recorded log.
	 * @param pId parallel Id.
	 * @param lId location id.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN testForMatch (
    		uint64_t pId,
    		uint64_t lId
    		) = 0;
}; /*class I_MessageReproducer*/

#endif /*I_MESSAGEREPRODUCER_H*/
