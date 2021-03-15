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
 * @file ShutdownHandler.h
 *       @see MUST::ShutdownHandler.
 *
 *  @date 27.06.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"

#include "I_ShutdownHandler.h"

#ifndef SHUTDOWNHANDLER_H
#define SHUTDOWNHANDLER_H

namespace gti
{
    /**
     * Implementation of I_ShutdownHandler.
     */
    class ShutdownHandler : public ModuleBase<ShutdownHandler, I_ShutdownHandler>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        ShutdownHandler (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~ShutdownHandler (void);

        /**
         * @see I_ShutdownHandler::notifyShutdown.
         */
        GTI_ANALYSIS_RETURN notifyShutdown (gti::I_ChannelId* channelId);

    protected:

    };
} /*namespace gti*/

#endif /*SHUTDOWNHANDLER_H*/
