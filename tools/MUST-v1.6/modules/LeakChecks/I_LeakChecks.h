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
 * @file I_LeakChecks.h
 *       @see I_LeakChecks.
 *
 *  @date 17.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_ChannelId.h"

#ifndef I_LEAKCHECKS_H
#define I_LEAKCHECKS_H

/**
 * Checks whether MPI resources were leaked when MPI_Finalize
 * is called.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - CommTrack
 * - DatatypeTrack
 * - ErrTrack
 * - GroupTrack
 * - KeyvalTrack
 * - OpTrack
 * - RequestTrack
 *
 */
class I_LeakChecks : public gti::I_Module
{
public:

	/**
	 * Notification of a set of MPI_Finalize calls.
	 *
	 * @param thisChannel channel id of completion event, or NULL if all processes connected to this place completed.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN finalizeNotify (gti::I_ChannelId *thisChannel) = 0;
};/*class I_LeakChecks*/

#endif /*I_LEAKCHECKS_H*/
