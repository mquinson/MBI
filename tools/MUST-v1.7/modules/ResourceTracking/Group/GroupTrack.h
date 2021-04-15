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
 * @file GroupTrack.h
 *       @see MUST::GroupTrack
 *
 *  @date 01.03.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "TrackBase.h"
#include "I_GroupTrack.h"
#include "Group.h"

#include <map>

#ifndef GROUPTRACK_H
#define GROUPTRACK_H

using namespace gti;

namespace must
{
	/**
     * Implementation for I_GroupTrack.
     */
    class GroupTrack : public TrackBase<Group, I_Group, MustGroupType, MustMpiGroupPredefined, GroupTrack, I_GroupTrack>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		GroupTrack (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~GroupTrack (void);

    		/**
    		 * @see I_GroupTrack::groupUnion
    		 */
    		GTI_ANALYSIS_RETURN groupUnion (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType group1,
    				MustGroupType group2,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupIntersection
    		 */
    		GTI_ANALYSIS_RETURN groupIntersection (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType group1,
    				MustGroupType group2,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupDifference
    		 */
    		GTI_ANALYSIS_RETURN groupDifference (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType group1,
    				MustGroupType group2,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupIncl
    		 */
    		GTI_ANALYSIS_RETURN groupIncl (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType oldGroup,
    				int n,
    				const int *ranks,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupExcl
    		 */
    		GTI_ANALYSIS_RETURN groupExcl (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType oldGroup,
    				int n,
    				const int* ranks,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupRangeIncl
    		 */
    		GTI_ANALYSIS_RETURN groupRangeIncl (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType oldGroup,
    				int n,
    				const int *ranges,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupRangeExcl
    		 */
    		GTI_ANALYSIS_RETURN groupRangeExcl (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType oldGroup,
    				int n,
    				const int *ranges,
    				MustGroupType newGroup);

    		/**
    		 * @see I_GroupTrack::groupFree
    		 */
    		GTI_ANALYSIS_RETURN groupFree (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustGroupType group);

    		/**
    		 * @see I_GroupTrack::addRemoteGroupTableRep1
    		 */
    		GTI_ANALYSIS_RETURN addRemoteGroupTableRep2 (
    		        int rank,
    		        MustRemoteIdType remoteId,
    		        int size,
    		        int* translation);

    		/**
    		 * @see I_GroupTrack::addRemoteGroupTableRep2
    		 */
    		GTI_ANALYSIS_RETURN addRemoteGroupTableRep1 (
    		        int rank,
    		        MustRemoteIdType remoteId,
    		        int beginRank,
    		        int endRank);

    		/**
    		 * @see I_GroupTrack::freeRemoteGroupTable
    		 */
    		GTI_ANALYSIS_RETURN freeRemoteGroupTable (
    		        int rank,
    		        MustRemoteIdType remoteId);

    		/**
    		 * @see I_GroupTrack::passGroupTableAcross
    		 */
    		bool passGroupTableAcross (
    		        MustParallelId pId,
    		        I_GroupTable *group,
    		        int toPlaceId,
    		        MustRemoteIdType* remoteId);

    		/**
    		 * @see I_GroupTrack::passGroupTableAcross
    		 */
    		bool passGroupTableAcross (
    		        int rank,
    		        I_GroupTable *group,
    		        int toPlaceId,
    		        MustRemoteIdType* remoteId);

    		/**
    		 * @see I_GroupTrack::getGroup
    		 */
    		I_Group* getGroup (
    				MustParallelId pId,
    				MustGroupType group);

    		/**
    		 * @see I_GroupTrack::getGroup
    		 */
    		I_Group* getGroup (
    				int rank,
    				MustGroupType group);

    		/**
    		 * @see I_GroupTrack::getPersistentGroup
    		 */
    		I_GroupPersistent* getPersistentGroup (
    		        MustParallelId pId,
    		        MustGroupType group);

    		/**
    		 * @see I_GroupTrack::getPersistentGroup
    		 */
    		I_GroupPersistent* getPersistentGroup (
    		        int rank,
    		        MustGroupType group);

    		/**
    		 * @see I_GroupTrack::getGroupTable
    		 */
    		I_GroupTable* getGroupTable (int intervalBegin, int intervalEnd);

    		/**
    		 * @see I_GroupTrack::getGroupTable
    		 */
    		I_GroupTable* getGroupTable (std::vector<int> group);

    		/**
    		 * @see I_GroupTrack::getGroupTable
    		 */
    		I_GroupTable* getGroupTable (
    		            int rank,
    		            MustRemoteIdType remoteId);

    		/**
    		 * Called to remove the given group from the
    		 * collection of groups and delete it.
    		 *
    		 * Should only be called by the implementation
    		 * of I_GroupTable.
    		 * (One might make this more restricted to
    		 *  enforce this ...)
    		 *
    		 * @param group to delete.
    		 */
    		void deleteGroupTable (I_GroupTable* group);

    		/**
    		 * @see I_GroupTrack::commGroup
    		 */
    		void commGroup (
    				MustParallelId pId,
    				MustLocationId lId,
    				must::I_GroupTable *commGroup,
    				MustGroupType newGroup);

    		protected:
    			/** List of groups.*/
    			typedef std::list<I_GroupTable*> GroupTableList;
    			/** Maps comm world rank of first and last rank in group to a list of groups with the same two ranks.*/
    			typedef std::map <std::pair<int, int>, GroupTableList> GroupTableSelection;
    			/** Maps the group size to a GroupTableSelection.*/
    			typedef std::map <int, GroupTableSelection> GroupTableCollection;

    			GroupTableCollection myGroupTables; /**< Collection of all existing groups.*/

    			passGroupTableAcrossRep1P myPassTableAcrossFunc1;
    			passGroupTableAcrossRep2P myPassTableAcrossFunc2;
    			passFreeAcrossP myFreeTableAcrossFunc;

    			//Extra mapping of (rank, remoteId) pairs to group tables for ressources that where received from remote
    			typedef std::pair<int, MustRemoteIdType> RemoteTableIdentifier;
    			typedef std::map<RemoteTableIdentifier, I_GroupTable*> RemoteMapType;
    			RemoteMapType myRemoteTables;

    			/**
    			 * Checks whether a group list for the described selection entry exists.
    			 * The entry in GroupTableCollection is groupSize, the entry in the resulting
    			 * selection is firstWorldRank, lastWorldRank.
    			 *
    			 * @param groupSize of the GroupTableCollection entry.
    			 * @param firstWorldRank of the GroupTableSelection entry.
    			 * @param lastWorldRank of the GroupTableSelection entry.
    			 * @param pList if found a pointer to the list associated with the found selection.
    			 * @return true if found, false otherwise.
    			 */
    			bool isGroupSelectionKnown (int groupSize, int firstWorldRank, int lastWorldRank, GroupTableList **pList);

    			/**
    			 * Searches the collection for the given group and returns true if it was found.
    			 *
    			 * @param group to search for.
    			 * @param pList if found pointer to the list in which the group was found.
    			 * @param pListIter if found pointer to the group position in the list.
    			 * @return true if successfull.
    			 */
    			bool isGroupTableKnown  (I_GroupTable* group, GroupTableList **pList, GroupTableList::iterator *pListIter);

    			/**
    			 * As the other isGroupTableKnown, but with a different specification for the group to search for.
    			 */
    			bool isGroupTableKnown  (int beginRank, int endRank, GroupTableList **pList, GroupTableList::iterator *pListIter);

    			/**
    			 * As the other isGroupTableKnown, but with a different specification for the group to search for.
    			 */
    			bool isGroupTableKnown  (std::vector<int> *set, GroupTableList **pList, GroupTableList::iterator *pListIter);

    			/**
    			 * Adds the given group into the group collection.
    			 * The caller must ensure that no equal group is already in the collection.
    			 * Will add missing selections and lists to the collection if needed.
    			 *
    			 * @param grouo to add.
    			 * @return true iff successful.
    			 */
    			bool addGroupTable (I_GroupTable *group);

    			/**
    			 * Returns the group for the given handle if found, returns
    			 * NULL otherwise. This includes user groups and MPI_GROUP_NULL.
    			 *
    			 * @param pId of handle context.
    			 * @param handle of group.
    			 * @return pointer to group, or NULL if not found.
    			 */
    			I_GroupTable* getGroupForHandle (MustParallelId pId, MustGroupType handle);

    			/**
    			 * Prints the whole group collection.
    			 */
    			void printCollection (void);

    			/**
    			 * Used to initialize myEmptyValue.
    			 * @see TrackBase::createPredefinedInfo.
    			 * (Implementation of hook)
    			 */
    			Group* createPredefinedInfo (int predefEnum, MustGroupType handle);

    }; /*class GroupTrack */
} /*namespace MUST*/

#endif /*GROUPTRACK_H*/
