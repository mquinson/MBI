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
 * @file Call.cpp
 * 		@see gti::weaver::Call
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Call.h"

using namespace gti::weaver::calls;


//=============================
// Static attributes
//=============================
int Call::ourNextUniqueId = 0;

//=============================
// Call
//=============================
Call::Call ( )
 : myName (""),
   myGroup (NULL),
   myArguments (),
   myWrappEverywhere (false),
   myReturnType ("void"),
   myIsFinalizer (false),
   myIsLocalFinalizer (false),
   myWrappAcross (false),
   myWrappDown (false),
   myIsShutdownNotify (false),
   myIsOutOfOrder (true),
   myIsCallback(false),
   myIsHook(false)
{
	myUniqueId = ourNextUniqueId;
	ourNextUniqueId++;
}

//=============================
// Call
//=============================
Call::Call (
		std::string name,
		ApiGroup* group,
		std::string returnType,
		bool wrappedEverywhere,
		bool isFinalizer,
		bool isLocalFinalizer,
		bool wrappAcross,
		bool wrappDown,
		bool isShutdownNotify,
		bool isOutOfOrder,
                bool isCallback,
		bool isHook)
 : myName (name),
   myGroup (group),
   myArguments(),
   myWrappEverywhere(wrappedEverywhere),
   myReturnType(returnType),
   myIsFinalizer(isFinalizer),
   myIsLocalFinalizer (isLocalFinalizer),
   myWrappAcross(wrappAcross),
   myWrappDown (wrappDown),
   myIsShutdownNotify (isShutdownNotify),
   myIsOutOfOrder (isOutOfOrder),
   myIsCallback(isCallback),
   myIsHook(isHook)
{
	myUniqueId = ourNextUniqueId;
	ourNextUniqueId++;
}

//=============================
// ~Call
//=============================
Call::~Call ( )
{
	for (int i = 0; i < myArguments.size(); i++)
	{
		if (myArguments[i])
			delete (myArguments[i]);
	}
	myArguments.clear();

	myGroup = NULL;
	myName = "";
}

//=============================
// setGroup
//=============================
void Call::setGroup ( ApiGroup * new_var )
{
  myGroup = new_var;
}

//=============================
// getGroup
//=============================
ApiGroup * Call::getGroup ( )
{
  return myGroup;
}

//=============================
// addArguments
//=============================
void Call::addArgument ( Argument * add_object )
{
  myArguments.push_back(add_object);
}

//=============================
// removeArgument
//=============================
void Call::removeArgument ( Argument * remove_object )
{
  int i, size = myArguments.size();
  for ( i = 0; i < size; ++i)
  {
  	Argument * item = myArguments.at(i);
  	if(item == remove_object)
  	{
  		std::vector<Argument *>::iterator it = myArguments.begin() + i;
  		if (*it)
  			delete (*it);
  		myArguments.erase(it);
  		return;
  	}
   }
}

//=============================
// getArguments
//=============================
std::vector<Argument *> Call::getArguments ( )
{
  return myArguments;
}

//=============================
// setName
//=============================
void Call::setName ( std::string new_var )
{
	myName = new_var;
}

//=============================
// getName
//=============================
std::string Call::getName ( )
{
	return myName;
}

//=============================
// print
//=============================
std::ostream& Call::print (std::ostream& out) const
{
	out
		<< "call={"
		<< "name=" << myName << ", "
		<< "group=" << myGroup->getApiName() << ", "
		<< "returnType=" << myReturnType << ", "
		<< "wrappEverywhere=" << myWrappEverywhere << ", "
		<< "wrappAcross=" << myWrappAcross << ", "
		<< "isFinalizer=" << myIsFinalizer << ", "
                << "isCallback=" << myIsCallback << ", "
		<< "arguments={";

	for (int i = 0; i < myArguments.size(); i++)
	{
		if (i != 0)
			out << ", ";
		out << *(myArguments[i]);
	}

	out << "}}";


	return out;
}

//=============================
// findArgument
//=============================
Argument* Call::findArgument (std::string name)
{
	for (int i = 0; i < myArguments.size(); i++)
	{
		if (myArguments[i]->getName() == name)
			return myArguments[i];
	}

	return NULL;
}

//=============================
// toDotNode
//=============================
std::string Call::toDotNode (void)
{
	std::string ret = "";
	std::string color="lightsalmon2";

	if (myWrappEverywhere)
	{
		color = "lightpink3";
	}

	if (myWrappAcross)
	{
	    color = "goldenrod2";
	}

	ret += getName() + "[label=\"{<" + getName() + ">" + getName() + "|{";

	for (int i = 0; i < myArguments.size(); i++)
	{
		if (i != 0)
			ret += "| ";

		ret += "<" + myArguments[i]->getName() + ">" + myArguments[i]->getName() + ":" + myArguments[i]->getType() + myArguments[i]->getTypeAfterArg();
	}

	ret += "}}\", shape=Mrecord, fillcolor=" + color +", style=filled];";



	return ret;
}

//=============================
// isWrappedEverywhere
//=============================
bool Call::isWrappedEverywhere (void)
{
	return myWrappEverywhere;
}

//=============================
// isWrappedAcross
//=============================
bool Call::isWrappedAcross (void)
{
    return myWrappAcross;
}

//=============================
// isWrappedDown
//=============================
bool Call::isWrappedDown (void)
{
    return myWrappDown;
}

//=============================
// isFinalizer
//=============================
bool Call::isFinalizer (void)
{
	return myIsFinalizer;
}

//=============================
// isLocalFinalizer
//=============================
bool Call::isLocalFinalizer (void)
{
    return myIsLocalFinalizer;
}

//=============================
// getReturnType
//=============================
std::string Call::getReturnType (void)
{
	return myReturnType;
}

//=============================
// getUniqueId
//=============================
int Call::getUniqueId (void)
{
	return myUniqueId;
}

//=============================
// getUniqueId
//=============================
bool Call::isNotifyFinalize (void)
{
    return myIsShutdownNotify;
}

//=============================
// isOutOfOrder
//=============================
bool Call::isOutOfOrder (void)
{
    return myIsOutOfOrder;
}

//=============================
// isCallback
//=============================
bool Call::isCallback (void)
{
    return myIsCallback;
}

//=============================
// isHook
//=============================
bool Call::isHook (void)
{
    return myIsHook;
}
