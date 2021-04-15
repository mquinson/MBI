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
 * @file I_CommTrack.h
 *       @see I_CommTrack.
 *
 *  @date 24.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "MustTypes.h"
#include "BaseIds.h"
#include "I_ChannelId.h"
#include "I_GroupTrack.h"
#include "I_GroupTable.h"
#include "I_Comm.h"
#include "I_TrackBase.h"

#include <list>

#ifndef I_COMMTRACK_H
#define I_COMMTRACK_H

/**
 * Interface for querying information on communicators.
 *
 * Important: This analysis module only tracks communicators,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 * - GroupTrack
 */
class I_CommTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Comm>
{
public:

	/**
	 * Creates a group that is a copy of the communicators group.
	 * Adds this group to I_GroupTrack. This is done by the
	 * communicator tracker to avoid a cyclic dependency between
	 * the communicator and group tracking.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param comm from which the group was copied.
	 * @param group handle of the newly created group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN commGroup (
		MustParallelId pId,
		MustLocationId lId,
		MustCommType comm,
		MustGroupType group
		) = 0;

	/**
	 * Creates a communicator from the given group.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param comm of which group is a subset.
	 * @param group for the new comm.
	 * @param newcomm the new communicator handle.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN commCreate(
			MustParallelId pId,
			MustLocationId lId,
			MustCommType comm,
			MustGroupType group,
			MustCommType newcomm) = 0;

	/**
	 * Creates a new communicator by duplicating an
	 * existing one.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param comm to duplicate.
	 * @param newcomm handle of the new comm.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN commDup(
			MustParallelId pId,
			MustLocationId lId,
			MustCommType comm,
			MustCommType newcomm) = 0;

	/**
	 * Frees a communicator.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param comm to free.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN commFree(
			MustParallelId pId,
			MustLocationId lId,
			MustCommType comm) = 0;

	/**
	 * Splits a communicator into multiple ones.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param comm to split.
	 * @param color of the calling process.
	 * @param key order influencing value.
	 * @param newcomm handle of the new communicator.
	 * @param newCommSize number of processes in the
	 *               group associated with newcomm.
	 * @param newRank2WorldArray array of newcomm group
	 *               ranks to MPI_COMM_WORLD rank translations.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN commSplit(
			MustParallelId pId,
			MustLocationId lId,
			MustCommType comm,
			int color,
			int key,
			MustCommType newcomm,
			int newCommSize,
			int *newRank2WorldArray) = 0;

	/**
	 * Creates a communicator with a graph topology.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param commOld old communicator in which processes
	 *              to empbedd the graph.
	 * @param nnodes number od nodes in the graph.
	 * @param indices number of outging arcs array,
	 *               see MPI standard for details.
	 * @param edges arc destinations,
	 *               see MPI standard for details.
	 * @param commGraph handle of the new communicator.
	 * @param newCommSize number of processes in the
	 *               group associated with newcomm.
	 * @param newRank2WorldArray array of newcomm group
	 *               ranks to MPI_COMM_WORLD rank translations.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN graphCreate(
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
			int *newRank2WorldArray) = 0;

	/**
	 * Creates a communicator with a cartesian toplogy.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param commOld old communicator.
	 * @param ndims number of dimensions for the
	 *              cartesian toplogy.
	 * @param dims array of dimension sizes.
	 * @param periods array of logical values specifing
	 *              which dimensions have wrapp-around
	 *              connections.
	 * @param reorder set to true if processes may be
	 *              reordered in the new communicator.
	 * @param newCart handle of the new communicator.
	 * @param newCommSize number of processes in the
	 *               group associated with newcomm.
	 * @param newRank2WorldArray array of newcomm group
	 *               ranks to MPI_COMM_WORLD rank translations.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN cartCreate(
			MustParallelId pId,
			MustLocationId lId,
			MustCommType commOld,
			int ndims,
			const int* dims,
			const int* periods,
			int reorder,
			MustCommType commCart,
			int newCommSize,
			int *newRank2WorldArray) = 0;

    /**
     * Creates a lower dimensional subsection of a communicator with a cartesian toplogy.
     *
     * @param pId parallel id of call site.
     * @param lId location id of call site.
     * @param commOld old communicator.
     * @param ndims number of dimensions for the
     *              cartesian toplogy.
     * @param remain array of logical values specifing
     *              which dimensions remain in the new
     *              communicator.
     * @param newCart handle of the new communicator.
     * @param newCommSize number of processes in the
     *               group associated with newcomm.
     * @param newRank2WorldArray array of newcomm group
     *               ranks to MPI_COMM_WORLD rank translations.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN cartSub(
            MustParallelId pId,
            MustLocationId lId,
            MustCommType commOld,
            int ndims,
            const int* remain,
            MustCommType newcomm,
            int newCommSize,
            int *newRank2WorldArray)=0;
	/**
	 * Creates an intercommunicator.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param local_comm comm defining the ranks in the local group.
	 * @param local_leader rank that leads in the local group.
	 * @param peer_comm communicator in which both leaders are, only significant on local leader ranks.
	 * @param remote_leader rank of remote leader in peer_comm, only significant on local leader ranks.
	 * @param tag to use for leader communication on peer_comm.
	 * @param newintercomm the newly created intercomm.
	 * @param remoteGroupSize size of the remote group associated with the new comm.
	 * @param remoteRank2WorldArray ranks in the remote group to world rank translation.
	 * @param contextId the contextId used when comparing comms (will be calculated by communication (an operaiton does this))
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN intercommCreate (
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
			int contextId) = 0;

	/**
	 * Merges the local and remote groups of an intercommunicator
	 * into an intracommunicator.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param intercomm that will be merged.
	 * @param high specifies what ordering is to be used when creating the new comm.
	 * @param newintracomm the newly created inter communicator.
	 * @param newCommSize size of the new communicator.
	 * @param newRank2WorldArray ranks in new communicator to world rank translation.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN intercommMerge (
			MustParallelId pId,
			MustLocationId lId,
			MustCommType intercomm,
			int high,
			MustCommType newintracomm,
			int newCommSize,
			int *newRank2WorldArray) = 0;

	/**
	 * Returns a new group that is the remote group of the given
	 * intercommunicator.
	 *
	 * @param pId parallel id of call site.
	 * @param lId location id of call site.
	 * @param comm from which the remote group is retrieved.
	 * @param newGroup handle of the new group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN commRemoteGroup (
			MustParallelId pId,
			MustLocationId lId,
			MustCommType comm,
			MustGroupType newGroup) = 0;

	/**
	 * Adds the predefineds and important values for them
	 * like the list of processes reachable by this place.
	 *
	 * The extra channel id is used to track whether the reachable
	 * interval is already complete, or whether some processes
	 * are missing.
	 *
	 * Beware, MPI_COMM_WORLD can have a different value on each process if
	 * an MPI split module is used to replace it with another communicator.
	 *
	 * @param reachableBegin start of interval of processes reachable by this place.
    	 * @param reachableEnd end of interval of processes reachable by this place.
    	 * @param worldSize size of MPI_COMM_WORLD
    	 * @param commNull value of MPI_COMM_NULL.
    	 * @param commSelf value of MPI_COMM_SELF.
    	 * @param commWorld value of the real MPI_COMM_WORLD constant (irespective of whether it was replaced with a different comm).
    	 * @param numWorlds number of MPI_COMM_WORLD values in list.
    	 * @param worlds values for MPI_COMM_WORLD on each task (Value of the possibly replaced MPI_COMM_WORLD, can differ on each rank e.g. MVAPICH).
    	 * @param channId channel id of this record.
	 */
    virtual gti::GTI_ANALYSIS_RETURN addPredefinedComms (
            MustParallelId pId,
    		int reachableBegin,
    		int reachableEnd,
    		int worldSize,
    		MustCommType commNull,
    		MustCommType commSelf,
    		MustCommType commWorld,
    		int numWorlds,
    		MustCommType *worlds,
    		gti::I_ChannelId *channId) = 0;

    /**
     * Adds a communicator that was passed to this from a different place on this level.
     *
     * For parameter descriptions see must::Comm and passCommAcrossP in
     * ResourceApi.h.
     *
     * @return @see GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteComm (
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
            const int *edges) = 0;

    /**
     * Frees a communicator that was passed from another place
     * on the same TBON level.
     *
     * @param pId context for the comm.
     * @param remoteId that was assigned to the comm on the remote side.
     * @return @see GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN freeRemoteComm (
            int rank,
            MustRemoteIdType remoteId) = 0;

	/**
	 * Returns pointer to communicator information.
	 * Is NULL if this is an unknown handle, note that
	 * a MPI_COMM_NULL handle returns a valid pointer though.
	 *
	 * Memory must not be freed and is valid until I_CommTrack
	 * receives the next event, if you need the information longer
	 * query getPersistentComm instead.
	 *
	 * @param pId of the communicator context.
	 * @param comm to query for.
	 * @return information for the given communicator.
	 */
    virtual must::I_Comm* getComm (
    		MustParallelId pId,
    		MustCommType comm) = 0;

    /** As I_CommTrack::getComm with rank instead of pid.*/
    virtual must::I_Comm* getComm (
    		int rank,
    		MustCommType comm) = 0;

    /**
     * Returns the handle of MPI_COMM_WORLD,
     * can be used to retrieve a handle information
     * on this communicator.
     * One should not use this handle for comparison with
     * other handle values but rather just the information
     * retrived by passing this handle to getComm!
     *
     * @return handle value of MPI_COMM_WORLD.
     */
    virtual MustCommType getWorldHandle (void) = 0;

    /**
     * Like I_CommTrack::getComm, though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_CommPersistent * commInfo = myCommTrack->getPersistentComm (pId, handle);
     if (commInfo == NULL) return;
     .... //Do something with commInfo
     commInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
     *
     * @param pId of the communicator context.
     * @param comm to query for.
     * @return information for the given communicator.
     */
    virtual must::I_CommPersistent* getPersistentComm (
            MustParallelId pId,
            MustCommType comm) = 0;

    /** As I_CommTrack::getPersistentComm with rank instead of pid.*/
    virtual must::I_CommPersistent* getPersistentComm (
            int rank,
            MustCommType comm) = 0;

    /**
     * Like I_CommTrack::getComm, but
     * with a remote id instead of a handle.
     */
    virtual must::I_Comm* getRemoteComm (
                MustParallelId pId,
                MustRemoteIdType remoteId) = 0;

    /**
     * Like I_CommTrack::getComm, but
     * with a remote id instead of a handle.
     */
    virtual must::I_Comm* getRemoteComm (
            int rank,
            MustRemoteIdType remoteId) = 0;

    /**
     * Like I_CommTrack::getPersistentComm, but
     * with a remote id instead of a handle.
     */
    virtual must::I_CommPersistent* getPersistentRemoteComm (
            MustParallelId pId,
            MustRemoteIdType remoteId) = 0;

    /**
     * Like I_CommTrack::getPersistentComm, but
     * with a remote id instead of a handle.
     */
    virtual must::I_CommPersistent* getPersistentRemoteComm (
            int rank,
            MustRemoteIdType remoteId) = 0;

	/**
	 * Returns a list of all currently known user handles.
	 * Usage scenarios involve logging lost handles at finalize.
	 * @return a list of pairs of the form (rank, handle id).
	 */
	virtual std::list<std::pair<int, MustCommType> > getUserHandles (void) = 0;

	/**
	 * Passes the given communicator to the given place on this tool level.
	 * @param pId context of the comm to pass
	 * @param comm to pass
	 * @param toPlaceId place to send to
	 * @return true iff successful.
	 *
	 * Reasons for this to fail include the unavailability of intra layer
	 * communication.
	 */
	virtual bool passCommAcross (
	        MustParallelId pId,
	        MustCommType comm,
	        int toPlaceId) = 0;

	/**
	 * Like the other passCommAcross version but with rank instead of a pId.
	 */
	virtual bool passCommAcross (
	        int rank,
	        MustCommType comm,
	        int toPlaceId) = 0;

	/**
	 * Like the other passCommAcross versions but with
	 * a comm info instead of a handle.
	 *
	 * This is usually more expensive than the other passCommAcross
	 * versions as this requires that the tracker checks whether there
	 * also exists a handle for this resource. To do that it has to
	 * search though all its handles which may be expensive.
	 *
	 * @param pOutRemoteId pointer to storage for a remote id, is set to
	 *               the remote id that is used to identify the resource on the
	 *               remote side.
	 */
	virtual bool passCommAcross (
	        int rank,
	        must::I_Comm* comm,
	        int toPlaceId,
	        MustRemoteIdType *pOutRemoteId) = 0;

	/**
	 * Allows other modules to notify this module of an ongoing shutdown.
	 * This influcences the behavior of passing free calls across to other layers.
	 */
	virtual void notifyOfShutdown (void) = 0;
        virtual bool isPredefined(must::I_Comm * info) {return info->isPredefined();}  
    
}; /*class I_CommTrack*/

#endif /*I_COMMTRACK_H*/
