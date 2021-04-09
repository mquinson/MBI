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
 * @file BreakReduction.h
 *       @see BreakReduction.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"

#include "I_BreakReduction.h"

#ifndef BREAKREDUCTION_H
#define BREAKREDUCTION_H

namespace gti
{
    /**
     * Implementation of I_BreakReduction.
     */
    class BreakReduction : public ModuleBase<BreakReduction, I_BreakReduction>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        BreakReduction (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~BreakReduction (void);

        /**
         * @see gti::addBreakRequest
         */
        GTI_ANALYSIS_RETURN addBreakRequest (
                gti::I_ChannelId *thisChannel,
                std::list<gti::I_ChannelId*> *outFinishedChannels);

        /**
         * @see gti::removeBreakRequest
         */
        GTI_ANALYSIS_RETURN removeBreakRequest (
                gti::I_ChannelId *thisChannel,
                std::list<gti::I_ChannelId*> *outFinishedChannels);

        /**
         * The timeout function, see gti::I_Reduction.timeout
         */
        void timeout (void);

    protected:
        int myRequestCount;

    }; /*BreakReduction*/
} /*namespace gti*/

#endif /*BREAKREDUCTION_H*/
