/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file I_BreakEnforcer.h
 *       @see I_BreakEnforcer.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#ifndef I_BREAKENFORCER_H
#define I_BREAKENFORCER_H

/**
 * Runs on application processes and checks whether we got a break.
 * If so we busy wait for the resume command.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_BreakEnforcer : public gti::I_Module
{
public:

    /**
     * Tests whether we got the break command.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN test (
            void) = 0;

    /**
     * Handles gtiBroadcastBreak events.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN handleBroadcastBreak (
            int code) = 0;

};/*class I_BreakEnforcer*/

#endif /*I_BREAKENFORCER_H*/
