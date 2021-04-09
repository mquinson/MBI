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
 * @file CallNameInput.cpp
 * 		@see gti::weaver::CallNameInput
 *
 * @author Tobias Hilbrich
 * @date 03.08.2010
 */

#include "CallNameInput.h"
#include <stdio.h>

using namespace gti::weaver::calls;

//=============================
// CallNameInput
//=============================
CallNameInput::CallNameInput ( )
 : myTargetCall (NULL)
{

}

//=============================
// CallNameInput
//=============================
CallNameInput::CallNameInput (Call* call)
 : myTargetCall (call)
{

}

//=============================
// ~CallNameInput
//=============================
CallNameInput::~CallNameInput ( )
{
	myTargetCall = NULL;
}

//=============================
// getCallName
//=============================
std::string CallNameInput::getCallName (void)
{
	return myTargetCall->getName();
}

//=============================
// getName
//=============================
std::string CallNameInput::getName ( ) const
{
	return "\"" + myTargetCall->getName() + "\"";
}

//=============================
// getType
//=============================
std::string CallNameInput::getType ( ) const
{
	return "char*"; /**< removed "const ", I think we don't need it*/
}

//=============================
// isArrayInput
//=============================
bool CallNameInput::isArrayInput () const
{
	return true;
}

//=============================
// getLenName
//=============================
std::string CallNameInput::getLenName () const
{
	char temp[64];
	sprintf (temp, "%ld", getName().length());
	return temp;
}

//=============================
// print
//=============================
std::ostream& CallNameInput::print (std::ostream& out) const
{
	out << "callNameInput{";
	Input::print(out);
	out << "}";

	return out;
}

//=============================
// getDotInputNodeName
//=============================
std::string CallNameInput::getDotInputNodeName (std::string callName)
{
	return callName + ":" + callName;
}

/*EOF*/
