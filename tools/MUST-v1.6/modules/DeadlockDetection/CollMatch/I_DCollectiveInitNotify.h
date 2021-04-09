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
 * @file I_DCollectiveInitNotify.h
 *       @see I_DCollectiveInitNotify.
 *
 *  @date 03.05.2012
 *  @author Tobias Hilbrich, Fabian Haensel, Mathias Korepkat, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_DCOLLECTIVEINITNOTIFY_H
#define I_DCOLLECTIVEINITNOTIFY_H

/**
 * Module that creates the ancestorHasIntraComm event for
 * DCollectiveMatch.
 *
 * Dependencies (order as listed):
 * - /
 *
 */
class I_DCollectiveInitNotify : public gti::I_Module
{
public:

	/**
	 * Mapped to MPI_Init, creates dCollMatchAncestorHasIntra.
	 *
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN notifyInit (void) = 0;
};/*class I_DCollectiveInitNotify*/

#endif /*I_DCOLLECTIVEINITNOTIFY_H*/
