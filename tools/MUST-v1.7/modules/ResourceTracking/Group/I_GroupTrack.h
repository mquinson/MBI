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
 * @file I_GroupTrack.h
 *       @see I_GroupTrack.
 *
 *  @date 24.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_GroupTable.h"
#include "I_Group.h"
#include "I_TrackBase.h"

#include <vector>
#include <list>

#ifndef I_GROUPTRACK_H
#define I_GROUPTRACK_H

/**
 * Interface for querying information on groups.
 *
 * Important: This analysis module only tracks groups,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 */
class I_GroupTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Group>
{
public:

	/**
	 * Creates a new group that is the union of the two
	 * given groups.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param group1 of union.
	 * @param group2 of union.
	 * @param newGroup the new group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupUnion (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType group1,
			MustGroupType group2,
			MustGroupType newGroup) = 0;

	/**
	 * Creates a new group that results from intersecting
	 * group1 and group2.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param group1 left side of intersection.
	 * @param group2 right side of intersection.
	 * @param newGroup resulting group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupIntersection (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType group1,
			MustGroupType group2,
			MustGroupType newGroup) = 0;

	/**
	 * Creates a new group that results from subtracting group2
	 * from group1.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param group1 left side of difference.
	 * @param group2 right side of difference.
	 * @param newGroup result of the difference.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupDifference (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType group1,
			MustGroupType group2,
			MustGroupType newGroup) = 0;

	/**
	 * Restricts the given group to the given list of ranks.
	 * This also influences their ordering within the group.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param oldGroup to which the operation is applied.
	 * @param n number of ranks to stay in the group.
	 * @param ranks of group oldGroup, ranks[i] will become
	 *               rank i in newGroup.
	 * @param newGroup handle of the new group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupIncl (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType oldGroup,
			int n,
			const int *ranks,
			MustGroupType newGroup) = 0;

	/**
	 * Excludes a list of ranks from the given group.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param oldGroup to exclude ranks from.
	 * @param n number of ranks to exclude.
	 * @param ranks list of ranks to exclude.
	 * @param newGroup handle of the resulting group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupExcl (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType oldGroup,
			int n,
			const int* ranks,
			MustGroupType newGroup) = 0;

	/**
	 * A groupIncl with triplets of (first, last, stride) that determines
	 * what ranks to include in the new group.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param oldGroup to appliy the range incl to.
	 * @param n number of ranges.
	 * @param array of size 3*n, where each three consecutive entries represents one triplet.
	 * @param newGroup handle of the resulting group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupRangeIncl (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType oldGroup,
			int n,
			const int *ranges,
			MustGroupType newGroup) = 0;

	/**
	 * A groupExcl with range triplets as in groupRangeIncl.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param oldGroup to appliy the range excl to.
	 * @param n number of ranges.
	 * @param array of size 3*n, where each three consecutive entries represents one triplet.
	 * @param newGroup handle of the resulting group.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupRangeExcl (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType oldGroup,
			int n,
			const int *ranges,
			MustGroupType newGroup) = 0;

	/**
	 * Frees the given group.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param group to free.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN groupFree (
			MustParallelId pId,
			MustLocationId lId,
			MustGroupType group) = 0;

	/**
	 * Adds a group table passed from another place to this tool level.
	 *
	 * Used for non-continous group tables.
	 *
	 * @param int rank
	 * @param remoteId used to identify the group table
	 * @param size of the group
	 * @param translation of the group ranks to MPI_COMM_WORLD ranks
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addRemoteGroupTableRep2 (
	        int rank,
	        MustRemoteIdType remoteId,
	        int size,
	        int* translation) = 0;

	/**
	 * Adds a group table passed from another place to this tool level.
	 *
	 * Used for continous group tables.
	 *
	 * @param int rank
	 * @param remoteId used to identify the group table
	 * @param beginRank begin rank of the continous groupt table
	 * @param endRank end rank
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN addRemoteGroupTableRep1 (
	        int rank,
	        MustRemoteIdType remoteId,
	        int beginRank,
	        int endRank) = 0;

	/**
	 * Notifies that a remote place on the same tool level doesn't needs
	 * a group table it passed to this place any more.
	 *
	 * @param int rank
	 * @param remoteId used to identify the group table
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN freeRemoteGroupTable (
	        int rank,
	        MustRemoteIdType remoteId) = 0;

	/**
	 * Returns pointer to group information.
     * Is NULL if this is an unknown handle, note that
     * a MPI_GROUP_NULL handle returns a valid pointer though.
     *
     * Memory must not be freed and is valid until I_GroupTrack
     * receives the next event, if you need the information longer
     * query getPersistentGroup instead.
	 *
	 * @param pId of the context.
	 * @param group to query for.
	 * @return information for the given group.
	 */
	virtual must::I_Group* getGroup (
			MustParallelId pId,
			MustGroupType group) = 0;

	/** As I_GroupTrack::getInfo with rank instead of pid.*/
	virtual must::I_Group* getGroup (
			int rank,
			MustGroupType group) = 0;

	/**
	 * Like I_GroupTrack::getComm, though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_Group * groupInfo = myGroupTrack->getPersistentGroup (pId, handle);
     if (groupInfo == NULL) return;
     .... //Do something with groupInfo
     groupInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
	 *
	 * @param pId of the context.
	 * @param group to query for.
	 * @return information for the given group.
	 */
	virtual must::I_GroupPersistent* getPersistentGroup (
	        MustParallelId pId,
	        MustGroupType group) = 0;

	/** As I_GroupTrack::getInfo with rank instead of pid.*/
	virtual must::I_GroupPersistent* getPersistentGroup (
	        int rank,
	        MustGroupType group) = 0;

	/**
	 * Returns a pointer to a group with the given description.
	 * This creates a group with no mapping! I.e. rank i in
	 * this group is mapped to rank i in MPI_COMM_WORLD.
	 *
	 * The reference count of the group will be incremented by one.
	 * Decrement it if you want to free the group, but don't free it.
	 *
	 * @param intervalBegin begin of rankInterval.
	 * @param intervalEnd end of rankInterval.
	 * @return pointer to the group.
	 */
	virtual must::I_GroupTable* getGroupTable (int intervalBegin, int intervalEnd) = 0;

	/**
	 * Returns a pointer to a group with the given description.
	 * This creates a group where rank i is mapped to the
	 * value of the i-th entry in the vector in MPI_COMM_WORLD.
	 *
	 * The given vector must not have duplicates !
	 *
	 * The reference count of the group will be incremented by one.
	 * Decrement it if you want to free the group, but don't free it.
	 *
	 * @param group mapping of group ranks to MPI_COMM_WORLD.
	 * @return pointer to the group.
	 */
	virtual must::I_GroupTable* getGroupTable (std::vector<int> group) = 0;

	/**
	 * Like the other getGroupTable functions but for remote group tables that
	 * are identified with a (rank,remoteId) pair
	 */
	virtual must::I_GroupTable* getGroupTable (
	        int rank,
	        MustRemoteIdType remoteId) = 0;

	/**
	 * Notifies the group tracking of a new group that was derived
	 * from a communicator, the tracking gets the group of the
	 * communicator, as it can't access information on the
	 * communicator.
	 *
	 * @param pId of the new groups context.
	 * @param lId of the new groups context.
	 * @param commGroup group of the associator.
	 * @param newGroup handle value of the new group.
	 */
	virtual void commGroup (
			MustParallelId pId,
			MustLocationId lId,
			must::I_GroupTable *commGroup,
			MustGroupType newGroup) = 0;

	/**
	 * Passes the given group table to the given place on this tool level.
	 * @param pId context of the table to pass
	 * @param group to pass, retrieved with getGroupTable at an earlier point
	 * @param toPlaceId place to send to
	 * @param remoteId of the group that was passed across (output argument).
	 * @return true iff successful.
	 *
	 * Reasons for this to fail include the unavailability of intra layer
	 * communication.
	 */
	virtual bool passGroupTableAcross (
	        MustParallelId pId,
	        must::I_GroupTable *group,
	        int toPlaceId,
	        MustRemoteIdType* remoteId) = 0;

	/**
	 * Like the other passGroupTableAcross version but with rank instead of a pId.
	 */
	virtual bool passGroupTableAcross (
	        int rank,
	        must::I_GroupTable *group,
	        int toPlaceId,
	        MustRemoteIdType* remoteId) = 0;

	/**
	 * Adds the integer (MustGroupType) values for all predefined
	 * (named) handles for groups.
	 *
     * @param pId of the context.
	 * @param groupNull value of MPI_GROUP_NULL.
	 * @param numPredefs number of predefined non null groups being sent.
	 * @param predefinedIds array of value of MustMpiGroupPredefined for each predefined type, array size is numPredefs.
	 * @param predefinedValues array of handles for the predefined types.
	 *
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
            MustParallelId pId,
			MustGroupType groupNull,
			int numPredefs,
			int* predefinedIds,
			MustGroupType* predefinedValues) = 0;

	/**
	 * Returns a list of all currently known user handles.
	 * Usage scenarios involve logging lost handles at finalize.
	 * @return a list of pairs of the form (rank, handle id).
	 */
	virtual std::list<std::pair<int, MustGroupType> > getUserHandles (void) = 0;

	/**
	 * Allows other modules to notify this module of an ongoing shutdown.
	 * This influcences the behavior of passing free calls across to other layers.
	 */
	virtual void notifyOfShutdown (void) = 0;
        virtual bool isPredefined(must::I_Group * info) {return false;}    
        
}; /*class I_GroupTrack*/

#endif /*I_GROUPTRACK_H*/
