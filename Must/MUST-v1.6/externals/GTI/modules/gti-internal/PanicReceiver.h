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
 * @file PanicReceiver.h
 *       @see MUST::PanicReceiver.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_PanicListener.h"

#include "I_PanicReceiver.h"

#include <list>

#ifndef PANICRECEIVER_H
#define PANICRECEIVER_H

namespace gti
{
    /**
     * Implementation of I_PanicReceiver.
     */
    class PanicReceiver : public ModuleBase<PanicReceiver, I_PanicReceiver>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        PanicReceiver (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~PanicReceiver (void);

        /**
         * @see I_PanicReceiver::notifyPanic
         */
        GTI_ANALYSIS_RETURN notifyPanic (void);

        /**
         * @see I_PanicReceiver::notifyFlush
         */
        GTI_ANALYSIS_RETURN notifyFlush (void);

        /**
         * @see I_PanicReceiver::notifyRaisePanic
         */
        GTI_ANALYSIS_RETURN notifyRaisePanic (void);

    protected:
        std::list<I_PanicListener*> myListeners;
    };
} /*namespace gti*/

#endif /*PANICRECEIVER_H*/
