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
 * @file DDP2PMatch.h
 *       @see must::DDP2PMatch.
 *
 *  @date 20.01.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#include "GtiEnums.h"
#include "ModuleBase.h"
#include "MustTypes.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"
#include "I_CreateMessage.h"
#include "CompletionTree.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"
#include "I_DatatypeTrack.h"
#include "I_BaseConstants.h"
#include "I_FloodControl.h"
#include "I_Profiler.h"
#include "DistributedDeadlockApi.h"
#include "DP2POp.h"

#include "I_DP2PMatch.h"

#ifndef DP2PMATCH_H
#define DP2PMATCH_H

using namespace gti;

namespace must
{
    /**
     * Forward declaration of DP2POp, we have cyclic
     * dependencies between the matcher and the op.
     */
    class DP2POp;

	/**
	 * Maps dests/sources (converted to respective world rank) to list of
	 * outstanding sends/recvs with this dest/source.
	 */
	typedef std::map<int, std::list<DP2POp*> > RankOutstandings;

	/**
	 * The send and receive tables for a rank (in comm world) and a certain
	 * communicator. This contains all outstanding (unmatched)
	 * sends and receives.
	 *
	 *  Important:
	 * - "recvs" have priority over "wcRecvs", so they are processed
	 *    first if a match partner is available.
	 * - If there is at least one entry in "wcRecvs", new outstanding recvs
	 *    are put into the "wcRecvs" list (to preserve order)
	 * - If the first enty of "wcRecvs" is removed, all successive entries
	 *    that have no MPI_ANY_SOURCE as rank are added to the
	 *    respective entry in "recvs". This is done until the list is empty
	 *    or the first element is a wildcard receive
	 * - When searching for an outstanding receive that matches with
	 *    a new send, it is necessary to search "recvs" first, and if no
	 *    partner was found "wcRecvs" afterwards
	 */
	class ProcessTable
	{
	public:
		RankOutstandings sends; /**< Maps destination of outstanding send to list of sends with this destination.*/
		RankOutstandings recvs; /**< Maps source of outstanding recv to list of recvs with this source.*/
		std::list<DP2POp*> wcRecvs; /**< List of receives that could not be enqueued at the moment, may also contain regular recvs (to any rank) if the first entry of this list is a wildcard receive.*/
	};

	/**
	 * Suspension information for a single rank.
	 */
	class SuspensionInfo
	{
	public:
	    SuspensionInfo (void);

	    bool isSuspended;
	    DP2POp* suspensionReason;
	    std::list<DP2POp*> furtherReasons; /**< Additional suspension reasons, now there may be multiple of these, first one is handled in "suspensionReason".*/
	    std::list<DP2POp*> queue;

	    GTI_STRATEGY_TYPE direction; //Information on the communication channel that provides P2P ops for this rank.
	    unsigned int channel;

	    /**
	     * Adds the given reason to the list of reasons, checks for
	     * and avoids duplication.
	     *
	     * @param reason to add
	     * @return true if the reason was added and was not yet present, false otherwise.
	     */
	    bool addReason (DP2POp* reason);

	    /**
	     * Removes a reason.
	     *
	     * If this reason was the one stored in "suspensionReason"
	     * the first element in the furtherReasons (if available) is
	     * stored in "suspensionReason".
	     *
	     * With that a check on "suspensionReason == NULL" can
	     * always indicate whether a reason reamains.
	     *
	     * @param reason to remove.
	     * @param true if this reason was indead a suspension reason and was thus removed, false otherwise.
	     */
	    bool removeReason (DP2POp* reason);
	};

	/**
	 * Info structure that holds all we need to pass information on a send operation
	 * to a sister node.
	 */
	class PassSendInfo
	{
	public:
	    MustParallelId pId;
	    MustLocationId lId;
	    int destIn;
	    int tag;
	    MustCommType comm;
	    MustDatatypeType type;
	    int count;
	    int mode;
	    int targetPlace;
	    bool hasRequest;
	    bool isPersistent;
	    MustRequestType request;
	    I_CommPersistent* cInfo;
	    I_DatatypePersistent *dInfo;
	};

	/**
     * Implementation for I_DP2PMatch.
     */
    class DP2PMatch : public gti::ModuleBase<DP2PMatch, I_DP2PMatch>
    {
        //Make friends with DP2POp
        friend class DP2POp;

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DP2PMatch (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~DP2PMatch (void);

    		/**
    		 * @see I_DP2PMatch::init
    		 */
    		GTI_ANALYSIS_RETURN init (
    		        MustParallelId pId
    		);

    		/**
    		 * @see I_DP2PMatch::send
    		 */
    		GTI_ANALYSIS_RETURN send (
    				MustParallelId pId,
    				MustLocationId lId,
    				int dest,
    				int tag,
    				MustCommType comm,
    	            MustDatatypeType type,
    	            int count,
    	            int mode,
    	            MustLTimeStamp ts);

    		/**
    		 * @see I_DP2PMatch::isend
    		 */
    		GTI_ANALYSIS_RETURN isend (
    				MustParallelId pId,
    				MustLocationId lId,
    				int dest,
    				int tag,
    				MustCommType comm,
    	            MustDatatypeType type,
    	            int count,
    	            int mode,
    				MustRequestType request,
    	            MustLTimeStamp ts);

    		/**
    		 * @see I_DP2PMatch::recv
    		 */
    		GTI_ANALYSIS_RETURN recv (
    				MustParallelId pId,
    				MustLocationId lId,
    				int source,
    				int tag,
    				MustCommType comm,
    	            MustDatatypeType type,
    	            int count);

    		/**
    		 * @see I_DP2PMatch::irecv
    		 */
    		GTI_ANALYSIS_RETURN irecv (
    				MustParallelId pId,
    				MustLocationId lId,
    				int source,
    				int tag,
    				MustCommType comm,
    	            MustDatatypeType type,
    	            int count,
    				MustRequestType request);

    		/**
    		 * @see I_DP2PMatch::recvUpdate
    		 */
    		GTI_ANALYSIS_RETURN recvUpdate (
    				MustParallelId pId,
    				MustLocationId lId,
    				int source);

    		/**
    		 * @see I_DP2PMatch::irecvUpdate
    		 */
    		GTI_ANALYSIS_RETURN irecvUpdate (
    				MustParallelId pId,
    				MustLocationId lId,
    				int source,
    				MustRequestType request);

    		/**
    		 * @see I_DP2PMatch::startPersistent
    		 */
    		GTI_ANALYSIS_RETURN startPersistent (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request,
    	            MustLTimeStamp ts);

    		/**
    		 * @see I_DP2PMatch::cancel
    		 */
    		GTI_ANALYSIS_RETURN cancel (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    		/**
    		 * @see I_DP2PMatch::printLostMessages
    		 */
    		GTI_ANALYSIS_RETURN printLostMessages (
    				I_ChannelId *thisChannel);

    		/**
    		 * @see I_DP2PMatch::registerListener
    		 */
    		void registerListener (I_DP2PListener *listener);

    		/**
    		 * @see I_DP2PMatch::isWorldRankSuspended
    		 */
    		bool isWorldRankSuspended (int worldRank);

    		/**
    		 * @see I_DP2PMatch::notifySendActivated
    		 */
    		void notifySendActivated (int rank, MustLTimeStamp ts);

    protected:
    		////Own place ID
    		int myPlaceId;

    		////Child modules
    		I_ParallelIdAnalysis* myPIdMod;
    		I_LocationAnalysis* myLIdMod;
    		I_BaseConstants *myConsts;
    	    I_CreateMessage* myLogger;
    	    I_CommTrack* myCTrack;
    	    I_RequestTrack* myRTrack;
    	    I_DatatypeTrack* myDTrack;
    	    I_FloodControl *myFloodControl;
    	    I_Profiler *myProfiler;

    	    ////Listener
    	    I_DP2PListener* myListener;

    	    ////Matching
    	    typedef std::map<I_CommPersistent*, ProcessTable> ProcessQueues; /**< Comm to table.*/
    	    typedef std::map <int, ProcessQueues> QT; /**< World rank to Queues.*/
    	    QT myQs; /**< My Matching structure .*/
    	    uint64_t myQSize;
    	    uint64_t myMaxQSize;
    	    std::map<std::pair<int, MustLTimeStamp>, PassSendInfo > mySendsToTransfer; /**< Stores send operations that we still need to pass (we may not pass them immediately to sister nodes if DWS tells us they are not active yet).*/

    	    ////Function pointers for wrap-across
    	    passSendForMatchingP myPassSendFunction;
    	    passIsendForMatchingP myPassIsendFunction;
    	    passSendStartForMatchingP myPassSendStartFunction;

    	    ////Suspension
    	    std::map<int, SuspensionInfo> mySuspension; /**< Map with suspension state for each rank, vector would be faster, but makes dynamic size harder.*/
    	    bool myIsInProcessQueues; /**< True if we are processing queued events right now, influences suspendOp.*/

    	    //// Debugging
#ifdef MUST_DEBUG
    	    typedef std::map<int, std::map<int, std::list<int> > > MatchHistory;
    	    typedef std::map<I_CommPersistent*,  MatchHistory> AllMatchHistories;
    	    AllMatchHistories myMatches;//comm->sender->receiver->list of tags (sender & receiver in MPI_COMM_WORLD)
#endif

    	    /**
    	     * Helper for common preparation of operations.
    	     * Queries for the persistent datatype and communicator informations
    	     * to use for a recv/send and translates a source/dest rank into
    	     * MPI_COMM_WORLD.
    	     *
    	     * @param rank from the subsequently supplie pid
    	     * @param pId context.
    	     * @param comm of the send/recv.
    	     * @param rankIn dest/source of the send/recv.
    	     * @param type datatype of the send/recv.
    	     * @param pOutComm persistent info of comm.
    	     * @param pOutTranslatedRank dest/source translated into MPI_COMM_WORLD.
    	     * @param pOutType persistent datatype.
    	     * @param isFromRemotePlace true if this is a send from a remote place.
    	     * @return true if successfull, false otherwise (persistent handles will already be erased in that case).
    	     */
    	    bool getCommTranslationAndType (
    	            int rank,
    	            MustParallelId pId,
    	            MustCommType comm,
    	            int rankIn,
    	            MustDatatypeType type,
    	            I_CommPersistent**pOutComm,
    	            int *pOutTranslatedRank,
    	            I_DatatypePersistent**pOutType,
    	            bool isFromRemotePlace);

    	    /**
    	     * Translates the given destination or source into a
    	     * rank in MPI_COMM_WORLD by using the given
    	     * communicator. If it is a source and also MPI_ANY_SOURCE
    	     * the function returns MPI_ANY_SOURCE.
    	     */
    	    int translateDestSource (I_Comm* comm, int destSource);

    	    /**
    	     * Translates the given destination or source which is a rank
    	     * in MPI_COMM_WORLD into its respective rank in the given communicator.
    	     */
    	    int invTranslateDestSource (I_Comm* comm, int destSource);

    	    /**
    	     * Translates the given issuing rank
    	     * in MPI_COMM_WORLD into its respective rank in the given communicator.
    	     */
    	    int invTranslateIssuingRank (I_Comm* comm, int destSource);

    	    /**
    	     * Handles the new operation from the given rank.
    	     * Either starts its processing (if possible) or
    	     * queues it with I_OperationReordering.
    	     * @param rank that executes the op.
    	     * @param op new operation.
    	     */
    	    void handleNewOp (int rank, DP2POp* op);

    	    /**
    	     * Internal function to perform the printing when the last
    	     * rank finished.
    	     */
    	    GTI_ANALYSIS_RETURN printLostMessages (void);

    	    /**
    	     * Prints all information in the queues.
    	     * (For Debugging)
    	     */
    	    void printQs (void);

    	    /**
    	     * Searches the queues for a recv that matches the given send info.
    	     * @param op to find a matching receive for.
    	     * @param pOutNeedsToSuspend pointer to storage for bool, set to true if a suspension needs to be started and this send needs to be queued for later processing.
    	     * @param pOutReason op that is the suspension reason (output).
    	     * @return true if a match was found, false if no match was found or a suspension is necessary.
    	     */
    	    bool findMatchingRecv (
    	    		DP2POp* op,
    	    		bool *pOutNeedsToSuspend,
    	    		DP2POp** pOutReason);

    	    /**
    	     * Adds an unmatched send to the Q.
    	     *
    	     * @param from rank that called the send.
    	     * @param dest rank to send to.
		 * @param tag of the send.
		 * @param comm of the send.
         * @param hasRequest true if there is a request associated with the send.
         * @param request that is associated, only evaluated it hasRequest == true.
    	     */
    	    void addOutstandingSend (
    	        DP2POp* op);

    	    /**
    	     * Searches the queues for a send that matches the given recv info.
    	     * @param op to find a matching send for
    	     * @param pOutNeedsToSuspend pointer to storage for bool, set to true if a suspension needs to be started and this receive needs to be queued for later processing.
    	     * @param pOutReason op that is the suspension reason (output).
    	     * @return true if a match was found, false if no match was found or a suspension is necessary.
    	     */
    	    bool findMatchingSend (
    	        DP2POp* op,
    	    		bool *pOutNeedsToSuspend,
    	    		DP2POp** pOutReason);

    	    /**
    	     * Adds an unmatched receive to the Q.
    	     *
    	     * @param from rank that called the receive.
    	     * @param source rank to receive from.
    	     * @param tag of the receive.
    	     * @param comm of the receive.
    	     * @param hasRequest true if there is a request associated with the receive.
    	     * @param request that is associated, only evaluated it hasRequest == true.
    	     */
    	    void addOutstandingRecv (
    	            DP2POp* op);

    	    /**
    	     * Searches the tables and afterwards the suspension queue
    	     * for the wildcard receive to update.
    	     *
    	     * @param from rank that called the receive.
    	     * @param newSource the new source for the receive.
    	     * @param hasRequest true if this was a non-blocking receive.
    	     * @param request value if hasRequest == true, otherwise not evaluated.
    	     * @param pOutReopenedRanks List of ranks with non-empty suspension queues that where unsuspended
    	     */
    	    void findRecvForUpdate (
    	            int from,
    	            int newSource,
    	            bool hasRequest,
    	            MustRequestType request,
    	            std::list<int>* pOutReopenedRanks);

    	    /**
    	     * Deletes the given matching queues.
    	     */
    	    void clearQ (QT* queue);

    	    /**
    	     * Suspends the given operation, if necessary it also
    	     * updates the suspension state for this ops rank.
    	     * I.e. if it was not suspended this rank will be set
    	     * as suspended and the given reason is set.
    	     * Finally, the operation is queued in this ranks
    	     * suspension queue (Only if myIsInProcessQueues
    	     * indicates that we aren't processing queued ops,
    	     * as in that case the op is already in the queue).
    	     */
    	    void suspendOp (DP2POp* opToSuspend, DP2POp* suspensionReason);

    	    /**
    	     * Helper function to remove a suspension reason from a rank.
    	     * If this reason was indeed a suspension reason for this rank,
    	     * it also removed this reason from all other ranks.
    	     *
    	     * @param rank of the rank from which to remove the reason.
    	     * @param reason to remove.
    	     * @param list of all reopeneed ranks that had a non empty queue as a result of the reason removal.
    	     */
    	    void removeReasonFromRank (int rank, DP2POp *reason, std::list<int>* pOutReopenedRanks);

    	    /**
    	     * Processes events that are queues in the suspension state for the given
    	     * list of ranks. Evaluates them until no more events are available or until
    	     * a suspension occured for all ranks.
    	     *
    	     * @param ranksToProcess pointer to storage with list of ranks to check queues.
    	     */
    	    void processQueuedEvents (std::list<int>* ranksToProcess);

#ifdef MUST_DEBUG
    	    void addMatchToHistory (DP2POp* send, DP2POp* recv);
    	    void printMatchHistories (void);
    	    void clearMatchHistory(AllMatchHistories *history);
    	    void copyMatchHistory(AllMatchHistories *from, AllMatchHistories *to);
#endif
    };
} /*namespace MUST*/

#endif /*DP2PMATCH_H*/
