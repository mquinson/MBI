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
 * @file GtiHelper.h
 *       @see gti::GtiHelper
 *
 * @author Felix M�nchhalfen
 *
 * @date 16.04.2014
 *
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include <pthread.h>

#ifndef GTIHELPER_H
#define GTIHELPER_H

namespace gti
{
    /**
     * Collection of helper functions for GTI classes.
     */
    class GtiHelper
    {
    protected:

        static int myRankInLayer; /**< Own ID within the TBON layer where the id is within a continous range of 0-[layerSize-1].*/
        static bool myInitedRank; /**< True if the own layer ID was set and false otherwise.*/
        static pthread_mutex_t myIdLock;
        
    public:
        
        class GtiHelperStaticInitializer {
        public:
            GtiHelperStaticInitializer();
            ~GtiHelperStaticInitializer();
        };

        /**
         * Constructor.
         */
        GtiHelper (void);

        /**
         * Destructor.
         */
        virtual ~GtiHelper (void);

        /**
         * Calls either buildLayerIdAsPureMpiLayer() or 
         * buildLayerIdAsHybridLayer(), depending
         * on the type of application(TODO) and compile-flags.
         */
        
        GtiTbonNodeInLayerId buildLayer(bool threaded = true);
        
        /**
         * Tries to retrieve an instanceName from PnMPI module local data.
         * Should only be used by modules which have 'instanceToUse'
         * set in their PnMPI configuration.
         */
        static GTI_RETURN getInstanceName (const char **instanceName);
        
    protected:
        /**
         * Provides a TBON layer id (0-[layerSize-1]) for this node.
         * This implementation assumes a pure MPI spawned layer and
         * uses the MPI rank as this layer id.
         */
        GtiTbonNodeInLayerId buildLayerIdAsPureMpiLayer ();
        
        /**
         * Provides a TBON layer id (0-[layerSize-1]) in the lower 32bits.
         * Provides a kernel-thread-id in the upper 32bits.
         * This implementation assumes an application that makes use both
         * of threads and MPI.
         */
        GtiTbonNodeInLayerId buildLayerIdAsHybridLayer();

    }; /*GtiHelper*/
} /*namespace gti*/

#endif	/* GTIHELPER_H */

