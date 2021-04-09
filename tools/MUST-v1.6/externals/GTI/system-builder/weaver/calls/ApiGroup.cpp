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
 * @file ApiGroup.cpp
 * 		@see gti::weaver::ApiGroup
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>

#include "ApiGroup.h"

using namespace gti::weaver::calls;

//=============================
// ApiGroup
//=============================
ApiGroup::ApiGroup ( )
 : myHeaders (), myApiName (""), myCalls ()
{
	/*Nothing to do*/
}

//=============================
// ApiGroup
//=============================
ApiGroup::ApiGroup (std::string name)
 : myHeaders (), myApiName (name), myCalls ()
{
	/*Nothing to do*/
}

//=============================
// ~ApiGroup
//=============================
ApiGroup::~ApiGroup ( )
{
	std::list<Call*>::iterator i;
	for (i = myCalls.begin(); i != myCalls.end(); i++)
	{
		if (*i)
			delete (*i);
	}
	myCalls.clear ();

	myHeaders.clear ();

	myApiName = "";
}

//=============================
// addHeader
//=============================
void ApiGroup::addHeader ( std::string new_var )
{
	myHeaders.push_back(new_var);
}

//=============================
// getHeaders
//=============================
std::list<std::string> ApiGroup::getHeaders (void)
{
	return myHeaders;
}

//=============================
// setApiName
//=============================
void ApiGroup::setApiName ( std::string new_var )
{
	myApiName = new_var;
}

//=============================
// getApiName
//=============================
std::string ApiGroup::getApiName ( )
{
	return myApiName;
}

//=============================
// registerCall
//=============================
void ApiGroup::registerCall (Call* new_call)
{
	assert (new_call);

	std::list <Call*>::iterator i;
	for (i = myCalls.begin(); i != myCalls.end(); i++)
	{
		if ((*i)->getName() == new_call->getName())
			return; //already exists ..
	}

	myCalls.push_back (new_call);
}

//=============================
// getCalls
//=============================
std::list<Call*> ApiGroup::getCalls (void)
{
	return myCalls;
}

//=============================
// removeCall
//=============================
void ApiGroup::removeCall (Call* call)
{
	assert (call);

	std::list <Call*>::iterator i;
	for (i = myCalls.begin(); i != myCalls.end(); i++)
	{
		if ((*i)->getName() == call->getName())
		{
			myCalls.erase(i);
			return;
		}
	}
}

//=============================
// print
//=============================
std::ostream& ApiGroup::print (std::ostream& out) const
{
	out
		<< "ApiGroup={"
		<< "name=" << myApiName << ", "
		<< "headers={";

	std::list<std::string>::const_iterator i;
	for (i = myHeaders.begin();i != myHeaders.end(); i++)
	{
		if (i != myHeaders.begin())
			out << ", ";
		out << "\"" << *i << "\"";
	}

	out << "}, calls={";

	std::list <Call*>::const_iterator j;
	for (j = myCalls.begin(); j != myCalls.end(); j++)
	{
		if (j != myCalls.begin())
			out << ", ";
		out << **j;
	}

	out << "}}";

	return out;
}

/*EOF*/
