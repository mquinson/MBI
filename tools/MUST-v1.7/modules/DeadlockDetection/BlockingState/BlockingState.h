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
 * @file BlockingState.h
 *       @see must::BlockingState.
 *
 *  @date 08.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat,
 */

#include "ModuleBase.h"
#include "CompletionTree.h"
#include "I_ParallelIdAnalysis.h"
#include "I_CreateMessage.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"
#include "I_BaseConstants.h"
#include "I_OperationReordering.h"
#include "I_P2PMatch.h"
#include "I_CollectiveMatch.h"
#include "I_LocationAnalysis.h"
#include "BlockingOp.h"
#include "BlockingP2P.h"
#include "BlockingColl.h"
#include "BlockingCompletion.h"
#include "BlockingRequestCompletion.h"

#include "I_BlockingState.h"

#include <sstream>

#ifndef BLOCKINGSTATE_H
#define BLOCKINGSTATE_H

using namespace gti;

namespace must
{

    /**
     * Variables controlling detection heuristic for potential
     * deadlocks that did not manifest.
     *
     * If I_OperationReordering has BLOCKING_STATE_MAX_QUEUE_SIZE
     * in its queue a detection is triggered. If no deadlock is detected
     * this is repeated whenever the total queue size grows by
     * BLOCKING_STATE_STEP_INCREASE.
     */
#define BLOCKING_STATE_MAX_QUEUE_SIZE 100000
#define BLOCKING_STATE_STEP_INCREASE      50000

    /**
     * Forward declaration of BlockingOp
     */
    class BlockingOp;

    /**
     * Information for the blocking state of a single rank.
     */
    class HeadInfo
    {
    public:

        HeadInfo ();
        ~HeadInfo ();

        std::list<MustRequestType> matchedReqs; /**<List of matched requests that have not been used yet for completions.*/
        std::list<MustRequestType> unexpectedCompletions; /**< List of requests that where reported as completed but had no notification of a match yet.*/
        bool hasCollCompletion; /**< True if the next collective for this rank was already reported as completed.*/
        bool hasSendCompletion; /**< True if a blocking send was reported as matched and not yet used (MPI_Send, MPI_Rsend, MPI_Ssend, send part of any of the Sendrecv calls).*/
        bool hasReceiveCompletion; /**< True if a blocking receive was reported as matched and not yet used (MPI_Recv, receive part of any of the Sendrecv calls).*/
        BlockingOp *primary, *secondary; /**< Primary and secondary operation that is blocked on this head.*/

#ifdef MUST_DEBUG
        int specialTime;
#endif
    };

    /**
     * Analysis of blocking calls with deadlock detection.
     *
     * We perform deadlock detection if:
     * - we get the last finalize to detect potential deadlocks that did not manifest
     * - we get a timeout from the placement driver, to detect actual deadlocks
     * - a heuristic warns us of operation queues exploding, this can happen for potential deadlocks that do not manifest
     */
    class BlockingState : public gti::ModuleBase<BlockingState, I_BlockingState>
    {
        friend class BlockingOp;
        friend class BlockingP2P;
        friend class BlockingColl;
        friend class BlockingCompletion;
        friend class BlockingRequestCompletion;

    public:

        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        BlockingState (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~BlockingState (void);

        /**
         * @see I_BlockingState::CollAll
         */
        GTI_ANALYSIS_RETURN CollAll (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly gti::MustCollCommType
                MustCommType comm,
                int isSend,
                int numTasks // counter for event aggregation
        );

        /**
         * @see I_BlockingState::CollRoot
         */
        GTI_ANALYSIS_RETURN CollRoot (
                MustParallelId pId,
                MustLocationId lId,
                int coll, // formerly gti::MustCollCommType
                MustCommType comm,
                int isSend,
                int root,
                int numTasks // counter for event aggregation
        ) ;

        /**
         * @see I_BlockingState::send
         */
        GTI_ANALYSIS_RETURN send (
                MustParallelId pId,
                MustLocationId lId,
                int dest) ;

       /**
         * @see I_BlockingState::srsend
         */
        GTI_ANALYSIS_RETURN srsend (
                MustParallelId pId,
                MustLocationId lId,
                int dest) ;

        /**
         * @see I_BlockingState::receive
         */
        GTI_ANALYSIS_RETURN receive (
                MustParallelId pId,
                MustLocationId lId,
                int source) ;

        /**
         * @see I_BlockingState::wait
         */
        GTI_ANALYSIS_RETURN wait (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request);

        /**
         * @see I_BlockingState::waitAny
         */
        GTI_ANALYSIS_RETURN waitAny (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType* requests,
                int count,
                int numProcNull);

        /**
         * @see I_BlockingState::waitAll
         */
        GTI_ANALYSIS_RETURN waitAll (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count,
                int numProcNull);

        /**
         * @see I_BlockingState::waitSome
         */
        GTI_ANALYSIS_RETURN waitSome (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count,
                int numProcNull);

        /**
         * @see I_BlockingState::completedRequest
         */
        GTI_ANALYSIS_RETURN completedRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request);

        /**
         * @see I_BlockingState::completedRequests
         */
        GTI_ANALYSIS_RETURN completedRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count);

        /**
         * @see I_BlockingState::notifyFinalize
         */
        GTI_ANALYSIS_RETURN notifyFinalize (
                    gti::I_ChannelId *thisChannel);

        /**
         * @see I_BlockingState::timeout
         */
        void timeout (void);

        /**
         * Notification of a new P2P match.
         * @see I_P2PMatchListener::newMatch.
         */
        void newMatch (
                        int sendRankWorld,
                        int receiveRankWorld,
                        bool sendHasRequest,
                        MustRequestType sendRequest,
                        bool receiveHasRequest,
                        MustRequestType receiveRequest,
                        must::MustSendMode sendMode);

        /**
         * Notification of a new collective tha was matched
         * completely.
         * @see I_CollMatchListener::newMatch.
         */
        void newMatch (
                        MustCollCommType collId,
                        I_Comm* comm);

        /**
         * @see I_BlockingState::checkpoint
         */
        void checkpoint (void);

        /**
         * @see I_BlockingState::rollback
         */
        void rollback (void);

    protected:
        ////Child modules
        I_ParallelIdAnalysis* myPIdMod;
        I_BaseConstants *myConsts;
        I_CreateMessage* myLogger;
        I_CommTrack* myCTrack;
        I_OperationReordering* myOrder;
        I_P2PMatch *myP2PMatch;
        I_CollectiveMatch *myCollMatch;
        I_LocationAnalysis *myLIdMod;
        I_RequestTrack* myRTrack;

        ////Wait-state data
        typedef std::vector<HeadInfo> HeadStates;
        HeadStates myHeads; /**< Information on blocking state of ranks.*/
        HeadStates myCheckpointHeads;

        ////Debugging
        typedef std::map<int, std::pair<MustParallelId, MustLocationId> > TimeSlice; /**< Rank to op completed on that op in this time step.*/
        typedef std::map<int, TimeSlice> BlockHistory; /**< Timestep to ops completed in that time step.*/
#ifdef MUST_DEBUG
        BlockHistory myHistory;
        BlockHistory myCheckpointHistory;
        int myTimeStep;
        int myCheckpointTimeStep;
#endif

        ////Finalize completion
        CompletionTree *myFinCompletion; /**< Used to determine when the last finalize call arrives at this place.*/
        CompletionTree *myCheckpointFinCompletion; /**< Used to determine when the last finalize call arrives at this place.*/

        /**
         * Initializes the heads used for tracking blocking state.
         */
        void initHeads (MustParallelId pId);
        void initHeads (int rank);

        /**
         * Handles a newly created operation.
         * @param rank of the issuing process,
         * @param newOp the operation.
         * @return true if successful.
         */
        bool handleNewOp (int rank, BlockingOp* newOp);

        /**
         * Applies a new P2P op to the current state.
         * Only done if its head is not blocked.
         * @param op to apply.
         * @return true iff successful.
         */
        bool applyNewP2POp (BlockingP2P* op);

        /**
         * Applies a new collective op to the current state.
         * Only done if its head is not blocked.
         * @param op to apply.
         * @return true iff successful.
         */
        bool applyNewCollectiveOp (BlockingColl* op);

        /**
         * Applies a new completion op to the current state.
         * Only done if its head is not blocked.
         * @param op to apply.
         * @return true iff successful.
         */
        bool applyNewCompletionOp (BlockingCompletion* op);

        /**
         * Applies a new completion update to the current state.
         * Only done if its head is not blocked.
         * @param op to apply.
         * @return true iff successful.
         */
        bool applyNewCompletionUpdateOp (BlockingRequestCompletion* op);

        /**
         * Removes the operations from the head and unblocks it.
         * @param rank of the operation.
         * @param head that completed a blocking op.
         * @return true iff successful.
         */
        bool completeHead (int rank, HeadInfo* head);

        /**
         * Adds the matched send or receive to the information for the head of the issuing rank.
         * @param isSend truee if P2P op was a send.
         * @param rank of the issuing process.
         * @param hasRequest true if the P2P op had an assocuatied request.
         * @param request associated with the op.
         */
        void newMatchedP2P (bool isSend, int rank, bool hasRequest, MustRequestType request);

        /**
         * Driver function for the actual deadlock detection, is used to
         * also decide suspension reasons if necessary and to perform
         * a full fledged deadlock exploration.
         * @return true if a deadlock was detected.
         */
        bool handleDeadlockDetection (bool enforceAValidSuspensionDecision);

        /**
         * Checks whether the current heads cause a deadlock.
         * If so it reports the deadlock.
         * @return true if we detected a deadlock (in this call or a previous call).
         */
        bool detectDeadlock (void);

        /**
         * Clears all heads and deletes their blocking state ops.
         * @param heads to clear.
         */
        void clearHeads (HeadStates *heads);

#ifdef MUST_DEBUG
        /**
         * Prints a history of the blocking ops.
         */
        void printHistoryAsDot (void);
#endif

#ifdef DOT
      /**
       * Writes a html file, containing the deadlock graphs
       * @param commOverview table of communicators to add to comm overview.
       */
        void generateDeadlockHtml (std::stringstream *commOverview);
#endif

        /**
         * Generates a communicator overview HTML table.
         * @param labels list of active communicators with their labels.
         * @param stream [out] to be filled with HTML table for deadlock overview.
         */
        void generateCommunicatorOverview (std::map<I_Comm*, std::string> *labels, std::stringstream *stream);

        /**
         * Generates a list of all active communicators for the given list of deadlocked processes.
         * @param deadlockedTasks list of deadlocked tasks.
         * @return list of active comms.
         */
        std::map<I_Comm*, std::string> generateActiveCommLabels (std::list<int> *deadlockedTasks);

        /**
         * Helper function to print a pid,lid pair as textual description (including stacktrace, if present).
         */
        void printLocation (MustParallelId pId, MustLocationId lId, std::stringstream &stream);

        /**
         * Generates a MUST_DeadlockMessageQueue.dot file that contains a reduced
         * message queue graph. Will use the given communicator labels to name communicators.
         * If any active and meaningful P2P message uses a further communicator, the
         * function adds it to commLabels.
         *
         * @param deadlockedTasks list of deadlock core tasks.
         * @param commLabels current labels.
         * @param refs either NULL or a pointer to a list to which the function appends (pId, lId)
         *                     pairs of any operation that appears in the message queue graph.
         */
        void generateReducedMessageQueueGraph (
                std::list<int> *deadlockedTasks,
                std::map<I_Comm*, std::string> *commLabels,
                std::list<std::pair<MustParallelId, MustLocationId> > *refs);

        /**
         * Generates a DOT file that represents a parallel call stack for the given list of (pid,lid) pairs.
         *
         * @param locations to include in the call stack.
         * @param outFileName for the DOT file.
         * @param includeMsgQSubGraphs true if sub graphs of the reduced message queue should be printed as leafs
         * @param deadlockedTasks (iff includeMsgQSubGraphs==true) list of all tasks that form the deadlock, otherwise NULL
         * @param commLabels (iff includeMsgQSubGraphs==true) labels for communicators, otherwise NULL
         */
        void generateParallelCallStackGraph (
                std::list<std::pair<MustParallelId, MustLocationId> > locations,
                std::string outFileName,
                bool includeMsgQSubGraphs,
                std::list<int> *deadlockedTasks,
                std::map<I_Comm*, std::string> *commLabels);

        /**
         * Prints a list of integers, prefers interval notation over individual values.
         * Format: X, Y, Z, ... where X,Y,Z... may be "A-B"
         * Designed for positive integers.
         *
         * @param out stream to put printed integers in.
         * @param ints list of integers to print.
         * @param isTagList iff true integers are checked for MPI_ANY_TAG and printed as such.
         */
        void printIntegerList (std::ostream &out, std::set<int> &ints, bool isTagList);

        /**
         * Generates a sub-graph of a recued message queue graph.
         *
         * @param deadlockedTasks task set of the deadlock.
         * @param commLabels labels for comms, may be augmented.
         * @param prefix for all nodes in this sub-graph.
         * @param fromTasks either empty list for full reduced message queue graph, or a
         *               list of tasks that specifies which tasks may have outgoing arcs in the sub-graph.
         * @param fromStack either NULL for full graph, or a call stack to restrict the operations in the
         *               sub-graph to.
         * @param refs either NULL or pointer to storage for a list of references to which the function
         *              adds any operation that the graph contains.
         * @out stream to ptint to.
         */
        void generatePartialReducedMessageQueueGraph (
                std::list<int> *deadlockedTasks,
                std::map<I_Comm*, std::string> *commLabels,
                std::string prefix,
                std::list<int> fromTasks,
                std::list<MustStackLevelInfo> *fromStack,
                std::list<std::pair<MustParallelId, MustLocationId> > *refs,
                std::ostream &out);

    }; /*BlockingState*/
} /*namespace must*/

#endif /*BLOCKINGSTATE_H*/
