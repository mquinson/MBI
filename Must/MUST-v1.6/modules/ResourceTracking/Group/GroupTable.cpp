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
 * @file GroupTable.cpp
 *       @see MUST::GroupTable.
 *
 *  @date 07.03.2011
 *  @author Tobias Hilbrich, Mathias Korepkat
 */

#include "GroupTable.h"

#include <assert.h>

using namespace must;

//=============================
// Constructor
//=============================
GroupTable::GroupTable (std::vector<int> set, GroupTrack * track)
	: HandleInfoBase ("GroupTable"),
	  mySet (set),
	  myWorld2Rank (),
	  myBeginRank (-1),
	  myEndRank (-1),
	  myTrack (track)

{
    //We modify ref coutns here, group tables are purely internal and don't have an MPI ref count
    mpiRefCount = 0;
    userRefCount = 1;
	//nothing to do
}

//=============================
// Constructor
//=============================
GroupTable::GroupTable (int beginRank, int endRank, GroupTrack *track)
	: HandleInfoBase ("GroupTable"),
      mySet (),
	  myWorld2Rank (),
	  myBeginRank (beginRank),
	  myEndRank (endRank),
	  myTrack (track)
{
    //We modify ref coutns here, group tables are purely internal and don't have an MPI ref count
    mpiRefCount = 0;
    userRefCount = 1;
	assert (endRank >= beginRank);
}

//=============================
// Destructor
//=============================
GroupTable::~GroupTable (void)
{
	mySet.clear();
	myTrack = NULL;
}

//=============================
// translate
//=============================
bool GroupTable::translate (int rank, int *outWorldRank)
{
	//Representation 2:
	if (myBeginRank >= 0)
	{
		if (rank > myEndRank - myBeginRank)
			return false;

		if (outWorldRank)
			*outWorldRank = myBeginRank + rank;
		return true;
	}

	//Representation 1:
	if (rank >= mySet.size())
		return false;

	if (outWorldRank)
		*outWorldRank = mySet[rank];

	return true;
}

//=============================
// containsWorldRank
//=============================
bool GroupTable::containsWorldRank (int worldRank, int *outGroupRank)
{
    //Representation 2:
    if (myBeginRank >= 0)
    {
        if (worldRank > myEndRank || worldRank < myBeginRank)
            return false;

        if (outGroupRank)
            *outGroupRank = worldRank  - myBeginRank;
        return true;
    }

    //Representation 1:
    if (myWorld2Rank.size () == 0)
    {
        for (std::vector<int>::size_type i = 0; i < mySet.size(); i++)
        {
            myWorld2Rank.insert (std::make_pair (mySet[i],i));
        }
    }

    std::map<int, int>::iterator pos = myWorld2Rank.find(worldRank);
    if (pos == myWorld2Rank.end())
        return false;

    if (outGroupRank)
        *outGroupRank = pos->second;
    return true;
}

//=============================
// getSize
//=============================
int GroupTable::getSize ()
{
	//Representation 2:
	if (myBeginRank >= 0)
		return myEndRank - myBeginRank + 1;

	//Representation 1:
	return mySet.size();
}

//=============================
// isEqual
//=============================
bool GroupTable::isEqual (std::vector<int> *set)
{
	//does the size matches ?
	if (set->size() != getSize())
		return false;

	//Loop over all elements in the given set
	for (std::vector<int>::size_type i = 0; i < set->size(); i++)
	{
		int otherWorldRank = (*set)[i];

		if (myBeginRank < 0)
		{
			//Representation 1:
			if (mySet[i] != otherWorldRank)
				return false;
		}
		else
		{
			//Representation 2:
			if (myBeginRank + i != otherWorldRank)
				return false;
		}
	}

	return true;
}

//=============================
// isEqual
//=============================
bool GroupTable::isEqual (int beginRank, int endRank)
{
	//Representation 2:
	if (myBeginRank >= 0)
	{
		return (myBeginRank == beginRank && myEndRank == endRank);
	}

	//Representation 1:
	//Does the size matches ?
	if (getSize() != endRank - beginRank + 1)
		return false;

	//Loop over all elements in this set
	for (std::vector<int>::size_type i = 0; i < mySet.size(); i++)
	{
		if (mySet[i] != beginRank + i)
			return false;
	}

	return true;
}

//=============================
// isEqual
//=============================
bool GroupTable::isEqual (I_GroupTable* group)
{
	if (myBeginRank < 0)
		return group->isEqual(&mySet);
	return group->isEqual(myBeginRank, myEndRank);
}

//=============================
// getMapping
//=============================
const std::vector<int>& GroupTable::getMapping (void)
{
    //If we are in representation2, we have to switch to representation1 now
    if (myBeginRank >= 0)
    {
        mySet.resize(myEndRank - myBeginRank + 1);
        for (int i = 0; i <= myEndRank - myBeginRank; i++)
        {
            mySet[i] = myBeginRank + i;
        }

        myBeginRank = -1;
        myEndRank = -1;
    }

    return mySet;
}

//=============================
// deleteThis
//=============================
void GroupTable::deleteThis (void)
{
    if (ourAllowFreeForwarding && myTrack)
        myTrack->deleteGroupTable (this);

    HandleInfoBase::deleteThis();
}

//=============================
// getResourceName
//=============================
std::string GroupTable::getResourceName (void)
{
    return "GroupTable";
}

//=============================
// printInfo
//=============================
bool GroupTable::printInfo (
        std::stringstream &out,
        std::list<std::pair<MustParallelId,MustLocationId> > *pReferences)
{
    //TODO possibly we might be a bit more elaborate here
    out << "GroupTable";
    return true;
}

/*EOF*/
