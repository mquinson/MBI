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
 * @author Tobias Hilbrich
 *
 */

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

#ifndef THREADED_MPI_PLACE_H
#define THREADED_MPI_PLACE_H

namespace gti
{
    /**
     * Class for a multi-threaded MPI based placement
     * driver.
     *
     * Needs at least 2 sub modules:
     *  1st: Comm Strategy downwards (to receive trace information)
     *  2nd: Receival module (to process and forward trace information
     *  [Optional] 3rd/4th: Wrapper module runing on this place
     *                  Use case is to shut down the wrapper when the place
     *                  finishes.
     *  [Optional] 3rd/4th: Intra communication strategy.
     */
    class ThreadedMPIPlace : public ModuleBase<ThreadedMPIPlace, I_Place>, public GtiHelper//, public Place
    {
    public:
        /**
         * Constructor.
         * Sets up the this tool place with all its modules.
         * @param instanceName name of this module instance
         */
        ThreadedMPIPlace (const char* instanceName);

        /**
         * Destructor.
         */
        ~ThreadedMPIPlace (void);

        /**
         * Starts the tool place.
         */
        void run (void);

        /**
         * Inits the tool place.
         */
        void init (void);

        /**
         * @see gti::I_Place::testBroadcast
         */
        GTI_RETURN testBroadcast (void);
        /**
         * @see gti::I_Place::testIntralayer
         */
        GTI_RETURN testIntralayer (void);
        /**
         * @see gti::I_Place::getNodeInLayerId
         */
        GTI_RETURN getNodeInLayerId (GtiTbonNodeInLayerId* id);

        /**
         * @see gti::I_Place::getLayerIdForApplicationRank
         */
        GTI_RETURN getLayerIdForApplicationRank (int rank, GtiTbonNodeInLayerId* id);

    protected:
        I_CommStrategyDown* myTraceRecv;
        I_PlaceReceival*    myReceival;
        I_Module*           myWrapper;
        I_CommStrategyIntra* myIntraRecv;
        I_Profiler* myProfiler;
        I_FloodControl* myFloodControl;
        std::vector<I_CommStrategyUp*> myUpStrats;

        uint64_t myIntraCommTime, myIntraCommCount;
        uint64_t myUpCommTime, myUpCommCount;
        bool finalize;

        /**
         * Receives an intra layer event and processes it.
         * Returns true iff successful and false if some error
         * occured.
         * @param hadEvent true if an event was received.
         */
        bool receiveAndProcessIntraLayerEvent (bool *hadEvent);

        /**
         * Receives a broadcast event and processes it (if available).
         * Returns true iff successful and false if some error
         * occured.
         * @param hadEvent true if an event was received.
         * @param pOutFinalize pointer to bool, which is set to true if this place can shut down.
         */
        bool receiveAndProcessBroadcastEvent (bool *hadEvent, bool *pOutFinalize);

        /**
         * Receives all remaining intra layer events and issues
         * I_CommStrategyIntra::communicationFinished until
         * it indicates that no outstanding events exist.
         */
        bool finishIntraCommunication (void);

    }; /*class ThreadedMPIPlace*/
} /*namespace gti*/

#endif /* THREADED_MPI_PLACE_H */
