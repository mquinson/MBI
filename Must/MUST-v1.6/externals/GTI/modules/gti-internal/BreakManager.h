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
 * @file BreakManager.h
 *       @see BreakManager.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "GtiApi.h"

#include "I_BreakManager.h"

#ifndef BREAKMANAGER_H
#define BREAKMANAGER_H

namespace gti
{
    /**
     * Implementation of I_BreakManager.
     */
    class BreakManager : public ModuleBase<BreakManager, I_BreakManager>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        BreakManager (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~BreakManager (void);

        /**
         * @see gti::requestBreak
         */
        GTI_ANALYSIS_RETURN requestBreak (I_ChannelId *thisChannel);

        /**
         * @see gti::removeBreak
         */
        GTI_ANALYSIS_RETURN removeBreak (I_ChannelId *thisChannel);

    protected:
        gtiBroadcastBreakP myFBroadcastBreak;
        bool myRequestedBreak;

    }; /*BreakManager*/
} /*namespace gti*/

#endif /*BREAKMANAGER_H*/
