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
 * @file I_Place.h
 *       Place interface supported by all Placement modules.
 *
 * @author Joachim Protze
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"

#include <string>

#ifndef I_PLACE_H
#define I_PLACE_H

namespace gti
{
    /**
     * Interface used by all modules.
     */
    class I_Place : public I_Module
    {
    public:

        /**
         * Virtual destructor.
         */
        virtual ~I_Place () {}

        /**
         * test for incomming Broadcast event
         */
        virtual GTI_RETURN testBroadcast (void) {return GTI_SUCCESS;}

        /**
         * test for incomming Intralayer event
         */
        virtual GTI_RETURN testIntralayer (void) {return GTI_SUCCESS;}

        /**
         * Returns the the id of this node in this layer of the Tbon
         * @return name of this module.
         */
        virtual GTI_RETURN getNodeInLayerId (GtiTbonNodeInLayerId* id) = 0;

        /**
         * Returns the the id of this node in this layer of the Tbon
         * @return name of this module.
         */
        virtual GTI_RETURN getLayerIdForApplicationRank (int rank, GtiTbonNodeInLayerId* id) = 0;

    }; /*class I_Place*/
} /*namespace gti*/

#endif /* I_PLACE_H */
