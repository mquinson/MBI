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
 * @file I_BreakManager.h
 *       @see I_BreakManager.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_ChannelId.h"

#ifndef I_BREAKMANAGER_H
#define I_BREAKMANAGER_H

/**
 * Notifies all application processes if a tool node requests a break and
 * manages the resumption of the normal processing.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_BreakManager : public gti::I_Module
{
public:

    /**
     * Handles a request for a break.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN requestBreak (
            gti::I_ChannelId *thisChannel) = 0;

    /**
     * Handles a request to resume processing.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN removeBreak (
            gti::I_ChannelId *thisChannel) = 0;

};/*class I_BreakManager*/

#endif /*I_BREAKMANAGER_H*/
