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
 * @file BreakEnforcer.h
 *       @see BreakEnforcer.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_CommStrategyUp.h"

#include "I_BreakEnforcer.h"

#include <list>
#include <sys/time.h>

#ifndef BREAKENFORCER_H
#define BREAKENFORCER_H

namespace gti
{
    /**
     * Implementation of I_BreakEnforcer.
     */
    class BreakEnforcer : public ModuleBase<BreakEnforcer, I_BreakEnforcer>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        BreakEnforcer (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~BreakEnforcer (void);

        /**
         * @see gti::I_BreakEnforcer::test
         */
        GTI_ANALYSIS_RETURN test (void);

        /**
         * @see gti::I_BreakEnforcer::handleBroadcastBreak
         */
        GTI_ANALYSIS_RETURN handleBroadcastBreak (
                int code);

    protected:
        std::list<I_CommStrategyUp*> myStrats;
        uint64_t mySecLastTest;

    }; /*BreakEnforcer*/
} /*namespace gti*/

#endif /*BREAKENFORCER_H*/
