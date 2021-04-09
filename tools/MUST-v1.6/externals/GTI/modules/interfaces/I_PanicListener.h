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
 * @file I_PanicListener.h
 *       @see I_PanicListener.
 *
 * @author Tobias Hilbrich
 * @date 11.07.2012
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"

#ifndef I_PANICLISTENER_H
#define I_PANICLISTENER_H

namespace gti
{
    /**
     * Extensive documentation see:
     * @see I_CommStrategyDown
     */
    class I_PanicListener : public I_Module
    {
    public:
    	/**
    	 * Virtual destructor to make compilers happy.
    	 */
    	inline virtual ~I_PanicListener (void) {};

        /**
         * Flushes this communication strategy and sets it to communicate
         * immediately from now on.
         */
        virtual GTI_RETURN flushAndSetImmediate (void) = 0;

        /**
         * Causes the communication strategy to flush all its
         * not yet sent data.
         * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
         *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
         *         connected or was shutdown early.
         *
         * @todo Interaction with blocking receives !
         */
        virtual GTI_RETURN flush (
                void
        ) = 0;
    }; /*I_PanicListener*/
}

#endif /* I_PANICLISTENER_H */
