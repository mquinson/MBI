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
 * @file InputDescription.cpp
 * 		@see gti::weaver::InputDescription
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "InputDescription.h"

using namespace gti::weaver::analyses;

//=============================
// InputDescription
//=============================
InputDescription::InputDescription ( )
 : myArgumentType (""), myName ("")
{
	/*Nothing to do*/
}

//=============================
// InputDescription
//=============================
InputDescription::InputDescription (std::string argumentType, std::string name)
 : myArgumentType (argumentType), myName (name)
{
	/*Nothing to do*/
}

//=============================
// InputDescription
//=============================
InputDescription::~InputDescription ( )
{
	/*Nothing to do*/
}

//=============================
// setArgumentType
//=============================
void InputDescription::setArgumentType ( std::string new_var )
{
	myArgumentType = new_var;
}

//=============================
// getArgumentType
//=============================
std::string InputDescription::getArgumentType ( )
{
	return myArgumentType;
}

//=============================
// print
//=============================
std::ostream& InputDescription::print (std::ostream& out) const
{
	out << myArgumentType << ", " << myName;
	return out;
}

//=============================
// getName
//=============================
std::string InputDescription::getName (void)
{
	return myName;
}

/*EOF*/
