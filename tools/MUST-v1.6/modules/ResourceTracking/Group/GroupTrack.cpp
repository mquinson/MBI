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
 * @file GroupTrack.cpp
 *       @see MUST::GroupTrack.
 *
 *  @date 06.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "GroupTrack.h"
#include "MustEnums.h"
#include "GroupTable.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(GroupTrack)
mFREE_INSTANCE_FUNCTION(GroupTrack)
mPNMPI_REGISTRATIONPOINT_FUNCTION(GroupTrack)

//=============================
// Constructor
//=============================
GroupTrack::GroupTrack (const char* instanceName)
	: TrackBase<Group, I_Group, MustGroupType, MustMpiGroupPredefined, GroupTrack, I_GroupTrack> (instanceName),
	  myGroupTables (),
	  myRemoteTables ()
{
    //Get function pointers for passing group tables to other places on the same level
    getWrapAcrossFunction("passGroupTableAcrossRep1", (GTI_Fct_t*) &myPassTableAcrossFunc1);
    getWrapAcrossFunction("passGroupTableAcrossRep2", (GTI_Fct_t*) &myPassTableAcrossFunc2);
    getWrapAcrossFunction("passFreeGroupTableAcross", (GTI_Fct_t*) &myFreeTableAcrossFunc);
}

//=============================
// Destructor
//=============================
GroupTrack::~GroupTrack ()
{
    //Notify HandleInfoBase of ongoing shutdown
    HandleInfoBase::disableFreeForwardingAcross();

    //IMPORTANT: We must manually free any user/predefined handles now, otherwise we can't clean up our structures
    //                     as the TrackBase destructor is only going to be called after this function!
    //                     (I.e. after killing our structures, but they will use the "deleteGroupTable" callback and thus rely on them)
    freeHandleMaps ();

	//Delete all group tables
	GroupTableCollection::iterator i;
	for (i = myGroupTables.begin(); i != myGroupTables.end(); i++)
	{
		GroupTableSelection::iterator j;
		for (j = i->second.begin(); j != i->second.end(); j++)
		{
			GroupTableList::iterator k;
			for (k = j->second.begin(); k != j->second.end(); k++)
			{

#ifdef MUST_DEBUG
			    I_GroupTable *group = *k;
				std::cerr << "A GroupTable was not freed during shutdown, this suggest a reference count problem, but may also result from a non-optimal destruction order! Group size: " << group->getSize() << std::endl;
#endif

				//May be a bad idea, depends on destruction order
				//if (group) delete (group);
				//*k = NULL;
			}
		}
	}

	myGroupTables.clear();
}

//=============================
// groupUnion
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupUnion (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType group1,
		MustGroupType group2,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
	Group* newInfo = getHandleInfo (pId, newGroup);

	if (newInfo)
	{
	    //If already known, it is NULL, EMPTY, or a user defined
	    //For NULL and EMPTY nothing to do

	    //For user handles, inc the MPI ref count
	    if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
	}

	//==Find information on oldGroup
	I_GroupTable* old1 = getGroupForHandle (pId, group1);
	I_GroupTable* old2 = getGroupForHandle (pId, group2);
	if (!old1 || !old2) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;

	/*
	 * We may have duplicates here I guess ...
	 * (1) First we put tuples of the form (worldRank, groupRank)
	 * into a map to easily get rid of duplicates.
	 * (2)Then we resize the new set with the size of the map
	 * and (3) loop over the map to put in the entries.
	 */

	//===(1)
	std::map<int,int> worldToNewRank;
	std::map<int,int>::iterator iter;

	int newRank = 0;

	// Group1
	for (int i = 0; i < old1->getSize(); i++)
	{
		//translate
		int worldTranslate;
		if (!old1->translate(i, &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous

		//No duplicates yet
		worldToNewRank.insert (std::make_pair (worldTranslate, newRank));
		newRank++;
	}

	// Group2
	for (int i = 0; i < old2->getSize(); i++)
	{
		//translate
		int worldTranslate;
		if (!old2->translate(i, &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous

		//Check for duplicates
		if (worldToNewRank.find(worldTranslate) != worldToNewRank.end())
			continue;

		//Insert
		worldToNewRank.insert (std::make_pair (worldTranslate, newRank));
		newRank++;
	}

	//===(2)
	set.resize(worldToNewRank.size());

	//===(3)
	for (iter = worldToNewRank.begin(); iter != worldToNewRank.end(); iter++)
	{
		set[iter->second] = iter->first;
	}

	//== Create the group
	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group* info = new Group ();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupIntersection
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupIntersection (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType group1,
		MustGroupType group2,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
    }

	//==Find information on oldGroup
	I_GroupTable* old1 = getGroupForHandle (pId, group1);
	I_GroupTable* old2 = getGroupForHandle (pId, group2);
	if (!old1 || !old2) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;

	/*
	 * (1) First we put tuples of the form (worldRank, group1Rank) by
	 * going over the first group. Afterwards we go over the second
	 * group and check for each element whether it is in the map,
	 * if so we put it a second set as a tuple of the form (group1Rank, worldRank).
	 * (2)Then we resize the new set with the size of the second map
	 * and (3) loop over the second map to put in the entries.
	 * (We use that the second map is ordered as in group1, though we must
	 * not use the group1Rank values as they might not be contiguous.)
	 */

	//===(1)
	std::map<int,int> worldToG1Rank; //first map
	std::map<int,int> G1RankToWorld; //second map
	std::map<int,int>::iterator iter;

	// Group1
	for (int i = 0; i < old1->getSize(); i++)
	{
		//translate
		int worldTranslate;
		if (!old1->translate(i, &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous

		worldToG1Rank.insert (std::make_pair (worldTranslate, i));
	}

	// Group2
	for (int i = 0; i < old2->getSize(); i++)
	{
		//translate
		int worldTranslate;
		if (!old2->translate(i, &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous

		//Is it in the first map ? If not continue
		iter =worldToG1Rank.find(worldTranslate);
		if (iter == worldToG1Rank.end())
			continue;

		//Insert into second map
		G1RankToWorld.insert (std::make_pair (iter->second, worldTranslate));
	}

	//===(2)
	set.resize(G1RankToWorld.size());

	//===(3)
	int newRank = 0;
	for (iter = G1RankToWorld.begin(); iter != G1RankToWorld.end(); iter++)
	{
		set[newRank] = iter->second;
		newRank++;
	}

	//== Create the group
	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group* info = new Group ();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupDifference
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupDifference (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType group1,
		MustGroupType group2,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
	//Not checked atm
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
    }

	//==Find information on oldGroup
	I_GroupTable* old1 = getGroupForHandle (pId, group1);
	I_GroupTable* old2 = getGroupForHandle (pId, group2);
	if (!old1 || !old2) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;

	/*
	 * (1) First we put tuples of the form (worldRank, groupRank)
	 * into a map by going over group1. Afterwards we go over
	 * group2 and remove all entries in the map that are also in
	 * group2. Finally, we use a second map to order the entries
	 * by their groupRank.
	 * (2)Then we resize the new set with the size of the second map
	 * and (3) loop over the second map to put in the entries.
	 * (We must not use the groupRank in the second map, as these
	 * might not be contiguous)
	 */

	//===(1)
	std::map<int,int> worldToG1Rank;
	std::map<int,int> g1RankToWorld;
	std::map<int,int>::iterator iter;

	// Group1
	for (int i = 0; i < old1->getSize(); i++)
	{
		//translate
		int worldTranslate;
		if (!old1->translate(i, &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous

		//No duplicates yet
		worldToG1Rank.insert (std::make_pair (worldTranslate, i));
	}

	// Group2
	for (int i = 0; i < old2->getSize(); i++)
	{
		//translate
		int worldTranslate;
		if (!old2->translate(i, &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous

		//Check if in the first group
		iter = worldToG1Rank.find(worldTranslate);
		if (iter == worldToG1Rank.end())
			continue;

		//Remove from first map
		worldToG1Rank.erase (iter);
	}

	//Create second map
	for (iter = worldToG1Rank.begin(); iter != worldToG1Rank.end(); iter++)
	{
		g1RankToWorld.insert(std::make_pair (iter->second, iter->first));
	}

	//===(2)
	set.resize(g1RankToWorld.size());

	//===(3)
	int newRank = 0;
	for (iter = g1RankToWorld.begin(); iter != g1RankToWorld.end(); iter++)
	{
		set[newRank] = iter->second;
		newRank++;
	}

	//== Create the group
	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group *info = new Group ();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupIncl
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupIncl (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType oldGroup,
		int n,
		const int *ranks,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
	//Not checked atm
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
    }

	//==Find information on oldGroup
	I_GroupTable* old = getGroupForHandle (pId, oldGroup);
	if (!old) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;
	set.resize(n);

	for (int i = 0; i < n; i++)
	{
		int worldTranslate;
		if (!old->translate(ranks[i], &worldTranslate))
			return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous
		set[i] = worldTranslate;
	}

	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group* info = new Group();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupExcl
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupExcl (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType oldGroup,
		int n,
		const int* ranks,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
	//Not checked atm
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
    }

	//==Find information on oldGroup
	I_GroupTable* old = getGroupForHandle (pId, oldGroup);
	if (!old) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;
	int newSize = old->getSize() - n;

	if (newSize > 0)
		set.resize(newSize); //holds as all entries in ranks must be valid and distinct

	//first create a map with the ranks to exclude (its ordered)
	std::map<int,int> exclMap;
	std::map<int,int>::iterator iter;
	for (int i = 0; i < n; i++)
	{
		exclMap.insert (std::make_pair (ranks[i], ranks[i]));
	}

	//Go over the ranks in old group, skip if in map, otherwise add
	iter = exclMap.begin();
	int rank =0;
	for (int i = 0; i < old->getSize(); i++)
	{
		if (iter != exclMap.end () && i == iter->first)
		{
			iter++;
			continue;
		}

		int worldRank;
		if (!old->translate(i,&worldRank))
			return GTI_ANALYSIS_SUCCESS;

		set[rank] = worldRank;
		rank++;
	}

	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group* info = new Group();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupRangeIncl
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupRangeIncl (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType oldGroup,
		int n,
		const int *ranges,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
	//Not checked atm
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
    }

	//==Find information on oldGroup
	I_GroupTable* old = getGroupForHandle (pId, oldGroup);
	if (!old) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;

	/*
	 * 1) Determine the size of the set
	 *
	 * number of elements in ranges i:
	 * ceil((last_i - first_i + X)/stride)
	 * X is +1 if stride > 0
	 * X is -1 if stride < 0
	 */
	int size = 0;
	for (int i = 0; i < n; i++)
	{
		int first   = ranges[3*i+0],
				last    = ranges[3*i+1],
				stride = ranges[3*i+2];

		int X = 1;
		if (stride < 0) X = -1;

		int nTemp = (last - first + X)/stride;

		if (nTemp*stride != last - first + X) //manual ceil
			nTemp++;

		size += nTemp;
	}

	set.resize(size);

	/*
	 * 2) Go over all ranges and put the corresponding ranks into
	 *     the set.
	 */
	int newRank = 0;
	for (int i = 0; i < n; i++)
	{
		int 	first   = ranges[3*i+0],
				last    = ranges[3*i+1],
				stride = ranges[3*i+2];

		int X = 1;
		if (stride < 0) X = -1;

		for (int curRank = first; curRank*X <= last*X; curRank += stride)
		{
			int worldTranslate;
			if (!old->translate(curRank, &worldTranslate))
				return GTI_ANALYSIS_SUCCESS; //There was a rank in ranks[i] that was not in the old group, this is erroneous
			set[newRank] = worldTranslate;
			newRank ++;
		}
	}

	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group* info = new Group ();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupRangeExcl
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupRangeExcl (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType oldGroup,
		int n,
		const int *ranges,
		MustGroupType newGroup)
{
	//==Should not be known yet (except MPI_GROUP_EMPTY)
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return GTI_ANALYSIS_SUCCESS;
    }

	//==Find information on oldGroup
	I_GroupTable* old = getGroupForHandle (pId, oldGroup);
	if (!old) return GTI_ANALYSIS_SUCCESS; //=>Some error, return silently, errors will be catched by checks

	//==Create the new I_GroupTable
	std::vector<int> set;

	/*
	 * 1)
	 * Loop over the ranges and expand them into ranks.
	 * Put the ranks into a map of ranks to exclude,
	 * we will later on use that the map is ordered.
	 */
	std::map<int,int> exclMap;
	std::map<int,int>::iterator iter;
	for (int i = 0; i < n; i++)
	{
		int 	first   = ranges[3*i+0],
				last    = ranges[3*i+1],
				stride = ranges[3*i+2];

		int X = 1;
		if (stride < 0) X = -1;

		for (int curRank = first; curRank*X <= last*X; curRank += stride)
		{
			exclMap.insert(std::make_pair(curRank, curRank));
		}
	}

	/*
	 * 2) size the new set
	 */
	int newSize = old->getSize() - exclMap.size();
	if (newSize > 0)
		set.resize(newSize);

	/**
	 * 3) Go over the old group and exclude the ranks in the map
	 */
	iter = exclMap.begin();
	int rank =0;
	for (int i = 0; i < old->getSize(); i++)
	{
		if (iter != exclMap.end () && i == iter->first)
		{
			iter++;
			continue;
		}

		int worldRank;
		if (!old->translate(i,&worldRank))
			return GTI_ANALYSIS_SUCCESS;

		set[rank] = worldRank;
		rank++;
	}

	/*
	 * 4) set the group accordingly
	 */
	I_GroupTable *newG = getGroupTable (set);

	//==Create the full info
	Group* info = new Group ();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myGroup = newG;

	//==Store the full info in the user handles
	submitUserHandle (pId, newGroup, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// groupFree
//=============================
GTI_ANALYSIS_RETURN GroupTrack::groupFree (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType group)
{
    removeUserHandle (pId, group);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getGroup
//=============================
I_Group* GroupTrack::getGroup (
		MustParallelId pId,
		MustGroupType group)
{
	return getGroup (pId2Rank(pId), group);
}

//=============================
// getGroup
//=============================
I_Group* GroupTrack::getGroup (
		int rank,
		MustGroupType group)
{
	return getHandleInfo (rank, group);
}

//=============================
// getPersistentGroup
//=============================
I_GroupPersistent* GroupTrack::getPersistentGroup (
        MustParallelId pId,
        MustGroupType group)
{
    return getPersistentGroup (pId2Rank(pId), group);
}

//=============================
// getPersistentGroup
//=============================
I_GroupPersistent* GroupTrack::getPersistentGroup (
        int rank,
        MustGroupType group)
{
    Group* ret = getHandleInfo (rank, group);
    if (ret) ret->incRefCount();
    return ret;
}

//=============================
// getGroupTable
//=============================
I_GroupTable* GroupTrack::getGroupTable (int intervalBegin, int intervalEnd)
{
	GroupTableList *list;
	GroupTableList::iterator iter;

	//Search for the group
	if (isGroupTableKnown  (intervalBegin, intervalEnd, &list, &iter))
	{
		(*iter)->copy();
		I_GroupTable* ret = *iter;
		return ret;
	}

	//Add as new group
	I_GroupTable* group;
	group = new GroupTable (intervalBegin, intervalEnd, this);
	addGroupTable (group);

	return group;
}

//=============================
// getGroupTable
//=============================
I_GroupTable* GroupTrack::getGroupTable (std::vector<int> set)
{
	GroupTableList *list;
	GroupTableList::iterator iter;

	//Search for the group
	if (isGroupTableKnown  (&set, &list, &iter))
	{
		(*iter)->copy();
		I_GroupTable* ret = *iter;
		return ret;
	}

	//Add as new group
	I_GroupTable* group;
	group = new GroupTable (set, this);
	addGroupTable (group);

	return group;
}

//=============================
// getGroupTable
//=============================
I_GroupTable* GroupTrack::getGroupTable (
        int rank,
        MustRemoteIdType remoteId)
{
    RemoteTableIdentifier id = std::make_pair (rank, remoteId);
    RemoteMapType::iterator pos = myRemoteTables.find (id);

    //Not found
    if (pos == myRemoteTables.end())
        return NULL;

    //Valid?
    if (!pos->second)
        return NULL;

    //Found, increment ref count
    pos->second->copy();

    return pos->second;
}

//=============================
// deleteGroupTable
//=============================
void GroupTrack::deleteGroupTable (I_GroupTable* group)
{
	GroupTableList *list;
	GroupTableList::iterator iter;

	//Search for the group
	if (!isGroupTableKnown  (group, &list, &iter))
	{
		//Internal error
		//std::cerr << "Internal Error: tried to free a group that does not exists. (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
		//assert (0);
	    //This may happen if the destruction order is unlucky
	    return;
	}

	list->erase(iter);
	//DEBUGGING: printCollection ();
}

//=============================
// commGroup
//=============================
void GroupTrack::commGroup (
		MustParallelId pId,
		MustLocationId lId,
		must::I_GroupTable *commGroup,
		MustGroupType newGroup)
{
    //Check whether this group handle was already known
    Group* newInfo = getHandleInfo (pId, newGroup);

    if (newInfo)
    {
        //If already known, it is NULL, EMPTY, or a user defined
        //For NULL and EMPTY nothing to do

        //For user handles, inc the MPI ref count
        if (!newInfo->isEmpty() && !newInfo->isNull())
            newInfo->mpiIncRefCount();

        return;
    }

    //Create the new group info
	Group* info = new Group ();

	info->myIsNull = false;
	info->myIsEmpty = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;

	info->myGroup = commGroup;
	if (info->myGroup)
	    info->myGroup->copy();

	//Add to user map
	submitUserHandle (pId, newGroup, info);
}

//=============================
// addRemoteGroupTableRep1
//=============================
GTI_ANALYSIS_RETURN GroupTrack::addRemoteGroupTableRep2 (
        int rank,
        MustRemoteIdType remoteId,
        int size,
        int* translation)
{
    std::vector<int> transSet = std::vector<int> ();
    transSet.resize (size);

    for (int i = 0; i < size; i++)
        transSet[i] = translation[i];

    I_GroupTable *table = getGroupTable (transSet);
    RemoteTableIdentifier id = std::make_pair (rank, remoteId);
    myRemoteTables.insert (std::make_pair (id, table));

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteGroupTableRep2
//=============================
GTI_ANALYSIS_RETURN GroupTrack::addRemoteGroupTableRep1 (
        int rank,
        MustRemoteIdType remoteId,
        int beginRank,
        int endRank)
{
    I_GroupTable *table = getGroupTable (beginRank, endRank);
    RemoteTableIdentifier id = std::make_pair (rank, remoteId);
    myRemoteTables.insert (std::make_pair (id, table));

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// freeRemoteGroupTable
//=============================
GTI_ANALYSIS_RETURN GroupTrack::freeRemoteGroupTable (
        int rank,
        MustRemoteIdType remoteId)
{
    RemoteTableIdentifier id = std::make_pair (rank, remoteId);

    RemoteMapType::iterator pos = myRemoteTables.find (id);

    if (pos == myRemoteTables.end())
    {
        std::cerr << "Error: received a freeRemoteGroupTable for an unknown group table!" << std::endl;
        assert (0);
        return GTI_ANALYSIS_SUCCESS;
    }

    //Decrement the ref count of the group table
    pos->second->erase();

    //Remove it from our remote map
    myRemoteTables.erase(pos);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// freeRemoteGroupTable
//=============================
bool GroupTrack::passGroupTableAcross (
        MustParallelId pId,
        I_GroupTable *group,
        int toPlaceId,
        MustRemoteIdType* remoteId)
{
    return passGroupTableAcross (pId2Rank(pId), group, toPlaceId, remoteId);
}

//=============================
// freeRemoteGroupTable
//=============================
bool GroupTrack::passGroupTableAcross (
        int rank,
        I_GroupTable *group,
        int toPlaceId,
        MustRemoteIdType* remoteId)
{
    //Do we have wrap-across at all?
    if (!myPassTableAcrossFunc1 || !myPassTableAcrossFunc2)
        return false;

    //Cast to internal representation
    GroupTable* table = (GroupTable*) group; //Ugly, we need to cast to the actual implementation here

    //Store the remote id in the output argument
    if (remoteId)
        *remoteId = table->getRemoteId();

    //Did we already pass this group table?
    if (table->wasForwardedToPlace(toPlaceId, rank))
        return true;

    //No base resources to pass on!

    //Prepare the call, depending on representation
    if (table->myBeginRank >= 0)
    {
        //Representation 1
        (*myPassTableAcrossFunc1) (
                    rank,
                    table->getRemoteId(),
                    table->myBeginRank,
                    table->myEndRank,
                    toPlaceId
                );
    }
    else
    {
        //Representation 2
        int size = table->mySet.size();
        int *translation = new int[size];

        const std::vector<int>& mapping = table->getMapping();

        for (int i = 0; i < size; i++)
            translation[i] = mapping[i];

        (*myPassTableAcrossFunc2) (
                    rank,
                    table->getRemoteId(),
                    size,
                    translation,
                    toPlaceId
            );

        //Free temp memory
        delete[] translation;
    }

    //Tell the group table that we passed it across
    table->setForwardedToPlace(toPlaceId, rank, myFreeTableAcrossFunc);

    return true;
}

//=============================
// isGroupSelectionKnown
//=============================
bool GroupTrack::isGroupSelectionKnown (int groupSize, int firstWorldRank, int lastWorldRank, GroupTableList **pList)
{
	//Do we already have such a group in out collection ?
	GroupTableCollection::iterator posC = myGroupTables.find(groupSize);

	//No GroupTableSelection for this size present yet
	if (posC == myGroupTables.end())
	{
		return false;
	}

	GroupTableSelection::iterator posS = posC->second.find(std::make_pair(firstWorldRank, lastWorldRank));

	//No GroupTableList present for this begin+end rank yet
	if (posS == posC->second.end())
	{
		return false;
	}

	if (pList)
		*pList = &(posS->second);

	return true;
}

//=============================
// isGroupTableKnown
//=============================
bool GroupTrack::isGroupTableKnown  (I_GroupTable* group, GroupTableList **pList, GroupTableList::iterator *pListIter)
{
	GroupTableList *list;
	int worldFirstRank = 0, worldLastRank = 0;
	group->translate(0, &worldFirstRank);
	group->translate(group->getSize()-1, &worldLastRank);
	if (!isGroupSelectionKnown (group->getSize(), worldFirstRank, worldLastRank, &list))
		return false;

	GroupTableList::iterator iter;
	for (iter = list->begin(); iter != list->end(); iter++)
	{
		if ((*iter)->isEqual(group))
		{
			if (pList)
				*pList = list;
			if (pListIter)
				*pListIter = iter;
			return true;
		}
	}
	return false;
}

//=============================
// isGroupTableKnown
//=============================
bool GroupTrack::isGroupTableKnown  (int beginRank, int endRank, GroupTableList **pList, GroupTableList::iterator *pListIter)
{
	GroupTableList *list;
	if (!isGroupSelectionKnown (endRank - beginRank + 1, beginRank, endRank, &list))
		return false;

	GroupTableList::iterator iter;
	for (iter = list->begin(); iter != list->end(); iter++)
	{
		if ((*iter)->isEqual(beginRank, endRank))
		{
			if (pList)
				*pList = list;
			if (pListIter)
				*pListIter = iter;
			return true;
		}
	}
	return false;
}

//=============================
// isGroupTableKnown
//=============================
bool GroupTrack::isGroupTableKnown  (std::vector<int> *set, GroupTableList **pList, GroupTableList::iterator *pListIter)
{
	GroupTableList *list;
	int worldFirstRank = 0, worldLastRank = 0;
	if (set->size() > 0)
	{
		worldFirstRank = (*set)[0];
		worldLastRank = (*set)[set->size()-1];
	}
	if (!isGroupSelectionKnown (set->size(), worldFirstRank, worldLastRank, &list))
		return false;

	GroupTableList::iterator iter;
	for (iter = list->begin(); iter != list->end(); iter++)
	{
		if ((*iter)->isEqual(set))
		{
			if (pList)
				*pList = list;
			if (pListIter)
				*pListIter = iter;
			return true;
		}
	}
	return false;
}

//=============================
// addGroupTable
//=============================
bool GroupTrack::addGroupTable (I_GroupTable *group)
{
	int worldFirstRank = 0, worldLastRank = 0;
	group->translate(0, &worldFirstRank);
	group->translate(group->getSize()-1, &worldLastRank);

	//Do we already have such a group in out collection ?
	GroupTableCollection::iterator posC = myGroupTables.find(group->getSize());

	//No GroupTableSelection for this size present yet
	if (posC == myGroupTables.end())
	{
		GroupTableList list;
		list.push_back(group);

		GroupTableSelection selection;
		selection.insert(std::make_pair(
				std::make_pair(worldFirstRank, worldLastRank),
				list));

		myGroupTables.insert (std::make_pair (group->getSize(), selection));
		return true;
	}

	GroupTableSelection::iterator posS = posC->second.find(std::make_pair(worldFirstRank, worldLastRank));

	//No GroupTableList present for this begin+end rank yet
	if (posS == posC->second.end())
	{
		GroupTableList list;
		list.push_back(group);

		posC->second.insert (std::make_pair (
				std::make_pair (worldFirstRank, worldLastRank),
				list));
		return true;
	}

	//Add as last entry in list
	posS->second.push_back(group);

	return true;
}

//=============================
// fillPredefinedInfos
//=============================
Group* GroupTrack::createPredefinedInfo (int predefEnum, MustGroupType handle)
{
    //Creates either the null or the empty handle

    Group* ret = new Group ();
    ret->myIsEmpty = false;
    ret->myIsNull = true;
    ret->myGroup = NULL;

    if (predefEnum == MUST_MPI_GROUP_EMPTY && handle != myNullValue)
    {
        ret->myIsEmpty = true;
        ret->myIsNull = false;
        ret->myGroup = getGroupTable (std::vector<int> ());
    }

    return ret;
}

//=============================
// printCollection
//=============================
void GroupTrack::printCollection (void)
{
	//The collection
	GroupTableCollection::iterator i;
	for (i = myGroupTables.begin(); i != myGroupTables.end(); i++)
	{
		GroupTableSelection::iterator j;
		for (j = i->second.begin(); j != i->second.end(); j++)
		{
			GroupTableList::iterator k;
			for (k = j->second.begin(); k != j->second.end(); k++)
			{
				I_GroupTable *group = *k;
				if (group)
				{
					int count = group->copy();//TODO this should return the refcount, is broken right now
					count --;
					group->erase();
					std::cout << "[" << i->first << "]->[" << j->first.first << "," << j->first.second << "]->group with size=" << group->getSize() << " pointer=" << (unsigned long)group << " count=" << count << std::endl;
				}
				else
				{
					std::cout << "Group with null pointer in collection!" << std::endl;
				}
			}
		}
	}

	//The current list of handles
	HandleMap::iterator iter;
	for (iter = myUserHandles.begin(); iter != myUserHandles.end(); iter++)
	{
		std::cout << "Handle " << iter->first.second << "@" << iter->first.first << " groupPointer=" << (unsigned long)iter->second->myGroup << std::endl;
		int count = (int)iter->second->myGroup->copy(); //TODO this should return the refcount, is broken right now
		count --;
		iter->second->myGroup->erase();
		std::cout << "Handle EXT count=" << count << std::endl;
	}
}

//=============================
// getGroupForHandle
//=============================
I_GroupTable* GroupTrack::getGroupForHandle (MustParallelId pId, MustGroupType handle)
{
    Group* info= getHandleInfo (pId, handle);
    if (!info)
        return NULL;

    return info->myGroup;
}

/*EOF*/
