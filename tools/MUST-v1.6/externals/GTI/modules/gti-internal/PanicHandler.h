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
 * @file PanicHandler.h
 *       @see MUST::PanicHandler.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"

#include "I_PanicHandler.h"

#ifndef PANICHANDLER_H
#define PANICHANDLER_H

namespace gti
{
    /**
     * Implementation of I_PanicHandler.
     */
    class PanicHandler : public ModuleBase<PanicHandler, I_PanicHandler>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        PanicHandler (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~PanicHandler (void);

        /**
         * @see I_PanicHandler::raisePanic
         */
        GTI_ANALYSIS_RETURN raisePanic (void);

    protected:
        bool myHadPanic;
    };
} /*namespace gti*/

#endif /*PANICHANDLER_H*/
