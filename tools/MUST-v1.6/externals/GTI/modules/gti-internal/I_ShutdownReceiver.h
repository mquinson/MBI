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
 * @file I_ShutdownReceiver.h
 *       @see I_ShutdownReceiver.
 *
 *  @date 27.06.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"

#ifndef I_SHUTDOWNRECEIVER_H
#define I_SHUTDOWNRECEIVER_H

/**
 * Module that listens for the shutdown event created by ShutdownHandler.
 * It performs no action with this event, it just makes sure that it is forwarded
 * to the right places.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_ShutdownReceiver : public gti::I_Module
{
public:

    /**
     * Called when the shutdown event arrives.
     *
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN receive (void) = 0;
};/*class I_ShutdownReceiver*/

#endif /*I_SHUTDOWNRECEIVER_H*/
