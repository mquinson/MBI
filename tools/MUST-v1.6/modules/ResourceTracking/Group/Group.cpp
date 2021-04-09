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
 * @file I_Group.cpp
 *       @see I_Group.
 *
 *  @date 15.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "Group.h"

#include <sstream>

using namespace must;

//=============================
// Group
//=============================
Group::Group ()
: HandleInfoBase ("Group"),
  myIsNull (true),
  myIsEmpty (false),
  myCreationPId (0),
  myCreationLId (0),
  myGroup (NULL)
{
    //Nothing to do
}

//=============================
// ~Group
//=============================
Group::~Group ()
{
    if (myGroup)
        myGroup->erase();
    myGroup = NULL;
}

//=============================
// isNull
//=============================
bool Group::isNull (void)
{
    return myIsNull;
}

//=============================
// isEmpty
//=============================
bool Group::isEmpty (void)
{
    return myIsEmpty;
}

//=============================
// getGroup
//=============================
I_GroupTable* Group::getGroup (void)
{
    return myGroup;
}

//=============================
// getCreationPId
//=============================
MustParallelId Group::getCreationPId (void)
{
    return myCreationPId;
}

//=============================
// getCreationLId
//=============================
MustLocationId Group::getCreationLId (void)
{
    return myCreationLId;
}

//=============================
// printInfo
//=============================
bool Group::printInfo (
        std::stringstream &out,
        std::list<std::pair<MustParallelId,MustLocationId> > *pReferences)
{
    //Is Null
    if(myIsNull)
    {
        out << "MPI_GROUP_NULL";
        return true;
    }

    //Is Predefined
    if (myIsEmpty)
    {
        out << "MPI_GROUP_EMPTY";
        return true;
    }

    //A user defined group
    pReferences->push_back(std::make_pair (myCreationPId, myCreationLId));
    out << "Group created at reference  "<< pReferences->size();

    if (myGroup)
    {
        out << " size=" << myGroup->getSize();
    }

    return true;
}

//=============================
// getResourceName
//=============================
std::string Group::getResourceName (void)
{
    return "Group";
}

/*EOF*/
