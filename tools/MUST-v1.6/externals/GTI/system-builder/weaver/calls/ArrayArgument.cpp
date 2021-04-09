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
 * @file ArrayArgument.cpp
 * 		@see gti::weaver::ArrayArgument
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "ArrayArgument.h"

using namespace gti::weaver::calls;

//=============================
// constructor
//=============================
ArrayArgument::ArrayArgument ( )
 : Argument (),
   myLengthArgument (NULL)
{
	/*Nothing to do*/
}

//=============================
// constructor
//=============================
ArrayArgument::ArrayArgument (
		std::string name,
		std::string type,
		ArgumentIntent intent,
		Argument* lengthArgument,
		std::string typeAfterArg)
 : Argument (name, type, intent, typeAfterArg),
   myLengthArgument (lengthArgument)
{
	/*Nothing to do*/
}

//=============================
// destructor
//=============================
ArrayArgument::~ArrayArgument ( )
{
	myLengthArgument = NULL;
}

//=============================
// setLengthArgument
//=============================
void ArrayArgument::setLengthArgument ( Argument * new_var )
{
  myLengthArgument = new_var;
}

//=============================
// getLengthArgument
//=============================
Argument * ArrayArgument::getLengthArgument ( )
{
  return myLengthArgument;
}

//=============================
// isArray
//=============================
bool ArrayArgument::isArray (void) const
{
	return true;
}

//=============================
// getLengthVariable
//=============================
std::string ArrayArgument::getLengthVariable (void)
{
	return myLengthArgument->getName();
}

//=============================
// getLengthVariableDotName
//=============================
std::string ArrayArgument::getLengthVariableDotName (std::string callName)
{
	return callName + ":" + myLengthArgument->getName();
}

//=============================
// print
//=============================
std::ostream& ArrayArgument::print (std::ostream& out) const
{
	out << "arrayArgument={";
	Argument::print(out);
	out << ", lengthArgument=" << myLengthArgument->getName() << "}";

	return out;
}

/*EOF*/
