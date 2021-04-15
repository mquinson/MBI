/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file DCollectiveCommInfo.h
 *       @see must::CollectiveCommInfo.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#ifndef DCOLLECTIVECOMMINFO_H
#define DCOLLECTIVECOMMINFO_H

#include "DCollectiveWave.h"
#include "I_Comm.h"
#include "I_ChannelId.h"
#include "DCollectiveOp.h"
#include "I_LocationAnalysis.h"
#include "I_DCollectiveListener.h"
#include "DCollectiveTypeMatchInfo.h"

#include <list>
#include <map>

using namespace gti;

namespace must
{
    /**
     * Collective matching and verification data for a single communicator.
     */
    class DCollectiveCommInfo
    {
    public:

        /**
         * Constructor.
         * @param comm used for this comm info, is managed by this class.
         */
        DCollectiveCommInfo (I_CommPersistent* comm);

        /**
         * Destructor.
         */
        ~DCollectiveCommInfo ();

        /**
         * Returns the comm of this info.
         */
        I_Comm* getComm (void);

        /**
         * Adds a new op that belongs to this communicator.
         * @param cId channel Id of the op.
         * @param outFinishedChannels finished channels list associated with the op.
         * @param newOp the op.
         * @param forceTimeout true if the wave to which this op belongs should be considered as timed out from the beginning,
         *                                    only applied if this op starts a new wave.
         * @return processing status to use for this op (SUCCESS, WAITING, IRREDUCIBLE, FAILURE).
         */
        GTI_ANALYSIS_RETURN addNewOp (
                I_DCollectiveListener* listener,
                I_ChannelId* cId,
                std::list<I_ChannelId*> *outFinishedChannels,
                DCollectiveOp* newOp,
                bool runIntraChecks,
                bool ancestorRunsIntraChecks,
                bool forceTimeout);

        /**
         * Prints the matching stat for this communicators as a graph.
         * It will be automatically put the output into a subgraph structure.
         * @param out stream to use.
         * @param nodeNamePrefix prefix to add to all identifiers in the DOT graph.
         * @param locations for printing pId/lId locations.
         * @return the stream.
         */
        std::ostream& printAsDot (std::ostream& out, std::string nodeNamePrefix, I_LocationAnalysis *locations);

        /**
         * Times all active waves out.
         */
        void timeout (void);

        /**
         * Returns true if this communicator has any wave (active, timedout, waiting-for-intra)
         * that is not yet completed.
         * @return true if an uncompleted wave exists.
         */
        bool hasUncompletedWaves (void);

        /**
         * Adds an information on intra layer communication based type matching to this communicator.
         * @param matchInfo to add to this communicators matching state.
         */
        void addNewTypeMatchInfo (DCollectiveTypeMatchInfo *matchInfo);

        /**
         * Returns true iff this communicator has an active (not timed out) wave.
         * @return true/false.
         */
        bool hasActiveWave (void);

        /**
         * Notifies that no aggregations shall be ran on this communicator any more.
         */
        void disableAggregation (void);

    protected:

        I_CommPersistent* myComm;
        int myNumReachableRanks; /**< Number of ranks in myComm that can be reached from this node.*/
        int myNextWaveNumber; /**< Id number for the next wave.*/
        bool myAggregate;

        int myStride, myOffset;

        std::list<DCollectiveWave*> myActiveWaves; /**< Currently active waves.*/
        std::list<DCollectiveWave*> myTimedOutWaves; /**< Waves that timed out.*/
        std::map<int, DCollectiveWave*> myWaitingForIntraWaves; /**< Waves for which intra layer events are still needed, maps wave number to the wave.*/

        std::map<int, std::list<DCollectiveTypeMatchInfo*> > myUnexpectedTypeMatchInfos; /**< Maps collective number to a list of type match infos for which no wave was created yet.*/
    };

} /*namespace must*/

#endif /*DCOLLECTIVECOMMINFO_H*/
