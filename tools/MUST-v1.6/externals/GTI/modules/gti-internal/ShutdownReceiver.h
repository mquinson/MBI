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
 * @file ShutdownReceiver.h
 *       @see MUST::ShutdownReceiver.
 *
 *  @date 27.06.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"

#include "I_ShutdownReceiver.h"

#ifndef SHUTDOWNRECEIVER_H
#define SHUTDOWNRECEIVER_H

namespace gti
{
    /**
     * Implementation of I_ShutdownReceiver.
     */
    class ShutdownReceiver : public ModuleBase<ShutdownReceiver, I_ShutdownReceiver>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        ShutdownReceiver (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~ShutdownReceiver (void);

        /**
         * @see I_ShutdownReceiver::receive
         */
        GTI_ANALYSIS_RETURN receive (void);

    protected:
    };
} /*namespace gti*/

#endif /*SHUTDOWNRECEIVER_H*/
