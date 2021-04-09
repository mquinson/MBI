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
 * @file DWaitState.h
 *       @see DWaitState.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"
#include "I_LocationAnalysis.h"
#include "I_BaseConstants.h"
#include "I_DP2PMatch.h"
#include "I_DCollectiveMatchReduction.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"
#include "I_FloodControl.h"
#include "I_Profiler.h"
#include "DistributedDeadlockApi.h"
#include "GtiApi.h"

#include "QOp.h"
#include "QOpCompletion.h"
#include "QOpCommunication.h"
#include "QOpCommunicationColl.h"
#include "QOpCommunicationCollNonBlocking.h"
#include "QOpCommunicationP2P.h"
#include "QOpCommunicationP2PNonBlocking.h"

#include "I_DWaitState.h"
#include "I_DP2PListener.h"
#include "I_DCollectiveListener.h"

#include <string>
#include <vector>

#ifndef DWAITSTATE_H
#define DWAITSTATE_H

using namespace gti;

namespace must
{
    /**
     * Helper class to store information on a head.
     */
    class DHeadInfo
    {
    public:
        DHeadInfo (void);
        ~DHeadInfo (void);

        std::map<MustLTimeStamp, QOp*> trace; /**< Trace of operations that we got information about.*/
        MustLTimeStamp activeTS; /**< Timestamp of currently active op in trace.*/
        MustLTimeStamp nextTS; /**< Next timestamp to use when a new op arrives.*/
        std::map<MustRequestType, std::list<QOpCommunication*> > uncompletedNBOps; /**< Maps requests to lists of uncompleted ops, used to associate non blocking ops with completions.*/
        bool wasDecremented; /**< True if we decremented the activeTS to enforce that a head stays empty during a syncronization for a consistent state.*/

        /**
         * Retrieves the next timestamp and increments it.
         */
        MustLTimeStamp getAndIncNextTS (void);
    };

	/**
     * Template for correctness checks interface implementation.
     */
    class DWaitState
        : public gti::ModuleBase<DWaitState, I_DWaitState>,
          public I_DP2PListener,
          public I_DCollectiveListener
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DWaitState (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
        virtual ~DWaitState (void);

        /*======================================
         *    I_DWaitState
         * ======================================*/

        /**
         * @see I_DWaitState::wait
         */
        GTI_ANALYSIS_RETURN wait (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request);

        /**
         * @see I_DWaitState::waitAny
         */
        GTI_ANALYSIS_RETURN waitAny (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType* requests,
                int count,
                int numProcNull);

        /**
         * @see I_DWaitState::waitAll
         */
        GTI_ANALYSIS_RETURN waitAll (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count,
                int numProcNull);

        /**
         * @see I_DWaitState::waitSome
         */
        GTI_ANALYSIS_RETURN waitSome (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count,
                int numProcNull);

        /**
         * @see I_DWaitState::completedRequest
         */
        GTI_ANALYSIS_RETURN completedRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request);

        /**
         * @see I_DWaitState::completedRequests
         */
        GTI_ANALYSIS_RETURN completedRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count);

        /**
         * @see I_DWaitState::collectiveAcknowledge
         */
        GTI_ANALYSIS_RETURN collectiveAcknowledge (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize);

        /**
         * @see I_DWaitState::receiveActiveRequest
         */
        GTI_ANALYSIS_RETURN receiveActiveRequest (
                int sendRank,
                MustLTimeStamp sendLTS,
                MustLTimeStamp receiveLTS);

        /**
         * @see I_DWaitState::receiveActiveAcknowledge
         */
        GTI_ANALYSIS_RETURN receiveActiveAcknowledge (
                int receiveRank,
                MustLTimeStamp receiveLTS);

        /**
         * @see I_DWaitState::requestWaitForInfos
         */
        GTI_ANALYSIS_RETURN requestWaitForInfos (void);

        /**
         * @see I_DWaitState::requestConsistentState
         */
        GTI_ANALYSIS_RETURN requestConsistentState (void);

        /**
         * @see I_DWaitState::handlePing
         */
        GTI_ANALYSIS_RETURN handlePing (int fromNode, int pingsRemaining);

        /**
         * @see I_DWaitState::handlePong
         */
        GTI_ANALYSIS_RETURN handlePong (int fromNode, int pingsRemaining);

        /*======================================
         *    I_DP2PListener
         * ======================================*/

    		/**
    		 * @see I_DP2PListener::newP2POp
    		 */
    		MustLTimeStamp newP2POp (
    		        MustParallelId pId,
    		        MustLocationId lId,
    		        I_CommPersistent *comm,
    		        bool isSend,
    		        int sourceTarget,
    		        bool isWc,
    		        MustSendMode mode,
    		        int tag,
    		        bool hasRequest,
    		        MustRequestType request,
    		        bool *outIsActive
    		);

    		/**
    		 * @see I_DP2PListener::notifyP2PRecvMatch
    		 */
    		void notifyP2PRecvMatch (
    		        MustParallelId pIdRecv,
    		        MustLTimeStamp recvTS,
    		        MustParallelId pIdSend,
    		        MustLTimeStamp sendTS
    		);

    		/*======================================
    		 *    I_DCollectiveListener
    		 * ======================================*/

    		/**
    		 * @see I_DCollectiveListener::newCollectiveOp
    		 */
    		MustLTimeStamp newCollectiveOp (
    		        MustParallelId pId,
    		        MustLocationId lId,
    		        I_CommPersistent *comm,
    		        MustCollCommType collType,
    		        MustLTimeStamp waveNumberInComm,
                    bool hasRequest,
                    MustRequestType request
    		);

    		/**
    		 * @see I_DCollectiveListener::notifyCollectiveLocalComplete
    		 */
    		void notifyCollectiveLocalComplete (
    		        std::list<std::pair<MustParallelId, MustLTimeStamp> > &ops
    		);

    		/*======================================
    		 *   (Unnamed Interface) For QOp implementations
    		 * ======================================*/

    		/**
    		 * Returns a pointer to the parallel ID analysis.
    		 */
    		I_ParallelIdAnalysis* getParallelIdAnalysis (void);

    		/**
    		 * Returns a pointer to the location ID analysis
    		 */
    		I_LocationAnalysis* getLocationlIdAnalysis (void);

    		/**
    		 * Searches for the non-blocking operation associated with the given
    		 * request for the specified pId.
    		 * It searches the youngest operation with this request, as this should
    		 * due to order criterion be the right one (assuming we call this immediately
    		 * when we get notification of a completion call.
    		 *
    		 * If this returns NULL the request has no associated op, e.g., it is NULL,
    		 * invalid, inactive, or the communication targeted MPI_PROC_NULL (DP2POp
    		 * drops these communications).
    		 *
    		 * Increments the reference count of the found op, the caller needs to call
    		 * erase if this returns an op.
    		 *
    		 * @param pId for op to find.
    		 * @param request associated with op to find.
    		 * @return pointer to op if found, NULL otherwise.
    		 */
    		QOpCommunication* getNonBlockingOpForRequest(
    		        MustParallelId pId,
    		        MustRequestType request);

    		/**
    		 * Returns the TBON node in this layer that will receive information on
    		 * the given rank, i.e., that has the given rank as ancestor.
    		 * @param rank to get node for.
    		 * @param outIsThisNode pointer to storage for a bool or NULL, if not
    		 *               NULL it is set to true if the returned node is this very node
    		 *               itself and to false otherwise.
    		 * @return TBON node id.
    		 */
    		int getNodeForWorldRank (int rank, bool *outIsThisNode);

    		/**
    		 * Returns true if the given tag is the constant MPI_ANY_TAG and
    		 * false otherwise.
    		 * @param tag to check.
    		 * @return result.
    		 */
    		bool isMpiAnyTag (int tag);

    		/**
    		 * Returns function used to generate a CollectiveActiveRequest.
    		 */
    		generateCollectiveActiveRequestP getCollRequestFunction (void);

    		/**
    		 * Returns function used to generate a ReceiveActiveRequest.
    		 */
    		generateReceiveActiveRequestP getReceiveActiveRequestFunction (void);

    		/**
    		 * Returns function used to generate a ReceiveActiveAcknowledge.
    		 */
    		generateReceiveActiveAcknowledgeP getReceiveActiveAcknowledgeFunction (void);

    		/**
    		 * Returns function used to provide wait for information for a rank or one of its sub-nodes.
    		 */
    		provideWaitForInfosSingleP getProvideWaitSingleFunction (void);

    		/**
    		 * Returns function used to provide wait for information of a rank with AND-OR semantic.
    		 */
    		provideWaitForInfosMixedP getProvideWaitMultiFunction (void);

    		/**
    		 * Returns function used to provide wait for information of a rank blocked in a collective
    		 */
    		provideWaitForInfosCollP getProvideWaitCollFunction (void);

    		/**
    		 * Returns function used to provide wait for information of a rank blocked in an NBC operation
    		 */
    		provideWaitForInfosNbcCollP getProvideWaitNbcCollFunction (void);

    		/**
    		 * Returns function used to provide matching (background) information for active and uncompleted NBC ops
    		 */
    		provideWaitForNbcBackgroundP getProvideWaitNbcBackgroundFunction (void);

    protected:

    		/*======================================
    		 *   Internals
    		 * ======================================*/

    		I_ParallelIdAnalysis* myPIdMod;
    	    I_CreateMessage* myLogger;
    	    I_ArgumentAnalysis* myArgMod;
    	    I_LocationAnalysis* myLocations;
    	    I_BaseConstants* myConstants;
    	    I_DP2PMatch* myDP2P;
    	    I_DCollectiveMatchReduction* myDCollMatch;
    	    I_CommTrack* myCommTrack;
    	    I_RequestTrack* myRequestTrack;
    	    I_FloodControl* myFloodControl;
    	    I_Profiler *myProfiler;

    	    std::vector<DHeadInfo> myHeads;
    	    int myFirstWorldRank; /**< myHeads[0] represents this rank.*/
    	    int myNodeId;
    	    uint64_t myTraceSize;
    	    uint64_t myMaxTraceSize;

    	    generateCollectiveActiveRequestP myFCollRequest;
    	    generateReceiveActiveRequestP myFReceiveActiveRequest;
    	    generateReceiveActiveAcknowledgeP myFReceiveActiveAcknowledge;
    	    provideWaitForInfosEmptyP myFProvideWaitEmpty;
    	    provideWaitForInfosSingleP myFProvideWaitSingle;
    	    provideWaitForInfosMixedP myFProvideWaitMulti;
    	    provideWaitForInfosCollP myFProvideWaitColl;
    	    provideWaitForInfosNbcCollP myFProvideWaitNbcColl;
    	    provideWaitForNbcBackgroundP myFProvideWaitNbcBackground;
    	    pingDWaitStateP myFPing;
    	    pongDWaitStateP myFPong;
    	    acknowledgeConsistentStateP myFPAcknowledgeConsistent;
    	    gtiBreakRequestP myFPBreakRequest;
    	    gtiBreakConsumeP myFPBreakConsume;

    	    bool myStopTime;
    	    int myNumOutstandingPingPongs;
    	    bool myGotEarlyCStateRequest;

    	    bool myVotedForBreak;
    	    bool myReadEnvs;
    	    unsigned long myThresholdBreak;
    	    unsigned long myThresholdResume;

    	    /**
    	     * Initialized the heads.
    	     */
    	    void initHeads (MustParallelId pId);
    	    void initHeads (int rank);

    	    /**
    	     * Prints the current configurations as a dot file
    	     * @param prefix for the file name to use, the function adds
    	     *               "_<FIRST_WORLD_RANK>.dot" automatically.
    	     */
    	    void printHeadsAsDot (std::string prefix);

    	    /**
    	     * Returns the rank and head for this parallel id.
    	     * Also initializes heads if necessary.
    	     *
    	     * @param pId to retrieve head for.
    	     * @param outRank storage to store rank in or NULL.
    	     * @param outPHead storage to store pointer to head in or NULL.
    	     * @return true if this was a valid pId, false otherwise.
    	     */
    	    bool getRankAndHead (MustParallelId pId, int *outRank, DHeadInfo **outPHead);
    	    bool getRankAndHead (int rank, int *outRank, DHeadInfo **outPHead);

    	    /**
    	     * Checks whether some criterion has arrived to advance the given op
    	     * or to let it perform some action (e.g., to issue a request).
    	     *
    	     * Afterwards it also checks progress for the heads current operation
    	     * (if it is not the given op). I.e., if advancing a non-blocking op
    	     * allows a completion to advance.
    	     *
    	     * If op is NULL it only checks the current op.
    	     *
    	     * @param op to advance.
    	     * @param head associated with op.
    	     */
    	    void advanceOp (QOp* op, DHeadInfo *head);

    	    /**
    	     * Generates a list of labels for our communicators.
    	     * @return list of labels.
    	     */
    	    std::map<I_Comm*, std::string> generateActiveCommLabels (void);

    	    /**
    	     * Helper to inject requests for creating/lifting a break.
    	     */
    	    inline void checkForBreakConsumeRequest (int newTraceSize);
    };
} /*namespace MUST*/

#endif /*DWAITSTATE_H*/
