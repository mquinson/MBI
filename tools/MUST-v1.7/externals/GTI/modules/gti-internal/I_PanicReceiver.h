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
 * @file I_PanicReceiver.h
 *       @see I_PanicReceiver.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"

#ifndef I_PANICRECEIVER_H
#define I_PANICRECEIVER_H

/**
 * Listens for any incoming panic notifications or flush requests.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_PanicReceiver : public gti::I_Module
{
public:

    /**
     * Called when a panic notification arrives.
     *
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN notifyPanic (void) = 0;

    /**
     * Called when a panic notification arrives.
     *
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN notifyFlush (void) = 0;

    /**
     * Called when a gtiRaisePanic event crosses this TBON node.
     *
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN notifyRaisePanic (void) = 0;

};/*class I_PanicReceiver*/

#endif /*I_PANICRECEIVER_H*/
