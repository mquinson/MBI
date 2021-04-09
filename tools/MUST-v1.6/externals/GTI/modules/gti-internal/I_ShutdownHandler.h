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
 * @file I_ShutdownHandler.h
 *       @see I_ShutdownHandler.
 *
 *  @date 27.06.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_ChannelId.h"

#ifndef I_SHUTDOWNHANDLER_H
#define I_SHUTDOWNHANDLER_H

/**
 * Handler for GTI shutdown.
 * Waits till the last shutdown event arrives and then broadcasts a notification event that lets all
 * preceding layers shut down.
 *
 * Dependencies (order as listed):
 * - X
 */
class I_ShutdownHandler : public gti::I_Module
{
public:

    /**
     * Notification of a shutdown event.
     *
     * @param channelId of the event.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN notifyShutdown (gti::I_ChannelId* channelId) = 0;

};/*class I_ShutdownHandler*/

#endif /*I_SHUTDOWNHANDLER_H*/
