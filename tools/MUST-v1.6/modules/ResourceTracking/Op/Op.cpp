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
 * @file Op.cpp
 *       @see Op.
 *
 *  @date 19.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "Op.h"

#include <sstream>

using namespace must;

//=============================
// Constructor (MPI_OP_NULL)
//=============================
Op::Op ()
 : HandleInfoBase ("Op"),
   myPredefined (MUST_MPI_OP_SUM),
   myPredefinedName (""),
   myIsNull (true),
   myIsPredefined (false),
   myIsCommutative (false),
   myCreationPId (0),
   myCreationLId (0)
{

}

//=============================
// Constructor (Predefined Op)
//=============================
Op::Op (MustMpiOpPredefined predefined, const char* predefinedName)
 : HandleInfoBase ("Op"),
   myPredefined (predefined),
   myPredefinedName (predefinedName),
   myIsNull (false),
   myIsPredefined (true),
   myIsCommutative (true),
   myCreationPId (0),
   myCreationLId (0)
{
    //TODO: are all predefined ops commutative ?
}

//=============================
// ~Op
//=============================
Op::~Op ()
{
}

//=============================
// isNull
//=============================
bool Op::isNull (void)
{
    return myIsNull;
}

//=============================
// isPredefined
//=============================
bool Op::isPredefined (void)
{
    return myIsPredefined;
}

//=============================
// isCommutative
//=============================
bool Op::isCommutative (void)
{
    return myIsCommutative;
}

//=============================
// getCreationPId
//=============================
MustParallelId Op::getCreationPId (void)
{
    return myCreationPId;
}

//=============================
// getCreationLId
//=============================
MustLocationId Op::getCreationLId (void)
{
    return myCreationLId;
}

//=============================
// getPredefinedInfo
//=============================
MustMpiOpPredefined Op::getPredefinedInfo (void)
{
    return myPredefined;
}

//=============================
// getPredefinedName
//=============================
std::string Op::getPredefinedName (void)
{
    return myPredefinedName;
}

//=============================
// printInfo
//=============================
bool Op::printInfo (
        std::stringstream &out,
        std::list<std::pair<MustParallelId,MustLocationId> > *pReferences)
{
    //Is Null
    if (myIsNull)
    {
        out << "MPI_OP_NULL";
        return true;
    }

    //Is Predefined
    if (myIsPredefined)
    {
        out << getPredefinedName();
        return true;
    }

    //A user defined operation
    pReferences->push_back(std::make_pair (myCreationPId, myCreationLId));
    out << "Operation created at reference  "<< pReferences->size();

    return true;
}

//=============================
// getResourceName
//=============================
std::string Op::getResourceName (void)
{
    return "Op";
}

//=============================
// operator ==
//=============================
bool Op::operator == (I_Op &other)
{
	if (isPredefined() || other.isPredefined() )
	{
		if (! isPredefined() ||
			! other.isPredefined() ||
			getPredefinedInfo() != other.getPredefinedInfo())
		{
			return false;
		}
	}
	return true;
}

//=============================
// operator !=
//=============================
bool Op::operator != (I_Op &other)
{
	return ! this->operator==(other);
}

/*EOF*/
