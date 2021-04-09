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
 * @file CallReturnInput.cpp
 * 		@see gti::weaver::CallReturnInput
 *
 * @author Tobias Hilbrich
 * @date 03.08.2010
 */

#include "CallReturnInput.h"
#include <stdio.h>

using namespace gti::weaver::calls;

//=============================
// CallReturnInput
//=============================
CallReturnInput::CallReturnInput ( )
 : myTargetCall (NULL)
{

}

//=============================
// CallReturnInput
//=============================
CallReturnInput::CallReturnInput (Call* call)
 : myTargetCall (call)
{

}

//=============================
// ~CallReturnInput
//=============================
CallReturnInput::~CallReturnInput ( )
{
	myTargetCall = NULL;
}

//=============================
// getCallName
//=============================
std::string CallReturnInput::getCallName (void)
{
	return myTargetCall->getName();
}

//=============================
// getName
//=============================
std::string CallReturnInput::getName ( ) const
{
   // name of return variable according to GTI/system-builder/wrapper-generator/WrapperGenerator.cpp
	return "ret" + myTargetCall->getName();
}

//=============================
// getType
//=============================
std::string CallReturnInput::getType ( ) const
{
	return "const char*";
}

//=============================
// isArrayInput
//=============================
bool CallReturnInput::isArrayInput () const
{
	return true;
}

//=============================
// getLenName
//=============================
std::string CallReturnInput::getLenName () const
{
	char temp[64];
	sprintf (temp, "%ld", getName().length());
	return temp;
}

//=============================
// print
//=============================
std::ostream& CallReturnInput::print (std::ostream& out) const
{
	out << "callNameInput{";
	Input::print(out);
	out << "}";

	return out;
}

//=============================
// getDotInputNodeName
//=============================
std::string CallReturnInput::getDotInputNodeName (std::string callName)
{
	return callName + ":" + callName;
}

/*EOF*/
