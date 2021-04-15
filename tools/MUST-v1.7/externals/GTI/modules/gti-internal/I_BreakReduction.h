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
 * @file I_BreakReduction.h
 *       @see I_BreakReduction.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_Reduction.h"
#include "I_ChannelId.h"

#include <list>

#ifndef I_BREAKREDUCTION_H
#define I_BREAKREDUCTION_H

/**
 * Removes unnecessary break requests and makes sure that we react to
 * the right (last) consume request.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_BreakReduction : public gti::I_Module, public gti::I_Reduction
{
public:

    /**
     * Handles a request for a break.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addBreakRequest (
            gti::I_ChannelId *thisChannel,
            std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

    /**
     * Handles a request to resume processing.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN removeBreakRequest (
            gti::I_ChannelId *thisChannel,
            std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

};/*class I_BreakReduction*/

#endif /*I_BREAKREDUCTION_H*/
