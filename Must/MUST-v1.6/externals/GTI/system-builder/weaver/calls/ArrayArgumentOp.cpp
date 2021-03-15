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
 * @file ArrayArgumentOp.cpp
 * 		@see gti::weaver::ArrayArgumentOp
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "ArrayArgumentOp.h"

using namespace gti::weaver::calls;

//=============================
// constructor
//=============================
ArrayArgumentOp::ArrayArgumentOp ( )
 : Argument (),
   myLengthOp (NULL),
   myId(-1),
   myWantOpLen(false)
{
	/*Nothing to do*/
}

//=============================
// constructor
//=============================
ArrayArgumentOp::ArrayArgumentOp (
		std::string name,
		std::string type,
		ArgumentIntent intent,
		Operation* lengthOp,
		int id,
		std::string typeAfterArg)
 : Argument (name, type, intent, typeAfterArg),
   myLengthOp(lengthOp),
   myId(id),
   myWantOpLen(false)
{
	/*Nothing to do*/
}

//=============================
// constructor
//=============================
ArrayArgumentOp::ArrayArgumentOp (
		std::string name,
		std::string type,
		ArgumentIntent intent,
		Operation* lengthOp,
		int id,
		bool wantOpLen,
		std::string typeAfterArg)
 : Argument (name, type, intent, typeAfterArg),
   myLengthOp(lengthOp),
   myId(id),
   myWantOpLen(wantOpLen)
{
	/*Nothing to do*/
}

//=============================
// destructor
//=============================
ArrayArgumentOp::~ArrayArgumentOp ( )
{
	myLengthOp = NULL;
}

//=============================
// setLengthOp
//=============================
void ArrayArgumentOp::setLengthOp ( Operation * new_var )
{
	myLengthOp = new_var;
}

//=============================
// getLengthOp
//=============================
Operation * ArrayArgumentOp::getLengthOp ( )
{
	return myLengthOp;
}

//=============================
// isArray
//=============================
bool ArrayArgumentOp::isArray (void) const
{
	return true;
}

//=============================
// getLengthVariable
//=============================
std::string ArrayArgumentOp::getLengthVariable (void)
{
	if (myWantOpLen)
		return myLengthOp->getResultLenVarName(myId);

	return myLengthOp->getResultVarName(myId);
}

//=============================
// getLengthVariableDotName
//=============================
std::string ArrayArgumentOp::getLengthVariableDotName (std::string callName)
{
	return myLengthOp->getDotNodeName(myId);
}

//=============================
// print
//=============================
std::ostream& ArrayArgumentOp::print (std::ostream& out) const
{
	out << "arrayArgument={";
	Argument::print(out);
	out
		<< ", inputOperation=" << myLengthOp->getName()
		<< ", id=" << myId;

	if (myWantOpLen)
		out << ", usesReturnedArrayLengthOfOperation";

	out << "}";

	return out;
}

//=============================
// isArrayWithLengthOp
//=============================
bool ArrayArgumentOp::isArrayWithLengthOp (void) const
{
	return true;
}

//=============================
// getMappingId
//=============================
int ArrayArgumentOp::getMappingId (void)
{
	return myId;
}

/*EOF*/
