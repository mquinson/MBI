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
 * @file ArgumentInput.cpp
 * 		@see gti::weaver::ArgumentInput
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "ArgumentInput.h"

#include "ArrayArgumentOp.h"

using namespace gti::weaver::calls;

//=============================
// ArgumentInput
//=============================
ArgumentInput::ArgumentInput ( )
 : Input (),
   myTargetArgument (NULL)
{

}

//=============================
// ArgumentInput
//=============================
ArgumentInput::ArgumentInput (Argument* argument)
 : Input (),
   myTargetArgument (argument)
{

}

//=============================
// ArgumentInput
//=============================
ArgumentInput::~ArgumentInput ( )
{
	myTargetArgument = NULL;
}

//=============================
// setTargetArgument
//=============================
void ArgumentInput::setTargetArgument ( Argument * new_var )
{
  myTargetArgument = new_var;
}

//=============================
// getMyTargetArgument
//============================
Argument * ArgumentInput::getTargetArgument ( )
{
  return myTargetArgument;
}

//=============================
// getName
//=============================
std::string ArgumentInput::getName ( ) const
{
	return myTargetArgument->getName();
}

//=============================
// getType
//=============================
std::string ArgumentInput::getType ( ) const
{
	std::string ret = myTargetArgument->getType();

	std::string afterArg = myTargetArgument->getTypeAfterArg();

	/*
	 * How many "[" do we have here?
	 */
	std::size_t pos = 0;
	int numArrays = 0;
	do
	{
		pos = afterArg.find("[", pos);
		if (pos != std::string::npos)
		{
			pos++;
			numArrays++;
		}
	}
	while (pos != std::string::npos);

	/*
	 * Add an "*" for each array bracket in the type after arg
	 */
	for (int i = 0; i < numArrays; i++)
	{
		ret += "*";
	}

	/*
	 * Remove any "const " qualifiers
	 */
	std::string constQ = "const ";
	pos = ret.find(constQ);
	if (pos != std::string::npos)
	{
		ret.replace (pos, constQ.length(), "");
	}

	return ret;
}

//=============================
// isArrayInput
//=============================
bool ArgumentInput::isArrayInput () const
{
	if (myTargetArgument->isArray())
		return true;
	return false;
}

//=============================
// needsOperation
//=============================
bool ArgumentInput::needsOperation (Operation** pOutOp, int *pOutId)
{
    if (!myTargetArgument)
        return false;

    if (!myTargetArgument->isArrayWithLengthOp())
        return false;

    ArrayArgumentOp* lenOp = (ArrayArgumentOp*) myTargetArgument;

    if (pOutOp)
        *pOutOp = lenOp->getLengthOp();

    if (pOutId)
        *pOutId = lenOp->getMappingId();

    return true;
}

//=============================
// getLenName
//=============================
std::string ArgumentInput::getLenName () const
{
	if (isArrayInput())
		return myTargetArgument->getLengthVariable ();

	return "";
}

//=============================
// print
//=============================
std::ostream& ArgumentInput::print (std::ostream& out) const
{
	out << "argumentInput{";
	Input::print(out);
	out << "}";

	return out;
}

//=============================
// getDotInputNodeName
//=============================
std::string ArgumentInput::getDotInputNodeName (std::string callName)
{
	return callName + ":" + myTargetArgument->getName();
}

/*EOF*/
