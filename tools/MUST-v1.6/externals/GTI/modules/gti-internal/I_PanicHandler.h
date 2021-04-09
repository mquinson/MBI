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
 * @file I_PanicHandler.h
 *       @see I_PanicHandler.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"

#ifndef I_PANICHANDLER_H
#define I_PANICHANDLER_H

/**
 * Handler for GTI panic signals.
 * If a panic event arives (the first one), it notifies all non-application TBON nodes
 * of the ongoing panic, i.e., pending application crash.
 *
 * Dependencies (order as listed):
 * - X
 */
class I_PanicHandler : public gti::I_Module
{
public:

    /**
     * Notification of a panic event.
     *
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN raisePanic (void) = 0;

};/*class I_PanicHandler*/

#endif /*I_PANICHANDLER_H*/
