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
 * @file I_FinalizeMust.h
 *       @see I_FinalizeMust.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_FINALIZEMUST_H
#define I_FINALIZEMUST_H

/**
 * Trigers the actual finalize event once we get informed of the finalizeNotify event.
 * This is since MPI_Finalize now is only a local finalizer, i.e. it shuts down the application
 * layer, but the remainder of the TBON is now either shut down with this module (when
 * we see the last MPI_Finalize) or with DWaitState, which has its own logic.
 *
 * Dependencies (order as listed):
 *
 */
class I_FinalizeMust : public gti::I_Module
{
public:

	/**
	 * Trigers us to generate the finalize event we define in BaseApi.h.
	 *
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN notify () = 0;
};/*class I_FinalizeMust*/

#endif /*I_FINALIZEMUST_H*/
