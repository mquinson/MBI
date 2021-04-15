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
 * @file I_PanicFilter.h
 *       @see I_PanicFilter.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_Reduction.h"
#include "I_ChannelId.h"

#include <list>

#ifndef I_PANICFILTER_H
#define I_PANICFILTER_H

/**
 * Removes unnecessary gtiRaisePanic events.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_PanicFilter : public gti::I_Module, public gti::I_Reduction
{
public:

    /**
     * Called when a panic event arrives.
     *
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN propagate (
            gti::I_ChannelId *thisChannel,
            std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;
};/*class I_PanicFilter*/

#endif /*I_PANICFILTER_H*/
