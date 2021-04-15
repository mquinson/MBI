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
 * @file Operation.cpp
 * 		@see gti::weaver::Operation
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */
#include <stdio.h>

#include "Operation.h"

using namespace gti::weaver::analyses;

//=============================
// Constructor
//=============================
Operation::Operation ( )
 : Calculation (),
   myReturnType (""),
   myExtraHeaders (),
   mySourceTemplate (""),
   myCleanupTemplate (""),
   myHasArrayReturn (false),
   myArrayLenType ("")
{
	/*Nothing to do*/
}

//=============================
// Constructor
//=============================
Operation::Operation (
		std::string name,
		std::vector<InputDescription*> argumentSpec,
		AnalysisGroup * group,
		std::string returnType,
		std::list<std::string> extraHeaders,
		std::string sourceTemplate,
		std::string cleanupTemplate)
 : Calculation (name, argumentSpec, group),
   myReturnType (returnType),
   myExtraHeaders (extraHeaders),
   mySourceTemplate (sourceTemplate),
   myCleanupTemplate (cleanupTemplate),
   myHasArrayReturn (false),
   myArrayLenType ("")
{
	/*Nothing to do*/
}

//=============================
// Constructor
//=============================
Operation::Operation (
		std::string name,
		std::vector<InputDescription*> argumentSpec,
		AnalysisGroup * group,
		std::string returnType,
		std::list<std::string> extraHeaders,
		std::string sourceTemplate,
		std::string cleanupTemplate,
		std::string arrayLenType)
 : Calculation (name, argumentSpec, group),
   myReturnType (returnType),
   myExtraHeaders (extraHeaders),
   mySourceTemplate (sourceTemplate),
   myCleanupTemplate (cleanupTemplate),
   myHasArrayReturn (true),
   myArrayLenType (arrayLenType)
{
	/*Nothing to do*/
}

//=============================
// Destructor
//=============================
Operation::~Operation ( )
{
	/*Nothing to do*/
}

//=============================
// setReturnType
//=============================
void Operation::setReturnType ( std::string new_var )
{
	myReturnType = new_var;
}

//=============================
// getReturnType
//=============================
std::string Operation::getReturnType ( )
{
	return myReturnType;
}

//=============================
// setExtraHeaders
//=============================
void Operation::setExtraHeaders ( std::list<std::string> new_var )
{
	myExtraHeaders = new_var;
}

//=============================
// getExtraHeaders
//=============================
std::list<std::string> Operation::getExtraHeaders ( )
{
	return myExtraHeaders;
}

//=============================
// setSourceTemplate
//=============================
void Operation::setSourceTemplate ( std::string new_var )
{
	mySourceTemplate = new_var;
}

//=============================
// getSourceTemplate
//=============================
std::string Operation::getSourceTemplate ( )
{
	return mySourceTemplate;
}

//=============================
// getCleanupTemplate
//=============================
std::string Operation::getCleanupTemplate (void)
{
	return myCleanupTemplate;
}

//=============================
// print
//=============================
bool Operation::isOperation (void)
{
	return true;
}

//=============================
// print
//=============================
std::ostream& Operation::print (std::ostream& out) const
{
	Calculation::print (out);

	out
		<< ", returnType=" << myReturnType
		<< ", headers={";

	std::list<std::string>::const_iterator i;
	for (i = myExtraHeaders.begin(); i != myExtraHeaders.end(); i++)
	{
		if (i != myExtraHeaders.begin())
			out << ", ";
		out << *i;
	}

	out
		<< "}, sourceTemplate={\""
		<< mySourceTemplate
		<< "\"}, cleanupTemplate={\""
		<< myCleanupTemplate
		<< "\"}";

	if (myHasArrayReturn)
	{
		out << ", returnsArray=yes, arrayLengthType=" << myArrayLenType;
	}

	return out;
}

//=============================
// getResultVarName
//=============================
std::string Operation::getResultVarName (int mappingId)
{
	char temp[128];
	sprintf (temp, "%d", mappingId);
	return myGroup->getGroupName() + "_" + myName + "_" + temp;
}

//=============================
// getResultLenVarName
//=============================
std::string Operation::getResultLenVarName (int mappingId)
{
	return getResultVarName (mappingId) + "_LEN";
}

//=============================
// getResultVarName
//=============================
bool Operation::hasArrayReturn (void)
{
	return myHasArrayReturn;
}

//=============================
// getLenReturnType
//=============================
std::string Operation::getLenReturnType (void)
{
	return myArrayLenType;
}

//=============================
// getDotNodeName
//=============================
std::string Operation::getDotNodeName (int mappingId)
{
	char temp[128];
	sprintf (temp, "%d", mappingId);

	return std::string("id") + temp + getName();
}

//=============================
// getDotNodeColor
//=============================
std::string Operation::getDotNodeColor (void)
{
	std::string ret = "olivedrab2";

	if (myHasArrayReturn)
		ret = "darkseagreen";

	return ret;
}

//=============================
// replaceSourceForMapping
//=============================
std::string Operation::replaceSourceForMapping (Mapping *m)
{
	return replaceTemplate (mySourceTemplate, m);
}

//=============================
// replaceCleanupForMapping
//=============================
std::string Operation::replaceCleanupForMapping (Mapping *m)
{
	return replaceTemplate (myCleanupTemplate, m);
}

//=============================
// replaceTemplate
//=============================
std::string Operation::replaceTemplate (std::string codeTemplate, Mapping *m)
{
	/*
	 * - Replace RETURN with result name
	 * - Replace RETURN_LEN with result length for array ops
	 * - Repalce ARG0 - ARGN with inputs
	 */

	std::vector<Input*> inputs = m->getArgumentInputs ();

	//==build array of things to search
	std::string *finds = new std::string[2+inputs.size()];
	finds[0] = "RETURN";
	finds[1] = "RETURN_LEN";

	for (int i = 0; i < inputs.size(); i++)
	{
		char temp[64];
		sprintf (temp, "ARG%d", i);
		finds[i+2] = temp;
	}

	//==build array of things to replace with
	std::string *replacements = new std::string[2+inputs.size()];
	replacements[0] = getResultVarName (m->getId());
	if (myHasArrayReturn)
		replacements[1] = getResultLenVarName (m->getId());
	else
		replacements[1] = "";

	for (int i = 0; i < inputs.size(); i++)
		replacements[i+2] = inputs[i]->getName();

	//==do the replacements
	for (int i = 0; i < inputs.size() + 2; i++)
	{
		size_t pos = codeTemplate.find(finds[i]);
		while (pos != std::string::npos)
		{
			size_t length;

			//If there is a "?" char in front of the found replacement string, do no replacement!
			if (pos != 0 && codeTemplate[pos-1] == '?')
			{
				codeTemplate.erase(pos-1,1);
				length = finds[i].length();
			}
			else
			{
				codeTemplate.replace (pos, finds[i].length(), replacements[i]);
				length = replacements[i].length();
			}

			//next
			pos = codeTemplate.find(finds[i], pos+length);
		}
	}

	return codeTemplate;
}

/*EOF*/
