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
 * @file I_FinalizeNotify.h
 *       @see I_FinalizeNotify.
 *
 *  @date 04.04.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_FINALIZENOTIFY_H
#define I_FINALIZENOTIFY_H

/**
 * Triggers a notifcation event on the API call finalizeNotify when MPI_Finalize
 * is issued, must be placed on all application processes for MUST based tools.
 *
 * Dependencies (order as listed):
 *
 */
class I_FinalizeNotify : public gti::I_Module
{
public:

	/**
	 * Triggers the notification, must be mapped to MPI_Finalize pre.
	 *
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN notify () = 0;
};/*class I_FinalizeNotify*/

#endif /*I_FINALIZENOTIFY_H*/
