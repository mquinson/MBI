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
 * @file Mapping.cpp
 * 		@see gti::weaver::Mapping
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>

#include "Mapping.h"

using namespace gti::weaver::calls;

//=============================
// Mapping
//=============================
Mapping::Mapping ( )
 : myApiCall (NULL),
   myArgumentInputs (),
   myOrder(ORDER_PRE),
   myId (-1),
   myIntraCallOrder (-1)
{
	/*Nothing to do*/
}

//=============================
// Mapping
//=============================
Mapping::Mapping (Call* apiCall, CalculationOrder order, int id, int intraCallOrder)
 : myApiCall (apiCall),
   myArgumentInputs (),
   myOrder(order),
   myId (id),
   myIntraCallOrder (intraCallOrder)
{
	assert (myApiCall);
}

//=============================
// ~Mapping
//=============================
Mapping::~Mapping ( )
{
	for (int i = 0; i < myArgumentInputs.size(); i++)
	{
		if (myArgumentInputs[i])
			delete (myArgumentInputs[i]);
	}
	myArgumentInputs.clear ();

	myApiCall = NULL;
}

//=============================
// setApiCall
//=============================
void Mapping::setApiCall ( Call * new_var )
{
  myApiCall = new_var;
}

//=============================
// getApiCall
//=============================
Call * Mapping::getApiCall ( )
{
  return myApiCall;
}

//=============================
// addArgumentInput
//=============================
void Mapping::addArgumentInput ( Input * add_object )
{
  myArgumentInputs.push_back(add_object);
}

//=============================
// getMyArgumentInputs
//=============================
std::vector<Input *> Mapping::getArgumentInputs ( )
{
  return myArgumentInputs;
}

//=============================
// print
//=============================
std::ostream& Mapping::print (std::ostream& out) const
{
	std::string orderString = "post";
	if (myOrder == ORDER_PRE)
		orderString = "pre";

	out << "mapping={"
	    << "order=" << orderString;

	if (myId != -1)
		out << ", id=" << myId;

	out
	    << ", call=" << myApiCall->getName()
	    << ", inputs={";

	for (int i = 0; i < myArgumentInputs.size(); i++)
	{
		if (i != 0)
			out << ", ";

		out << myArgumentInputs[i];
	}

    out << "}}";

	return out;
}

//=============================
// getOrder
//=============================
CalculationOrder Mapping::getOrder (void)
{
	return myOrder;
}

//=============================
// setOrder
//=============================
void Mapping::setOrder (CalculationOrder order)
{
	myOrder = order;
}

//=============================
// getId
//=============================
int Mapping::getId (void)
{
	return myId;
}

//=============================
// getIntraCallOrder
//=============================
int Mapping::getIntraCallOrder (void)
{
	return myIntraCallOrder;
}

/*EOF*/
