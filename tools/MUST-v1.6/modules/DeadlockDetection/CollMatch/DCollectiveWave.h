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
 * @file DCollectiveWave.h
 *       @see must::DCollectiveWave.
 *
 *  @date 26.04.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#ifndef DCOLLECTIVEWAVE_H
#define DCOLLECTIVEWAVE_H

#include "DCollectiveOp.h"
#include "DCollectiveTypeMatchInfo.h"
#include "I_ChannelId.h"
#include "CompletionTree.h"
#include "I_LocationAnalysis.h"
#include "I_DCollectiveListener.h"

#include <list>

using namespace gti;

namespace must
{
    /**
     * Collective matching and verification data for a wave of collective calls.
     */
    class DCollectiveWave
    {
    public:

        /**
         * Constructor.
         * @param collId operation in this wave
         * @param numReachableRanks number of ranks that this process can reach
         * @param waveNumber id of this wave (identifies this wave within the communicator)
         */
        DCollectiveWave (
                MustCollCommType collId,
                int numReachableRanks,
                int waveNumber);

        /**
         * Destructor.
         */
        ~DCollectiveWave ();

        /**
         * Checks whether the given op belongs to this wave.
         * @param cId channel id of the op.
         * @param op to check.
         * @return true iff the op belongs to the wave.
         */
        bool belongsToWave (I_ChannelId *cId, DCollectiveOp *op);

        /**
         * Adds the given op to this wave.
         * @param cId channel ID of the op.
         * @param outFinishedChannels associated with the op.
         * @param op to add.
         * @return @see GTI_ANALYSIS_RETURN as is used for reductions.
         */
        GTI_ANALYSIS_RETURN addNewOp (
                I_DCollectiveListener *listener,
                I_ChannelId *cId,
                std::list<I_ChannelId*> *outFinishedChannels,
                DCollectiveOp *op,
                bool runIntraChecks,
                bool ancestorRunsIntraChecks,
                int commStride,
                int commOffset);

        /**
         * Returns true if this wave is completed, i.e.
         * all transfers arrived and where processed.
         * @return true iff wave completed.
         */
        bool isCompleted (void);

        /**
         * Prints the matching state for this wave as a graph.
         * It will be automatically put the output into a subgraph structure.
         * @param out stream to use.
         * @param nodeNamePrefix prefix to add to all identifiers in the DOT graph.
         * @param locations for printing pId/lId locations.
         * @return the stream.
         */
        std::ostream& printAsDot (std::ostream& out, std::string nodeNamePrefix, I_LocationAnalysis *locations);

        /**
         * Times this wave out.
         */
        void timeout (void);

        /**
         * Aborts the aggregations of this wave, moves all channels for which we returned waiting into
         * the given list.
         * @param outFininshedChannels to add our waiting channels to.
         */
        void abort (std::list<I_ChannelId*> *outFinishedChannels);

        /**
         * True if this wave still needs type match info events communicated via
         * intra layer communication.
         * @return true if waiting.
         */
        bool waitsForIntraTypeMatchInfos (void);

        /**
         * Returns the number of this wave.
         * (Identifier used to identify the wave within its communicator)
         */
        int getWaveNumber (void);

        /**
         * Adds a new type match info that was received via intra layer communication.
         */
        void addNewTypeMatchInfo (DCollectiveTypeMatchInfo *matchInfo);

        /**
         * Like the other version of this function but adds a whole list of type match infos.
         * It removes them from the input list and adds them to this waves list.
         */
        void addNewTypeMatchInfo (std::list<DCollectiveTypeMatchInfo*> &matches);

    protected:

        int myNumReachableRanks;
        int myNumJoinedSendRanks;
        int myNumJoinedReceiveRanks;
        int myNumExpectedSendRanks;
        int myNumExpectedReceiveRanks;
        bool mySendIsRooted;
        bool myReceiveIsRooted;

        int myNumExpectedIntraTypeMatchInfos; /**< Number of messages (DCollectiveTypeMatchInfo objects) to receive.*/
        int myNumJoinedIntraTypeMatchInfos;

        int myRoot; /**< Rank of root transfer (if present, -1 otherwise).*/
        bool myRootReachable; /**< True if this node can receive the root event (if present for this collective).*/
        MustCollCommType myCollId;
        DCollectiveOp* myRootOp; //If we have a root transfer, this is its op (send or receive)

        CompletionTree *mySendCompletion;
        CompletionTree *myReceiveCompletion;

        std::list<DCollectiveOp*> mySendTransfers;
        std::list<DCollectiveOp*> myReceiveTransfers;

        std::list<I_ChannelId*> mySendChannels;
        std::list<I_ChannelId*> myReceiveChannels;

        bool myTimedOut;

        int myWaveNumber;

        std::list<DCollectiveTypeMatchInfo*> myRemoteTypeMatchInfos;

        /**
         * Initializes all completions.
         */
        void initCompletions (I_ChannelId *cId);

        /**
         * Creates the reduced record for the send or the receive wave.
         * @param forSend true if send wave record should be created.
         */
        void createReducedRecord (bool forSend, int commStride, int commOffset);

        /**
         * Runs the intra layer type matching, only run if ALL information
         * for this wave arrived.
         */
        void intraLayerTypeMatching (void);
    };

} /*namespace must*/

#endif /*DCOLLECTIVEWAVE_H*/
