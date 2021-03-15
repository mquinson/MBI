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
 * @file P2PMatch.h
 *       @see must::P2PMatch.
 *
 *  @date 26.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_CreateMessage.h"
#include "CompletionTree.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"
#include "I_DatatypeTrack.h"
#include "I_BaseConstants.h"
#include "I_Operation.h"
#include "I_LocationAnalysis.h"
#include "I_OperationReordering.h"
#include "I_FloodControl.h"
#include "P2POp.h"

#include "I_P2PMatch.h"

#ifndef P2PMATCH_H
#define P2PMATCH_H

using namespace gti;

namespace must
{
    /**
     * Forward declaration of P2POp, we have cyclic
     * dependencies between the matcher and the op.
     */
    class P2POp;

	/**
	 * Maps dests/sources (converted to respective world rank) to list of
	 * outstanding sends/recvs with this dest/source.
	 */
	typedef std::map<int, std::list<P2POp*> > RankOutstandings;

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
		std::list<P2POp*> wcRecvs; /**< List of receives that could not be enqueued at the moment, may also contain regular recvs (to any rank) if the first entry of this list is a wildcard receive.*/
	};

	/**
     * Implementation for I_LostMessage.
     */
    class P2PMatch : public gti::ModuleBase<P2PMatch, I_P2PMatch>
    {
        //Make friends with P2POp
        friend class P2POp;

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        P2PMatch (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~P2PMatch (void);

    		/**
    		 * @see I_P2PMatch::send
    		 */
    		GTI_ANALYSIS_RETURN send (
    				MustParallelId pId,
    				MustLocationId lId,
    				int dest,
    				int tag,
    				MustCommType comm,
    	            MustDatatypeType type,
    	            int count,
    	            int mode);

    		/**
    		 * @see I_P2PMatch::isend
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
    				MustRequestType request);

    		/**
    		 * @see I_P2PMatch::recv
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
    		 * @see I_P2PMatch::irecv
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
    		 * @see I_P2PMatch::recvUpdate
    		 */
    		GTI_ANALYSIS_RETURN recvUpdate (
    				MustParallelId pId,
    				MustLocationId lId,
    				int source);

    		/**
    		 * @see I_P2PMatch::irecvUpdate
    		 */
    		GTI_ANALYSIS_RETURN irecvUpdate (
    				MustParallelId pId,
    				MustLocationId lId,
    				int source,
    				MustRequestType request);

    		/**
    		 * @see I_P2PMatch::startPersistent
    		 */
    		GTI_ANALYSIS_RETURN startPersistent (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    		/**
    		 * @see I_P2PMatch::cancel
    		 */
    		GTI_ANALYSIS_RETURN cancel (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request);

    		/**
    		 * @see I_P2PMatch::printLostMessages
    		 */
    		GTI_ANALYSIS_RETURN printLostMessages (
    				I_ChannelId *thisChannel);

    		/**
    		 * @see I_P2PMatch::registerMatchCallback
    		 */
    		GTI_RETURN registerListener (I_P2PMatchListener *listener);

    		/**
    		 * @see I_P2PMatch::getP2PInfo
    		 */
    		bool getP2PInfo (int rank, MustRequestType request, P2PInfo* outInfo);

    		/**
    		 * @see I_P2PMatch::getP2PInfo
    		 */
    		bool getP2PInfo (int rank, bool isSend, P2PInfo* outInfo);

    		/**
    		 * @see I_P2PMatch::getP2PInfos
    		 */
    		std::list<P2PInfo> getP2PInfos (int fromRank, int toRank, bool sends, bool receives);

    		/**
    		 * @see I_P2PMatch::notifyDeadlock
    		 */
    		void notifyDeadlock (void);

    		/**
    		 * @see I_P2PMatch::checkpoint
    		 */
    		void checkpoint (void);

    		/**
    		 * @see I_P2PMatch::rollback
    		 */
    		void rollback (void);

    		/**
    		 * @see I_P2PMatch::decideSuspensionReason
    		 */
    		bool decideSuspensionReason (
    		            int decissionIndex,
    		            int *outNumAlternatives);

    		/**
    		 * @see I_P2PMatch::canOpBeProcessed
    		 */
    		bool canOpBeProcessed (
    		        MustParallelId pId,
    		        MustCommType comm,
    		        int sourceDest);

    		/**
    		 * @see I_P2PMatch::canOpBeProcessed
    		 */
    		bool canOpBeProcessed (
    		        MustParallelId pId,
    		        I_Comm* comm,
    		        int sourceDest);

    protected:
    		////Child modules
    		I_ParallelIdAnalysis* myPIdMod;
            I_LocationAnalysis* myLIdMod;
    		I_BaseConstants *myConsts;
    	    I_CreateMessage* myLogger;
    	    I_CommTrack* myCTrack;
    	    I_RequestTrack* myRTrack;
    	    I_DatatypeTrack* myDTrack;
    	    I_OperationReordering* myOrder;
    	    I_FloodControl* myFloodControl;

    	    ////Matching
    	    typedef std::map<I_CommPersistent*, ProcessTable> ProcessQueues; /**< Comm to table.*/
    	    typedef std::map <int, ProcessQueues> QT; /**< World rank to Queues.*/
    	    QT myQs; /**< My Matching structure .*/
    	    QT myCheckpointQs; /**< Copy of matching structure for checkpoint & restart*/

    	    /**
    	     * Thoughts on operation processing in suspension case:
    	     * Ops:
    	     * send  (pId, lId, dest,     tag, comm)
    	     * isend (pId, lId, dest,     tag, comm, request)
    	     * recv   (pId, lId, source, tag, comm)
    	     * irecv  (pId, lId, source, tag, comm, request)
    	     *
    	     * sendRecv -> regular send/recv handling
    	     * recvUpdate -> can be executed immediately, must include search within the suspended queue.
    	     * irecvUpdate -> can be executed immediately, must include search within the suspended queue.
    	     * startPersistent -> Calls isend/irecv
    	     * startPersistentArray -> Calls isend/irecv
    	     * cancel -> Not supported!
    	     */
    	    P2POp* mySuspendedForEntry; /**< The operation that needs to be update to continue operation (Copy, the receive is either in a wcRecvs entry of a table or in the queue of suspended operations (I_OperationReordering)).*/
    	    P2POp* myCheckpointSuspendedForEntry;

    	    /**
    	     * Structures that store information about enforced wild-card matching decisions.
    	     * These are checkpointed and rolled back if necessary.
    	     */
    	    std::map<int, std::list<MustRequestType> > myDecidedIrecvs; /**< Maps rank to  list of decided irecv requests.*/
    	    std::map<int, int> myDecidedRecvs; /**< Maps rank to count of recvs decided of this rank.*/
    	    std::map<int, std::list<MustRequestType> > myCheckpointDecidedIrecvs;
    	    std::map<int, int> myCheckpointDecidedRecvs;

    	    /**
    	     * This attribute points to the suspended wild-card receives of each rank.
    	     * It maps the rank to a list of operations that are currently in the queues
    	     * of I_OperationReorderings.
    	     *
    	     * It is used to speed up updating of suspended wc receives for scenarios
    	     * where large numbers of ops are suspended.
    	     * This need was discovered when running lref of 142.dmilc of Spec MPI2007 v2.0.
    	     */
    	    std::map<int, std::list<P2POp*> > suspendedWcRecvs;
    	    std::map<int, std::list<P2POp*> > myCheckpointSuspendedWcRecvs;

    	    /**
    	     * The following list is for watching whether certain ops had some match.
    	     * Used in the decideSuspensionReason function.
    	     *
    	     * The list is reset during checkpoint/rollback and considered in the
    	     * notifyMatch function.
    	     */
    	    std::list<P2POp*> myOpsToWatchForMatch;

    	    //// Printing lost messages
    	    CompletionTree *myFinCompletion; /**< Used to determine when the last finalize call arrives at this place.*/
    	    CompletionTree *myCheckpointFinCompletion;

    	    //// Debugging
#ifdef MUST_DEBUG
    	    typedef std::map<int, std::map<int, std::list<int> > > MatchHistory;
    	    typedef std::map<I_CommPersistent*,  MatchHistory> AllMatchHistories;
    	    AllMatchHistories myMatches;//comm->sender->receiver->list of tags (sender & receiver in MPI_COMM_WORLD)
    	    AllMatchHistories myCheckpointMatches;//comm->sender->receiver->list of tags (sender & receiver in MPI_COMM_WORLD)
#endif

    	    //// Listener
    	    std::list<I_P2PMatchListener*> myListeners;

    	    bool myPrintLostMessages; /**< True if lost messages should be printed, false otherwise. I.e. if we have a deadlock we do not want to print lost messages. */
    	    bool myCheckpointPrintLostMessages;

    	    /**
    	     * Helper for common preparation of operations.
    	     * Queries for the persistent datatype and communicator informations
    	     * to use for a recv/send and translates a source/dest rank into
    	     * MPI_COMM_WORLD.
    	     *
    	     * @param pId context.
    	     * @param comm of the send/recv.
    	     * @param rankIn dest/source of the send/recv.
    	     * @param type datatype of the send/recv.
    	     * @param pOutComm persistent info of comm.
    	     * @param pOutTranslatedRank dest/source translated into MPI_COMM_WORLD.
    	     * @param pOutType persistent datatype.
    	     * @return true if successfull, false otherwise (persistent handles will already be erased in that case).
    	     */
    	    bool getCommTranslationAndType (
    	            MustParallelId pId,
    	            MustCommType comm,
    	            int rankIn,
    	            MustDatatypeType type,
    	            I_CommPersistent**pOutComm,
    	            int *pOutTranslatedRank,
    	            I_DatatypePersistent**pOutType);

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
    	    void handleNewOp (int rank, P2POp* op);

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
    	     * @param from rank calling the send.
    	     * @param dest rank to send to.
    	     * @param tag of the send.
    	     * @param comm of the send.
    	     * @param pOutNeedsToSuspend pointer to storage for bool, set to true if a suspension needs to be started and this send needs to be queued for later processing.
    	     * @return true if a match was found, false if no match was found or a suspension is necessary.
    	     */
    	    bool findMatchingRecv (
    	    		P2POp* op,
    	    		bool *pOutNeedsToSuspend);

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
    	        P2POp* op);

    	    /**
    	     * Searches the queues for a send that matches the given recv info.
    	     * @param from rank calling the receive.
    	     * @param source rank to receive from.
    	     * @param tag of the receive.
    	     * @param comm of the receive.
    	     * @param pOutNeedsToSuspend pointer to storage for bool, set to true if a suspension needs to be started and this receive needs to be queued for later processing.
    	     * @return true if a match was found, false if no match was found or a suspension is necessary.
    	     */
    	    bool findMatchingSend (
    	        P2POp* op,
    	    		bool *pOutNeedsToSuspend);

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
    	            P2POp* op);

    	    /**
    	     * Searches the tables and afterwards the suspension queue
    	     * for the wildcard receive to update.
    	     *
    	     * @param from rank that called the receive.
    	     * @param newSource the new source for the receive.
    	     * @param hasRequest true if this was a non-blocking receive.
    	     * @param request value if hasRequest == true, otherwise not evaluated.
    	     */
    	    void findRecvForUpdate (
    	            int from,
    	            int newSource,
    	            bool hasRequest,
    	            MustRequestType request,
    	            bool *pOutUnsuspend);

    	    /**
    	     * Notifies all listeners of the new match between a and b.
    	     */
    	    void notifyMatch (P2POp* send, P2POp *recv);

    	    /**
    	     * Filles the given info with information from the given op.
    	     * @param op  to fill with.
    	     * @param info to fill into.
    	     */
    	    void fillInfo (P2POp* op, P2PInfo *info);

    	    /**
    	     * Deletes the given matching queues.
    	     */
    	    void clearQ (QT* queue);

#ifdef MUST_DEBUG
    	    void addMatchToHistory (P2POp* send, P2POp* recv);
    	    void printMatchHistories (void);
    	    void clearMatchHistory(AllMatchHistories *history);
    	    void copyMatchHistory(AllMatchHistories *from, AllMatchHistories *to);
#endif
    };
} /*namespace MUST*/

#endif /*P2PMATCH_H*/
