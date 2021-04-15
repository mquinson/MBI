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
 * @file ThreadedMpiPlace.h
 *       Header for a threaded MPI application based
 *       placement driver.
 *
 * @author Tobias Hilbrich, Felix Münchhalven, Joachim Protze
 *
 */

#define PROVIDE_TID_IMPLEMENTATION 1
#include "GtiTLS.h"
#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Place.h"
#include "GtiHelper.h"
#include "ModuleBase.h"
#include "I_CommStrategyUp.h"
#include "I_CommStrategyDown.h"
#include "I_CommStrategyIntra.h"
#include "I_PlaceReceival.h"
#include "I_Profiler.h"
#include "I_FloodControl.h"

#ifndef APP_PLACE_H
#define APP_PLACE_H

namespace gti
{

    /**
     * Class for an Application Place
     *
     * Needs at least 1 sub module:
     *  1st: Comm Strategy upwards (to get Placement information)
     */
    class AppPlace : public ModuleBase<AppPlace, I_Place>//, public GtiHelper
    {
    public:
        /**
         * Constructor.
         * Sets up the this tool place with all its modules.
         * @param instanceName name of this module instance
         */
        AppPlace (const char* instanceName);

        /**
         * Destructor.
         */
        ~AppPlace (void);

    protected:
        I_CommStrategyUp* myCommStratUp;

        /**
         * @see gti::I_Place::getNodeInLayerId
         */
        GTI_RETURN getNodeInLayerId (GtiTbonNodeInLayerId* id);

        /**
         * @see gti::I_Place::getLayerIdForApplicationRank
         */
        GTI_RETURN getLayerIdForApplicationRank (int rank, GtiTbonNodeInLayerId* id);

    }; /*class AppPlace*/
} /*namespace gti*/

#endif /* THREADED_APP_PLACE_H */
