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
 * @file I_Module.h
 *       Base interface supported by all modules.
 *
 * @author Tobias Hilbrich
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include <string>

#ifndef I_MODULE_H
#define I_MODULE_H

namespace gti
{
    /**
     * Interface used by all modules.
     */
    class I_Module
    {
    public:

        /**
         * Virtual destructor.
         */
        virtual ~I_Module () {}

        /**
         * Returns the name of this module.
         * @return name of this module.
         */
        virtual std::string getName (void) = 0;
        virtual bool usesTLS()=0;

    }; /*class I_Module*/
} /*namespace gti*/

#endif /* I_MODULE_H */
