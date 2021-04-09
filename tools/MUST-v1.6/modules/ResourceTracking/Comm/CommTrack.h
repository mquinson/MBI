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
 * @file CommTrack.h
 *       @see MUST::CommTrack
 *
 *  @date 01.03.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "TrackBase.h"
#include "CompletionTree.h"

#include "I_CommTrack.h"
#include "Comm.h"

#include <map>

#ifndef COMMTRACK_H
#define COMMTRACK_H

using namespace gti;

namespace must
{
	/**
     * Implementation for I_CommTrack.
     */
    class CommTrack : public TrackBase<Comm, I_Comm, MustCommType, MustMpiCommPredefined, CommTrack, I_CommTrack>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		CommTrack (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~CommTrack (void);

    		/**
    		 * @see I_CommTrack::commGroup.
    		 */
    		GTI_ANALYSIS_RETURN  commGroup (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm,
    				MustGroupType group
    			);

    		/**
    		 * @see I_CommTrack::commCreate.
    		 */
    		GTI_ANALYSIS_RETURN commCreate(
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm,
    				MustGroupType group,
    				MustCommType newcomm);

    		/**
    		 * @see I_CommTrack::commDup.
    		 */
    		GTI_ANALYSIS_RETURN commDup(
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm,
    				MustCommType newcomm);

    		/**
    		 * @see I_CommTrack::commFree.
    		 */
    		GTI_ANALYSIS_RETURN commFree(
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm);

    		/**
    		 * @see I_CommTrack::commSplit.
    		 */
    		GTI_ANALYSIS_RETURN commSplit(
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm,
    				int color,
    				int key,
    				MustCommType newcomm,
    				int newCommSize,
    				int *newRank2WorldArray);

    		/**
    		 * @see I_CommTrack::graphCreate.
    		 */
    		GTI_ANALYSIS_RETURN graphCreate(
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType commOld,
    				int nnodes,
    				int nedges,
    				const int* indices,
    				const int* edges,
    				int reorder,
    				MustCommType commGraph,
    				int newCommSize,
    				int *newRank2WorldArray);

    		/**
    		 * @see I_CommTrack::cartCreate.
    		 */
    		GTI_ANALYSIS_RETURN cartCreate(
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType commOld,
    				int ndims,
    				const int* dims,
    				const int* periods,
    				int reorder,
    				MustCommType commCart,
    				int newCommSize,
    				int *newRank2WorldArray);

            /**
             * @see I_CommTrack::cartSub.
             */
            GTI_ANALYSIS_RETURN cartSub(
                    MustParallelId pId,
                    MustLocationId lId,
                    MustCommType commOld,
                    int ndims,
                    const int* remain,
                    MustCommType newcomm,
                    int newCommSize,
                    int *newRank2WorldArray);
    		/**
    		 * @see I_CommTrack::intercommCreate.
    		 */
    		GTI_ANALYSIS_RETURN intercommCreate (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType local_comm,
    				int local_leader,
    				MustCommType peer_comm,
    				int remote_leader,
    				int tag,
    				MustCommType newintercomm,
    				int remoteGroupSize,
    				int *remoteRank2WorldArray,
    				int contextId);

    		/**
    		 * @see I_CommTrack::intercommMerge.
    		 */
    		GTI_ANALYSIS_RETURN intercommMerge (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType intercomm,
    				int high,
    				MustCommType newintracomm,
    				int newCommSize,
    				int *newRank2WorldArray);

    		/**
    		 * @see I_CommTrack::commRemoteGroup.
    		 */
    		GTI_ANALYSIS_RETURN commRemoteGroup (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustCommType comm,
    				MustGroupType newGroup);

    		/**
    		 * @see I_CommTrack::addPredefinedComms.
    		 */
    		GTI_ANALYSIS_RETURN addPredefinedComms (
                        MustParallelId pId,
    		    		int reachableBegin,
    		    		int reachableEnd,
    		    		int worldSize,
    		    		MustCommType commNull,
    		    		MustCommType commSelf,
    		    		MustCommType commWorld,
    		    		int numWorlds,
    		    		MustCommType *worlds,
    		    		I_ChannelId *channId);

    		/**
    		 * @see I_CommTrack::addRemoteComm.
    		 */
    		GTI_ANALYSIS_RETURN addRemoteComm (
    		        int rank,
    		        int hasHandle,
    		        MustCommType commHandle,
    		        MustRemoteIdType remoteId,
    		        int isNull,
    		        int isPredefined,
    		        int predefinedEnum,
    		        int isCartesian,
    		        int isGraph,
    		        int isIntercomm,
    		        unsigned long long myContextId,
    		        MustRemoteIdType groupTableId,
    		        MustRemoteIdType groupTableIdRemte,
    		        MustParallelId creationPId,
    		        MustLocationId creationLId,
    		        int reorder,
    		        int ndims,
    		        const int *dims,
    		        const int *periods,
    		        int nnodes,
    		        int nedges,
    		        const int *indices,
    		        const int *edges);

    		/**
    		 * @see I_CommTrack::freeRemoteComm.
    		 */
    		GTI_ANALYSIS_RETURN freeRemoteComm (
    		        int rank,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_CommTrack::getComm
    		 */
    		I_Comm* getComm (
    				MustParallelId pId,
    				MustCommType comm);

    		/**
    		 * @see I_CommTrack::getComm
    		 */
    		I_Comm* getComm (
    				int rank,
    				MustCommType comm);

    		/**
    		 * @see I_CommTrack::getPersistentComm
    		 */
    		I_CommPersistent* getPersistentComm (
    		        MustParallelId pId,
    		        MustCommType comm);

    		/**
    		 * @see I_CommTrack::getPersistentComm
    		 */
    		I_CommPersistent* getPersistentComm (
    		        int rank,
    		        MustCommType comm);

    		/**
    		 * @see I_CommTrack::getRemoteComm
    		 */
    		I_Comm* getRemoteComm (
    		        MustParallelId pId,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_CommTrack::getRemoteComm
    		 */
    		I_Comm* getRemoteComm (
    		        int rank,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_CommTrack::getPersistentRemoteComm
    		 */
    		I_CommPersistent* getPersistentRemoteComm (
    		        MustParallelId pId,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_CommTrack::getPersistentRemoteComm
    		 */
    		I_CommPersistent* getPersistentRemoteComm (
    		        int rank,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_CommTrack::getWorldHandle
    		 */
    		MustCommType getWorldHandle (void);

    		/**
    		 * @see I_CommTrack::passCommAcross
    		 */
    		bool passCommAcross (
    		        MustParallelId pId,
    		        MustCommType comm,
    		        int toPlaceId);

    		/**
    		 * @see I_CommTrack::passCommAcross
    		 */
    		bool passCommAcross (
    		        int rank,
    		        MustCommType comm,
    		        int toPlaceId);

    		/**
    		 * @see I_CommTrack::passCommAcross
    		 */
    		bool passCommAcross (
    		        int rank,
    		        I_Comm* comm,
    		        int toPlaceId,
    		        MustRemoteIdType *pOutRemoteId);

    		/**
    		 * @see I_CommTrack::notifyOfShutdown
    		 * @see TrackBase::notifyOfShutdown
    		 */
    		void notifyOfShutdown (void);

    protected:
    		I_GroupTrack *myGroupMod; /**< The group tracking module.*/

    		MustCommType myCommSelfHandle; /**< The handle that represents MPI_COMM_SELF.*/
    		std::vector <Comm*> mySelfCommInfos; /**< Information for each MPI_COMM_SELF of each rank (keep in mind they have distinct groups on each rank).*/
    		std::vector <Comm*> myWorldCommInfos; /**< Information for MPI_COMM_WORLD.*/
    		Comm* myNullInfo;

    		bool myReachableAvailable; /**< Set to true once all information on reachable processes arrived.*/
    		int myReachableBegin; /**< Beginning MPI_COMM_WORLD rank of reachable processes.*/
    		int myReachableEnd; /**< Last MPI_COMM_WORLD rank of reachable processes.*/

    		/*
    		 * In some scenarios we may hit a replaced or the real MPI_COMM_WORLD handle,
    		 * so we keep both of them and use both of them to compare against MPI_COMM_WORLD.
    		 */
    		std::map<int, MustCommType> myCommWorldHandles; //maps a rank to its comm world handle (Wirh MPI virtualization replacing the real comm world, we may have a different handle on each process)
    		MustCommType myRealCommWorld; //the real MPI_COMM_WORLD constant handle value, not a replaced one which is handled by myCommWorldHandles

    		passCommAcrossP myPassCommAcrossFunc; /**< Function pointer to use for passing a communicator to another node in this level.*/
    		passFreeAcrossP myFreeCommAcrossFunc; /**< Function pointer to use for freeing a communicator on a remote side (previously passed to another node in this level).*/

    		/**
    		 * Returns the full communicator information for the given communicator and context.
    		 * Returns an info for user, predefined and null comms.
    		 *
    		 * We can't use TrackBase::getHandleInfo here as we have a specialized handling of
    		 * MPI_COMM_SELF and MPI_COMM_WORLD as predefineds!
    		 *
    		 * @param pId context for comm.
    		 * @param comm to query for.
    		 * @return the communicator in question or NULL if not found.
    		 */
    		Comm* getCommInfo (
    				MustParallelId pId,
    				MustCommType comm);

    		/**
    		 * @see other getCommInfo, difference is pid.
    		 */
    		Comm* getCommInfo (
    				int rank,
    				MustCommType comm);

    		/**
    		 * Implementation for the different passCommAcross versions.
    		 */
    		bool passCommAcrossInternal (
    		        int rank,
    		        Comm* comm,
    		        int toPlaceId,
    		        MustRemoteIdType *pOutRemoteId,
    		        bool hasHandle,
    		        MustCommType handle);


    }; /*class CommTrack */
} /*namespace MUST*/

#endif /*COMMTRACK_H*/
