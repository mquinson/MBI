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
 * @file CollectiveMatch.h
 *       @see must::CollectiveMatch.
 *
 *  @date 30.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_CreateMessage.h"
#include "I_CommTrack.h"
#include "I_DatatypeTrack.h"
#include "I_OpTrack.h"
#include "I_BaseConstants.h"
#include "I_OperationReordering.h"
#include "CollectiveOp.h"

#include "I_CollectiveMatch.h"

#ifndef COLLECTIVEMATCH_H
#define COLLECTIVEMATCH_H

using namespace gti;

namespace must
{
    /**
     * Forward declaration of CollectiveOp
     */
    class CollectiveOp;

    /**
     * Data structure used to hold information on a single
     * participant of a collective call.
     */
    class CollectiveParticipantInfo
    {
    public:
        /**
         * Default constructor.
         */
        CollectiveParticipantInfo (void);

        int currentMatchId; /**< Id of the match in which this participant is, if it equals CollectiveMatchInfo::currentMatchId it is part of the active match.*/
        bool isComplete; /**< True if all transfer operations for the collective arrived for this participant, false otherwise. Even if currentMatchId if of the current match the participant is only complete if this was set..*/
        CollectiveOp* sendTransfer; /**< Send part of the transfer, can be NULL for some collectives, information on collectives that do not perform a transfer is stored here too.*/
        CollectiveOp* recvTransfer; /**< Receive part of the transfer, can be NULL for some collectives..*/

        std::list<CollectiveOp*> queuedTransfers; /**< List of all transfers that can't be processed right now.*/
    };

    /**
     * Data structure used to hold information on matching going
     * on for a certain communicator.
     */
    class CollectiveMatchInfo
    {
    public:
        /**
         * Constructor that initializes the match info for the given communicator.
         */
        CollectiveMatchInfo (I_Comm*, int worldSize);

        int commSize; /**< Just for caching, could be queried from the comm at any time.*/
        int numParticipants; /**< Determines whether a match is going on (>0 if so) and how many tasks already joined. */
        int numCompletedParticipants; /**< Determines how many tasks provided complete information to the match (some ops require multiple transfer ops from each task). */
        int currentMatchId; /**< Used to determine whether a certain rank already provided information to a match or not,  the id is incremented after each completed match.*/
        MustCollCommType collId; /**< Collective used in the current match (used to detect collective mismatches).*/

        int rootRank; /**< Rank of the root process (in MPI_COMM_WORLD) only significant if the collective uses a root, KEEP IN MIND, THE ROOT DATA MAY NOT HAVE ARRIVED EVEN THOUGH THIS IS SET!.*/
        int firstParticipant; /**< Rank of the first participant that provided information to this collective.*/

        std::vector<CollectiveParticipantInfo> participants; /**< Individual informations for all the participants.*/
        std::vector<int> partIndices; /**< Indices (as rank used to index CollectiveMatchInfo::participants) of all participating ranks that provided their full information, .*/
    };

    /**
     * A matcher for collective calls. Implements
     * I_CollectiveMatch.
     */
    class CollectiveMatch : public gti::ModuleBase<CollectiveMatch, I_CollectiveMatch>
    {
        friend class CollectiveOp;

    public:

        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        CollectiveMatch (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~CollectiveMatch (void);

        /**
         * @see I_CollectiveMatch::CollNoTransfer.
         */
        GTI_ANALYSIS_RETURN CollNoTransfer (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                MustCommType comm,
                int numTasks, // counter for event aggregation
                int hasRequest
        );

        /**
         * @see I_CollectiveMatch::CollSend.
         */
        GTI_ANALYSIS_RETURN CollSend (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                int count,
                MustDatatypeType type,
                int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
                MustCommType comm,
                int hasOp,
                MustOpType op,
                int numTasks, // counter for event aggregation
                int hasRequest
        );

        /**
         * @see I_CollectiveMatch::CollSendN.
         */
        GTI_ANALYSIS_RETURN CollSendN (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                int count,
                MustDatatypeType type,
                int commsize,
                MustCommType comm,
                int hasOp,
                MustOpType op,
                int numTasks,
                int hasRequest
        ) ;

        /**
         * @see I_CollectiveMatch::CollSendCounts.
         */
        GTI_ANALYSIS_RETURN CollSendCounts (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                const int counts[],
                MustDatatypeType type,
                int commsize,
                MustCommType comm,
                int hasOp,
                MustOpType op,
                int numTasks,
                int hasRequest
        ) ;

        /**
         * @see I_CollectiveMatch::CollSendTypes.
         */
        GTI_ANALYSIS_RETURN CollSendTypes (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                const int counts[],
                const MustDatatypeType types[],
                int commsize,
                MustCommType comm,
                int numTasks,
                int hasRequest
        ) ;

        /**
         * @see I_CollectiveMatch::CollRecv.
         */
        GTI_ANALYSIS_RETURN CollRecv (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                int count,
                MustDatatypeType type,
                int src, /*Root process to receive from as a rank in MPI_COMM_WORLD*/
                MustCommType comm,
                int numTasks,
                int hasRequest
        ) ;

        /**
         * @see I_CollectiveMatch::CollRecvN.
         */
        GTI_ANALYSIS_RETURN CollRecvN (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                int count,
                MustDatatypeType type,
                int commsize,
                MustCommType comm,
                int hasOp,
                MustOpType op,
                int numTasks,
                int hasRequest
        ) ;


        /**
         * @see I_CollectiveMatch::CollRecvCounts.
         */
        GTI_ANALYSIS_RETURN CollRecvCounts (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                const int counts[],
                MustDatatypeType type,
                int commsize,
                MustCommType comm,
                int numTasks,
                int hasRequest
        ) ;

        /**
         * @see I_CollectiveMatch::CollRecvTypes.
         */
        GTI_ANALYSIS_RETURN CollRecvTypes (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly MustCollCommType
                const int counts[],
                const MustDatatypeType types[],
                int commsize,
                MustCommType comm,
                int numTasks,
                int hasRequest
        ) ;

        /**
         * @see I_CollectiveMatch::registerListener
         */
        GTI_RETURN registerListener (I_CollMatchListener *listener);

        /**
         * @see I_CollectiveMatch::checkpoint
         */
        void checkpoint (void);

        /**
         * @see I_CollectiveMatch::rollback
         */
        void rollback (void);

    protected:
        ////Child modules
        I_ParallelIdAnalysis* myPIdMod;
        I_BaseConstants *myConsts;
        I_CreateMessage* myLogger;
        I_CommTrack* myCTrack;
        I_DatatypeTrack* myDTrack;
        I_OpTrack* myOTrack;
        I_OperationReordering* myOrder;

        ////Matching data
        typedef std::map<I_CommPersistent*, CollectiveMatchInfo> MatchStructure;
        MatchStructure myMatching;
        MatchStructure myCheckpointMatching;

        bool myActive; /**< Controlls whether the matching is active, it deactivates with the first collective match error as no reasonable processing is possible after that point.*/
        bool myCheckpointActive;

        //// Debugging data
#ifdef MUST_DEBUG
        ////Matching history (Debugging)
        typedef std::map<I_CommPersistent*, std::list<MustCollCommType> > MatchHistory;
        MatchHistory myHistory;
        MatchHistory myCheckpointHistory;
#endif

        //// Listeners
        std::list<I_CollMatchListener*> myListeners;

        /**
         * Takes care of either executing or queueing (localy or in
         * I_OperationReordering) a new operation.
         * @param rank of the op.
         * @param newOp new operation.
         * @return true if successful, false otherwise.
         */
        bool handleNewOp (int rank, CollectiveOp* newOp);

        /**
         * Returns the information for the given communicator.
         * Checks whether this is a valid communicator for a
         * transfer (i.e. has a group). If so it returns true and
         * the persistent information in outComm. Otherwise
         * outComm will not be set and the funtion returns
         * false.
         * @param pId context for comm.
         * @param comm handle to get an information for.
         * @param outComm pointer to information, only set
         *            if successful, must be freed by the user if
         *            set.
         * @return true is successful, false otherwise.
         */
        bool getCommInfo (
                MustParallelId pId,
                MustCommType comm,
                I_CommPersistent **outComm);

        /**
         * Like getCommInfo but for datatypes, also checks for validity.
         *
         * @param pId context.
         * @param type handle to query for.
         * @param outType pointer to storage for information.
         * @return true is successful, false otherwise.
         */
        bool getTypeInfo (
                MustParallelId pId,
                MustDatatypeType type,
                I_DatatypePersistent **outType);

        /**
         * Like getCommInfo but for operations, also checks for validity.
         *
         * @param pId context.
         * @param op handle to query for.
         * @param outOp pointer to storage for information.
         * @return true is successful, false otherwise.
         */
        bool getOpInfo (
                MustParallelId pId,
                MustOpType op,
                I_OpPersistent** outOp);

        /**
         * Is called when a new operation was accepted for processing by
         * I_OperationReordering and can be tested against the current
         * matching state.
         * @param op to proces.
         * @return true if successful.
         */
        bool addCollectiveTransfer (CollectiveOp* op);

        /**
         * Adds the op to the given match info.
         * @param info for the comm used by the op.
         * @param op to add.
         * @return true iff successful.
         */
        bool processTransferForInfo (CollectiveMatchInfo* info, CollectiveOp* op);

        /**
         * Applies the given operation to the given participant of the given match info.
         * @param info match info.
         * @param part participant in match info.
         * @param op to apply.
         * @return true iff successful.
         */
        bool applyOpToParticipant (CollectiveMatchInfo* info, CollectiveParticipantInfo *part, CollectiveOp* op);

        /**
         * Returns a textual representation of the given collective id.
         * @param id to convert.
         * @return string.
         */
        static std::string collIdToString (MustCollCommType id);

        /**
         * Clears and deletes all information of a matching structure.
         * @param matching structure to clear.
         * @return true if there where any open operations in the matching, false otherwise.
         */
        bool clearMatching (MatchStructure *matching);

        /**
         * Disables collective matching and reports an error message due to the
         * presence of an unsupported collective operation.
         * @param pId parallel ID for the first operation that was unsupported.
         * @param lId location ID for the first operation that was unsupported.
         */
        bool reportNonblockingCollectiveUnsopported (
                MustParallelId pId,
                MustLocationId lId);

#ifdef MUST_DEBUG
        void clearHistory (MatchHistory* history);
#endif

    }; /*CollectiveMatch*/
} /*namespace must*/

#endif /*COLLECTIVEMATCH_H*/
