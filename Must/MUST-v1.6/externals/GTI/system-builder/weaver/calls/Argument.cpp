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
 * @file Argument.cpp
 * 		@see gti::weaver::Argument
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Argument.h"

using namespace gti::weaver::calls;

//=============================
// Constructor
//=============================
Argument::Argument ( )
 : myName (""),
   myType (""),
   myIntent (INTENT_IN),
   myTypeAfterArg ("")
{
	/*Nothing to do*/
}

//=============================
// Constructor
//=============================
Argument::Argument (
		std::string name,
		std::string type,
		ArgumentIntent intent,
		std::string typeAfterArg)
 : myName (name),
   myType (type),
   myIntent (intent),
   myTypeAfterArg (typeAfterArg)
{
	/*Nothing to do*/
}

//=============================
// Destructor
//=============================
Argument::~Argument ( )
{
	/*Nothing to do*/
}

//=============================
// setName
//=============================
void Argument::setName ( std::string new_var )
{
	myName = new_var;
}

//=============================
// getName
//=============================
std::string Argument::getName ( )
{
	return myName;
}

//=============================
// setType
//=============================
void Argument::setType ( std::string new_var )
{
	myType = new_var;
}

//=============================
// getType
//=============================
std::string Argument::getType ( )
{
	return myType;
}

//=============================
// setIntent
//=============================
void Argument::setIntent ( ArgumentIntent new_var )
{
	myIntent = new_var;
}

//=============================
// getIntent
//=============================
ArgumentIntent Argument::getIntent ( )
{
	return myIntent;
}

//=============================
// isArray
//=============================
bool Argument::isArray (void) const
{
	return false;
}

//=============================
// getLengthVariable
//=============================
std::string Argument::getLengthVariable (void)
{
	return "";
}

//=============================
// print
//=============================
std::ostream& Argument::print (std::ostream& out) const
{
	if (!isArray())
		out	<< "argument={";

	out
		<< "name=" << myName << ", "
		<< "type=" << myType << ", "
		<< "intent=";

	switch (myIntent)
	{
	case INTENT_IN:
		out << "in";
		break;
	case INTENT_OUT:
		out << "out";
		break;
	case INTENT_INOUT:
		out << "inout";
		break;
	}

	if (!isArray())
		out << "}";

	return out;
}

//=============================
// getArgumentDotName
//=============================
std::string Argument::getArgumentDotName (std::string callName)
{
	return callName + ":" + getName();
}

//=============================
// getLengthVariableDotName
//=============================
std::string Argument::getLengthVariableDotName (std::string callName)
{
	return "";
}

//=============================
// isArrayWithLengthOp
//=============================
bool Argument::isArrayWithLengthOp (void) const
{
	return false;
}

//=============================
// getTypeAfterArg
//=============================
std::string Argument::getTypeAfterArg (void)
{
	return myTypeAfterArg;
}

/*EOF*/
