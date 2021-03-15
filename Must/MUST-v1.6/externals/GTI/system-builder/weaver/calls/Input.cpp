/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file Input.cpp
 * 		@see gti::weaver::Input
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Input.h"

using namespace gti::weaver::calls;

//=============================
// Constructor
//=============================
Input::Input ( )
{
	/*Nothing to do*/
}

//=============================
// Destructor
//=============================
Input::~Input ( )
{
	/*Nothing to do*/
}

//=============================
// print
//=============================
std::ostream& Input::print (std::ostream& out) const
{
	out << "name=" << getName () << ", type=" << getType();

	return out;
}

//=============================
// isOpArrayLen
//=============================
bool Input::isOpArrayLen (void) const
{
	return false;
}

//=============================
// needsOperation
//=============================
bool Input::needsOperation (Operation** pOutOp, int *pOutId)
{
	return false;
}

//=============================
// isArrayInput
//=============================
bool Input::isArrayInput () const
{
	return false;
}

//=============================
// getLenName
//=============================
std::string Input::getLenName () const
{
	return "";
}

/*EOF*/
