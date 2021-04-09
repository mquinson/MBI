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
 * @file DCollectiveMatch.h
 *       @see must::DCollectiveMatch.
 *
 *  @date 24.04.2012
 *  @author Fabian Haensel, Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */


#ifndef DCOLLECTIVEMATCH_H
#define DCOLLECTIVEMATCH_H

#include "ModuleBase.h"
#include "GtiTypes.h"
#include "GtiApi.h"
#include "MustTypes.h"
#include "CompletionTree.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"
#include "I_CommTrack.h"
#include "I_OpTrack.h"
#include "I_DatatypeTrack.h"
#include "I_BaseConstants.h"
#include "I_CreateMessage.h"
#include "I_Reduction.h"
#include "CollectiveConditionApi.h"
#include "DCollectiveOp.h"
#include "I_DCollectiveOpProcessor.h"
#include "DCollectiveCommInfo.h"
#include "DistributedDeadlockApi.h"
#include "I_DCollectiveListener.h"
#include "I_CollCommListener.h"

using namespace gti;

namespace must
{
    /**
     * Base class for distributed collective matching and verification.
     */
    template <class INSTANCE, class BASE>
	class DCollectiveMatch : public gti::ModuleBase <INSTANCE, BASE>, public I_DCollectiveOpProcessor
	{
	protected:
		std::set<DCollectiveCommInfo*> myComms;
		bool myIsReduction;
		bool myIsActive;
		bool myDoIntraLayerChecking;
		bool myAncestorDoesIntraChecking;
		bool myHasIntraComm;
		int *myReusableCountArray; /**< Used for intralayer based message matching to avoid reocurring allocations.*/
		int myWorldSize;

		I_ParallelIdAnalysis* myPIdMod;
		I_LocationAnalysis* myLIdMod;
		I_CommTrack	*myCommTrack;
		I_DatatypeTrack *myTypeTrack;
		I_OpTrack* myOpTrack;
		I_CreateMessage* myLogger;
		I_BaseConstants *myConsts;

		Must_Coll_No_TransferP myCollNoTransferFct;
		Must_Coll_SendP mySendFct;
		Must_Coll_Op_SendP myOpSendFct;
		Must_Coll_Send_nP mySendNFct;
		Must_Coll_Op_Send_nP myOpSendNFct;
		Must_Coll_Send_buffersP mySendBuffersFct;
		Must_Coll_Op_Send_buffersP myOpSendBuffersFct;
		Must_Coll_Send_countsP mySendCountsFct;
		Must_Coll_Op_Send_countsP myOpSendCountsFct;
		Must_Coll_Send_typesP mySendTypesFct;
		Must_Coll_RecvP myRecvFct;
		Must_Coll_Recv_nP myRecvNFct;
		Must_Coll_Op_Recv_nP myOpRecvNFct;
		Must_Coll_Recv_buffersP myRecvBuffersFct;
		Must_Coll_Recv_countsP myRecvCountsFct;
		Must_Coll_Recv_typesP myRecvTypesFct;

		gtiSetNextEventStridedP myNextEventStridedFct;

		dCollMatchAncestorHasIntraP myAncestorHasIntraFct;

		passTypeMatchInfoP myPassTypeMatchInfoFct;
		passTypeMatchInfoTypesP myPassTypeMatchInfoTypesFct;

		bool myHadNewOp;

		I_DCollectiveListener *myListener;
		std::list<I_CollCommListener*> myCommListeners;

		/**
		 * Returns the value to return in case of an error.
		 * (GTI_ANALYSIS_SUCCESS for root,
		 *  GTI_ANALYSIS_IRREDUCIBLE for reduction)
		 */
		GTI_ANALYSIS_RETURN getErrorReturn (void);

		/**
		 * Takes care of executing a new operation.
		 * @param rank of the op.
		 * @param cId channel ID the operation.
		 * @param outFinishedChannels given for the event.
		 * @param newOp new operation.
		 * @return true if successful, false otherwise.
		 */
		GTI_ANALYSIS_RETURN handleNewOp (int rank, I_ChannelId* cId, std::list<gti::I_ChannelId*> *outFinishedChannels, DCollectiveOp* newOp);

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
		 * Prints the matching state as DOT output into a file.
		 * Creates one file per invocation.
		 */
		bool printAsDot (void);

	public:
		/**
		 * Constructor.
		 * @param instanceName name of this module instance.
		 * @param isReduction true if this is a reduction.
		 */
		DCollectiveMatch (const char* instanceName, bool isReduction);

		/**
		 * Destructor.
		 */
		virtual ~DCollectiveMatch();

		/**
		 * @see I_DCollectiveMatch::CollNoTransfer.
		 */
		GTI_ANALYSIS_RETURN CollNoTransfer (
		        MustParallelId pId,
		        MustLocationId lId,
		        int coll, // formerly MustCollCommType
		        MustCommType comm,
		        int numTasks,
		        int hasRequest,
		        MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		);

		/**
		 * @see I_DCollectiveMatch::CollSend.
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
		        int numTasks,
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		);

		/**
		 * @see I_DCollectiveMatch::CollSendN.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;

		/**
		 * @see I_DCollectiveMatch::CollSendCounts.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;

		/**
		 * @see I_DCollectiveMatch::CollSendTypes.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;

		/**
		 * @see I_DCollectiveMatch::CollRecv.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;

		/**
		 * @see I_DCollectiveMatch::CollRecvN.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;


		/**
		 * @see I_DCollectiveMatch::CollRecvCounts.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;

		/**
		 * @see I_DCollectiveMatch::CollRecvTypes.
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
                int hasRequest,
                MustRequestType request,
	            gti::I_ChannelId *cId,
	            std::list<gti::I_ChannelId*> *outFinishedChannels
		) ;

		/**
		 * @see I_DCollectiveMatchReduction::ancestorHasIntraComm.
		 */
		GTI_ANALYSIS_RETURN ancestorHasIntraComm (
		        int hasIntra,
		        gti::I_ChannelId *cId,
		        std::list<gti::I_ChannelId*> *outFinishedChannels);

		/**
		 * @see I_DCollectiveMatchReduction::handleIntraTypeMatchInfo.
		 */
		GTI_ANALYSIS_RETURN handleIntraTypeMatchInfo (
		        MustParallelId pId,
		        MustLocationId lId,
		        MustRemoteIdType commRId,
		        MustRemoteIdType typeRId,
		        int numCounts,
		        int* counts,
		        int firstRank,
		        int collectiveNumber,
		        int collId );

		/**
		 * @see I_DCollectiveMatchReduction::handleIntraTypeMatchInfoTypes.
		 */
		GTI_ANALYSIS_RETURN handleIntraTypeMatchInfoTypes (
		        MustParallelId pId,
		        MustLocationId lId,
		        MustRemoteIdType commRId,
		        int numCountsAndTypes,
		        MustRemoteIdType* typeRIds,
		        int* counts,
		        int firstRank,
		        int collectiveNumber,
		        int collId );

		/**
		 * @see I_Reduction::timeout
		 */
		void timeout (void);

		/**
		 * @see I_DCollectiveOpProcessor::pIdToRank
		 */
		int pIdToRank (MustParallelId pId);

		/**
		 * @see I_DCollectiveOpProcessor::getLevelIdForApplicationRank
		 */
		int getLevelIdForApplicationRank (int rank);

		/**
		 * @see I_DCollectiveOpProcessor::getLogger
		 */
		I_CreateMessage* getLogger (void);

		/**
		 * Provides a I_DatatypeTrack implementation.
		 */
		I_DatatypeTrack* getDatatypeTrack (void);

		/**
		 * Provides a I_CommTrack implementation.
		 */
		I_CommTrack* getCommTrack (void);

		/**
		 * Provides a I_LocationAnalysis implementation.
		 */
		I_LocationAnalysis* getLocationModule (void);

		/**
		 * Provides a I_LocationAnalysis implementation.
		 */
		int* getWorldSizedCountArray (void);

		/**
		 * Provides a I_LocationAnalysis implementation.
		 */
		int getWorldSize (void);

		/**
		 * @see I_DCollectiveOpProcessor::getNoTransferFct
		 */
		Must_Coll_No_TransferP getNoTransferFct (void);
		Must_Coll_SendP getSendFct (void);
		Must_Coll_Op_SendP getOpSendFct (void);
		Must_Coll_Send_nP getSendNFct (void);
		Must_Coll_Op_Send_nP getOpSendNFct (void);
		Must_Coll_Send_buffersP getSendBuffersFct (void);
		Must_Coll_Op_Send_buffersP getOpSendBuffersFct (void);
		Must_Coll_Send_countsP getSendCountsFct (void);
		Must_Coll_Op_Send_countsP getOpSendCountsFct (void);
		Must_Coll_Send_typesP getSendTypesFct (void);
		Must_Coll_RecvP getRecvFct (void);
		Must_Coll_Recv_nP getRecvNFct (void);
		Must_Coll_Op_Recv_nP getOpRecvNFct (void);
		Must_Coll_Recv_buffersP getRecvBuffersFct (void);
		Must_Coll_Recv_countsP getRecvCountsFct (void);
		Must_Coll_Recv_typesP getRecvTypesFct (void);

		/**
		 * @see I_DCollectiveOpProcessor::getPassTypeMatchInfoFct
		 */
		passTypeMatchInfoP getPassTypeMatchInfoFct (void);
		passTypeMatchInfoTypesP getPassTypeMatchInfoTypesFct (void);

		/**
		 * @see I_DCollectiveOpProcessor::getSetNextEventStridedFct
		 */
		gtiSetNextEventStridedP getSetNextEventStridedFct (void);

		/*
		 * @see I_DCollectiveMatchReduction::registerListener
		 */
		void registerListener (I_DCollectiveListener *listener);

		/**
		 * @see I_DCollectiveMatchReduction::registerCommListener
		 */
		void registerCommListener (I_CollCommListener *listener);
	};
} //namespace must

// as this class includes templates the implementation has to come alongside the declaration in one file
#include "DCollectiveMatch.hpp"

#endif /* DCOLLECTIVEMATCH_H */
