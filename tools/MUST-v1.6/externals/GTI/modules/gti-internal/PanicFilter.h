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
 * @file PanicFilter.h
 *       @see MUST::PanicFilter.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"

#include "I_PanicFilter.h"

#ifndef PANICFILTER_H
#define PANICFILTER_H

namespace gti
{
    /**
     * Implementation of I_PanicFilter.
     */
    class PanicFilter : public ModuleBase<PanicFilter, I_PanicFilter>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        PanicFilter (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~PanicFilter (void);

        /**
         * @see I_PanicFilter::propagate
         */
        GTI_ANALYSIS_RETURN propagate (
                gti::I_ChannelId *thisChannel,
                std::list<gti::I_ChannelId*> *outFinishedChannels);

        /**
         * The timeout function, see gti::I_Reduction.timeout
         */
        void timeout (void);

    protected:
        bool myHadPanic;
    };
} /*namespace gti*/

#endif /*PANICFILTER_H*/
