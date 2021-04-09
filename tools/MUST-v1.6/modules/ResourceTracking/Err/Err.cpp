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
 * @file Err.h
 *       @see Err.
 *
 *  @date 19.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "Err.h"

#include <sstream>

using namespace must;

//=============================
// Constructor
//=============================
Err::Err ()
 : HandleInfoBase ("Err"),
   myPredefined (MUST_MPI_ERRORS_ARE_FATAL),
   myPredefinedName (""),
   myIsNull (true),
   myIsPredefined (false),
   myCreationPId (0),
   myCreationLId (0)
{
    //Nothing to do
}

//=============================
// Err
//=============================
Err::Err (MustMpiErrPredefined predefined, std::string predefinedName)
: HandleInfoBase ("Err"),
  myPredefined (predefined),
  myPredefinedName (predefinedName),
  myIsNull (false),
  myIsPredefined (true),
  myCreationPId (0),
  myCreationLId (0)
{
    //Nothing to do
}

//=============================
// ~Err
//=============================
Err::~Err ()
{
    //Nothing to do
}

//=============================
// isNull
//=============================
bool Err::isNull (void)
{
    return myIsNull;
}

//=============================
// isPredefined
//=============================
bool Err::isPredefined (void)
{
    return myIsPredefined;
}

//=============================
// getCreationPId
//=============================
MustParallelId Err::getCreationPId (void)
{
    return myCreationPId;
}

//=============================
// getCreationLId
//=============================
MustLocationId Err::getCreationLId (void)
{
    return myCreationLId;
}

//=============================
// getPredefinedInfo
//=============================
MustMpiErrPredefined Err::getPredefinedInfo (void)
{
    return myPredefined;
}

//=============================
// getPredefinedName
//=============================
std::string Err::getPredefinedName (void)
{
    return myPredefinedName;
}

//=============================
// printInfo
//=============================
bool Err::printInfo (
        std::stringstream &out,
        std::list<std::pair<MustParallelId,MustLocationId> > *pReferences)
{
    //Is Null
    if(myIsNull)
    {
        out << "MPI_ERRHANDLER_NULL";
        return true;
    }

    //Is Predefined
    if (myIsPredefined)
    {
        out << myPredefinedName;
        return true;
    }

    //A user defined errorhandler
    pReferences->push_back(std::make_pair (myCreationPId, myCreationLId));
    out << "Error handler created at reference  "<< pReferences->size();

    return true;
}

//=============================
// getResourceName
//=============================
std::string Err::getResourceName (void)
{
    return "Err";
}

/*EOF*/
