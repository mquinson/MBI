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
 * @file OperationInput.cpp
 * 		@see gti::weaver::OperationInput
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <stdio.h>

#include "OperationInput.h"

using namespace gti::weaver::calls;

//=============================
// Constructor
//=============================
OperationInput::OperationInput ( )
 : Input (),
   myTargetOperation (NULL),
   myId (-1),
   myWantLen (false)
{
	/*Nothing to do*/
}

//=============================
// Constructor
//=============================
OperationInput::OperationInput (Operation *targetOperation, int id)
 : Input (),
   myTargetOperation (targetOperation),
   myId(id),
   myWantLen (false)
{
	/*Nothing to do*/
}

//=============================
// Constructor
//=============================
OperationInput::OperationInput (Operation *targetOperation, int id, bool wantLen)
 : Input (),
   myTargetOperation (targetOperation),
   myId(id),
   myWantLen (wantLen)
{
	/*Nothing to do*/
}

//=============================
// Destructor
//=============================
OperationInput::~OperationInput ( )
{
	myTargetOperation = NULL;
}

//=============================
// setTargetOperation
//=============================
void OperationInput::setTargetOperation ( Operation * new_var )
{
	myTargetOperation = new_var;
}

//=============================
// getTargetOperation
//=============================
Operation * OperationInput::getTargetOperation ( )
{
  return myTargetOperation;
}

//=============================
// getName
//=============================
std::string OperationInput::getName ( ) const
{
	if (myWantLen)
		return myTargetOperation->getResultLenVarName (myId);

	return myTargetOperation->getResultVarName (myId);
}

//=============================
// getType
//=============================
std::string OperationInput::getType ( ) const
{
	if (myWantLen)
		return myTargetOperation->getLenReturnType ();

	return myTargetOperation->getReturnType();
}

//=============================
// print
//=============================
std::ostream& OperationInput::print (std::ostream& out) const
{
	out << "operationInput={operation=" << myTargetOperation->getName();
	Input::print(out);
	out << ", mappingId=" << myId;

	if (myWantLen)
		out << ", usesOpArrayLenAsInput";

	out << "}";

	return out;
}

//=============================
// getDotInputNodeName
//=============================
std::string OperationInput::getDotInputNodeName (std::string callName)
{
	char temp[128];
	sprintf (temp, "%d", myId);
	return (std::string) "id" + temp + myTargetOperation->getName();
}

//=============================
// isLenInput
//=============================
bool OperationInput::isLenInput (void)
{
	return myWantLen;
}

//=============================
// isOpArrayLen
//=============================
bool OperationInput::isOpArrayLen (void) const
{
	return myWantLen;
}

//=============================
// isArrayInput
//=============================
bool OperationInput::isArrayInput () const
{
	if (!myWantLen && myTargetOperation->hasArrayReturn())
		return true;
	return false;
}

//=============================
// getLenName
//=============================
std::string OperationInput::getLenName () const
{
	if (isArrayInput())
		return myTargetOperation->getResultLenVarName (myId);

	return "";
}

//=============================
// needsOperation
//=============================
bool OperationInput::needsOperation (Operation** pOutOp, int *pOutId)
{
	if (pOutOp)
		*pOutOp = myTargetOperation;

	if (pOutId)
		*pOutId = myId;

	return true;
}

/*EOF*/
