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
 * @file CallIdInput.cpp
 * 		@see gti::weaver::CallIdInput
 *
 * @author Tobias Hilbrich
 * @date 07.01.2011
 */

#include "CallIdInput.h"
#include <stdio.h>

using namespace gti::weaver::calls;

//=============================
// CallIdInput
//=============================
CallIdInput::CallIdInput ( )
 : myTargetCall (NULL)
{

}

//=============================
// CallIdInput
//=============================
CallIdInput::CallIdInput (Call* call)
 : myTargetCall (call)
{

}

//=============================
// ~CallIdInput
//=============================
CallIdInput::~CallIdInput ( )
{
	myTargetCall = NULL;
}

//=============================
// getCallId
//=============================
int CallIdInput::getCallId (void) const
{
	return myTargetCall->getUniqueId ();
}

//=============================
// getName
//=============================
std::string CallIdInput::getName ( ) const
{
	char temp[128];
	sprintf (temp, "%d", getCallId());
	return temp;
}

//=============================
// getType
//=============================
std::string CallIdInput::getType ( ) const
{
	return "int";
}

//=============================
// print
//=============================
std::ostream& CallIdInput::print (std::ostream& out) const
{
	out << "callIdInput{";
	Input::print(out);
	out << "}";

	return out;
}

//=============================
// getDotInputNodeName
//=============================
std::string CallIdInput::getDotInputNodeName (std::string callName)
{
	return callName + ":" + callName;
}

/*EOF*/
