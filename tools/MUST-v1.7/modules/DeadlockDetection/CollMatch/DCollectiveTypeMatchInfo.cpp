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
 * @file DCollectiveTypeMatchInfo.cpp
 *       @see must::DCollectiveTypeMatchInfo.
 *
 *  @date 29.05.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#include "DCollectiveTypeMatchInfo.h"

using namespace must;

//=============================
// handleIntraTypeMatchInfo
//=============================
DCollectiveTypeMatchInfo::DCollectiveTypeMatchInfo (
                int rank,
                MustParallelId pId,
                MustLocationId lId,
                I_CommPersistent *comm,
                I_DatatypePersistent *type,
                int numCounts,
                int *counts,
                int firstRank,
                int collectiveNumber,
                MustCollCommType collId)
 : myRank (rank),
   myPId (pId),
   myLId (lId),
   myComm (comm),
   myType (type),
   myTypes (NULL),
   myNumCounts (numCounts),
   myCounts (NULL),
   myFirstRank (firstRank),
   myCollectiveNumber (collectiveNumber),
   myCollId (collId)
{
    myCounts = new int[numCounts];

    for (int i = 0; i < numCounts; i++)
    {
        myCounts[i] = counts[i];
    }
}

//=============================
// handleIntraTypeMatchInfo
//=============================
DCollectiveTypeMatchInfo::DCollectiveTypeMatchInfo (
                int rank,
                MustParallelId pId,
                MustLocationId lId,
                I_CommPersistent *comm,
                int numCounts,
                I_DatatypePersistent **types,
                int *counts,
                int firstRank,
                int collectiveNumber,
                MustCollCommType collId)
 : myRank (rank),
   myPId (pId),
   myLId (lId),
   myComm (comm),
   myType (NULL),
   myTypes (types),
   myNumCounts (numCounts),
   myCounts (NULL),
   myFirstRank (firstRank),
   myCollectiveNumber (collectiveNumber),
   myCollId (collId)
{
    myCounts = new int[numCounts];

    for (int i = 0; i < numCounts; i++)
    {
        myCounts[i] = counts[i];
    }
}

//=============================
// handleIntraTypeMatchInfo
//=============================
DCollectiveTypeMatchInfo::~DCollectiveTypeMatchInfo ()
{
    if (myCounts) delete [] myCounts;
    myCounts = NULL;

    if (myTypes)
    {
        for (int i = 0; i < myNumCounts; i++)
            myTypes[i]->erase();

        delete [] myTypes;
    }
    myTypes = NULL;

    if (myComm) myComm->erase();
    myComm = NULL;

    if (myType) myType->erase();
    myType = NULL;
}

//=============================
// getWaveNumber
//=============================
int DCollectiveTypeMatchInfo::getWaveNumber (void)
{
    return myCollectiveNumber;
}

//=============================
// getCollId
//=============================
MustCollCommType DCollectiveTypeMatchInfo::getCollId (void)
{
    return myCollId;
}

//=============================
// getNumCounts
//=============================
int DCollectiveTypeMatchInfo::getNumCounts (void)
{
    return myNumCounts;
}

//=============================
// getCounts
//=============================
int* DCollectiveTypeMatchInfo::getCounts (void)
{
    return myCounts;
}

//=============================
// getFirstRank
//=============================
int DCollectiveTypeMatchInfo::getFirstRank (void)
{
    return myFirstRank;
}

//=============================
// hasTypes
//=============================
bool DCollectiveTypeMatchInfo::hasTypes (void)
{
    return myTypes != NULL;
}

//=============================
// getTypes
//=============================
I_DatatypePersistent** DCollectiveTypeMatchInfo::getTypes (void)
{
    return myTypes;
}

//=============================
// getType
//=============================
I_DatatypePersistent* DCollectiveTypeMatchInfo::getType (void)
{
    return myType;
}

//=============================
// getPId
//=============================
MustParallelId DCollectiveTypeMatchInfo::getPId (void)
{
    return myPId;
}

//=============================
// getLId
//=============================
MustLocationId DCollectiveTypeMatchInfo::getLId (void)
{
    return myLId;
}

//=============================
// getRank
//=============================
int DCollectiveTypeMatchInfo::getRank (void)
{
    return myRank;
}

/*EOF*/
