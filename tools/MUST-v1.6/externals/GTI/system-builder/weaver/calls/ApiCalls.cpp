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
 * @file ApiCalls.cpp
 * 		@see gti::weaver::ApiCalls
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <list>
#include <map>
#include <fstream>
#include <iostream>

#include "ApiCalls.h"
#include "ArrayArgument.h"
#include "ArrayArgumentOp.h"
#include "Verbose.h"
#include "Analyses.h"
#include "OperationInput.h"
#include "ArgumentInput.h"
#include "CallNameInput.h"
#include "CallReturnInput.h"
#include "CallIdInput.h"

using namespace gti;
using namespace gti::weaver::calls;

ApiCalls* ApiCalls::ourInstance = NULL;

//=============================
// getInstance
//=============================
ApiCalls* ApiCalls::getInstance (void)
{
	if (!ourInstance)
	{
		ourInstance = new ApiCalls ();
		assert (ourInstance);
	}

	return ourInstance;
}

//=============================
// ApiCalls
//=============================
ApiCalls::ApiCalls ( )
	: myApis ()
{
	/*Nothing to do.*/
}

//=============================
// ~ApiCalls
//=============================
ApiCalls::~ApiCalls ( )
{
	std::vector<ApiGroup *>::iterator it;
	for (size_t i = 0; i < myApis.size(); i++)
	{
		if (myApis[i])
			delete(myApis[i]);
	}

	myApis.clear ();
}

//=============================
// addApi
//=============================
void ApiCalls::addApi ( ApiGroup * add_object )
{
  myApis.push_back(add_object);
}

//=============================
// removeApi
//=============================
void ApiCalls::removeApi ( ApiGroup * remove_object )
{
  int i, size = myApis.size();
  for ( i = 0; i < size; ++i)
  {
  	ApiGroup * item = myApis.at(i);
  	if(item == remove_object)
  	{
  		std::vector<ApiGroup *>::iterator it = myApis.begin() + i;
  		if (*it)
  			delete (*it);
  		*it = NULL;
  		myApis.erase(it);
  		return;
  	}
   }
}

//=============================
// load
//=============================
GTI_RETURN ApiCalls::load (std::list<std::string> apiSpecificationXmls )
{
	VERBOSE(1) << "Loading APIs ..." << std::endl;

	//Loop over all input XMLs
	//=================================
	xmlDocPtr document;
	SpecificationNode currentPointer;
	std::list<std::string>::iterator i;

	for (i = apiSpecificationXmls.begin(); i != apiSpecificationXmls.end(); i++)
	{
		VERBOSE(1) << "|-Loading input file: " << *i << " ..." << std::endl;

		document = xmlParseFile(i->c_str());

		if (document == NULL )
		{
			std::cerr << "| |-->Error loading input XML (" << *i << ")" << "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
			return GTI_ERROR;
		}

		currentPointer = xmlDocGetRootElement(document);

		if (currentPointer == NULL ||
				(xmlStrcmp(currentPointer()->name, (const xmlChar *) "api-specification") != 0))
		{
			std::cerr
			<< "| |-->"
			<< "Error: Document does not contains the root node (\"api-specification\")"
			<< "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
			if (currentPointer)
				std::cerr << "Found \"" << currentPointer()->name << "\" instead!" << std::endl;
			xmlFreeDoc(document);
			return GTI_ERROR;
		}

		if (readApiSpecification (currentPointer) != GTI_SUCCESS)
			return GTI_ERROR;

		VERBOSE(1) << "| |--> SUCCESS" << std::endl;
	}

	VERBOSE(1) << "--> SUCCESS" << std::endl;

	return GTI_SUCCESS;
}

//=============================
// readApiSpecification
//=============================
GTI_RETURN ApiCalls::readApiSpecification (SpecificationNode root)
{
	std::string name;
	SpecificationNode child;

	//==Attribute: unique-name
	if (!root.getAttributeOrErr(
			"unique-name",
			"| |-->Error: the api-specification root node has no \"unique-name\" attribute.",
			&name))
		return GTI_ERROR;

	//==Create API group
	ApiGroup *group = new ApiGroup (name);
	assert (group);

	//==Child: api-headers (and sub-childs)
	child = root.findChildNodeNamedOrErr(
			"api-headers",
			"| |-->Error: the api-specification root node has no \"api-headers\" child.");
	if (!child) return GTI_ERROR;

	child = child.findChildNodeNamed("header");
	while (child)
	{
		//add header
		group->addHeader(child.getNodeContent());

		//next
		child = child.findSiblingNamed("header");
	}

	//==Child: functions (and "function" sub-childs)
	child = root.findChildNodeNamedOrErr(
			"functions",
			"| |-->Error: the api-specification root node has no \"functions\" child.");
	if (!child) return GTI_ERROR;

	child = child.findChildNodeNamed("function");
	while (child)
	{
		//read function
		if (readFunction (child, group) != GTI_SUCCESS)
			return GTI_ERROR;

		//next
		child = child.findSiblingNamed("function");
	}

	//==VERBOSE
	VERBOSE (2) << "| |-> Loaded group: " << *group << std::endl;

	//==Add group to list of groups
	myApis.push_back (group);

	return GTI_SUCCESS;
}

//=============================
// readFunction
//=============================
GTI_RETURN ApiCalls::readFunction (SpecificationNode node, ApiGroup *group)
{
	std::string name, wrappEverywhereString, wrappAcrossString, wrappDownString, isFinalizerString, returnType, isNotifyFinalizeString, isOutOfOrderString, isCallbackString, isHookString;
	bool wrappEverywhere = false;
	bool isFinalizer = false;
	bool isLocalFinalizer = false;
	bool wrappAcross = false;
	bool wrappDown = false;
	bool isNotifyFinalize = false;
	bool isOutOfOrder = false;
        bool isCallback = false;
	bool isHook = false;
	std::map<int, Argument*> argMap; //maps argument order to argument

	SpecificationNode child;

	//==Attribute: name
	if (!node.getAttributeOrErr(
			"name",
			"| |-->Error: a function node has no \"name\" attribute.",
			&name))
		return GTI_ERROR;

	//==Attribute: return-type
	if (!node.getAttributeOrErr(
			"return-type",
			"| |-->Error: a function node has no \"return-type\" attribute.",
			&returnType))
		return GTI_ERROR;

	//==Attribute: wrapp-everywhere (Optional)
	if (node.getAttribute(
			"wrapp-everywhere",
			&wrappEverywhereString))
	{
		if (wrappEverywhereString == "yes")
			wrappEverywhere=true;

		if (wrappEverywhereString != "yes" &&
			wrappEverywhereString != "no")
		{
			std::cerr
			<< "| |-->Error: a function node uses the \"wrapp-everywhere\" attribute with the unknown value \""
			<< wrappEverywhereString
			<< "\"; valid values are \"yes\" and \"no\"."
			<< std::endl;
			return GTI_ERROR;
		}
	}

	//==Attribute: wrapp-across (Optional)
	if (node.getAttribute(
	        "wrapp-across",
	        &wrappAcrossString))
	{
	    if (wrappAcrossString == "yes")
	        wrappAcross=true;

	    if (wrappAcrossString != "yes" &&
	            wrappAcrossString != "no")
	    {
	        std::cerr
	        << "| |-->Error: a function node uses the \"wrapp-across\" attribute with the unknown value \""
	        << wrappAcrossString
	        << "\"; valid values are \"yes\" and \"no\"."
	        << std::endl;
	        return GTI_ERROR;
	    }
	}

	//==Attribute: wrapp-down  (Optional)
	if (node.getAttribute(
	        "wrapp-down",
	        &wrappDownString))
	{
	    if (wrappDownString == "yes")
	        wrappDown=true;

	    if (wrappDownString != "yes" &&
	            wrappDownString != "no")
	    {
	        std::cerr
	        << "| |-->Error: a function node uses the \"wrapp-down\" attribute with the unknown value \""
	        << wrappAcrossString
	        << "\"; valid values are \"yes\" and \"no\"."
	        << std::endl;
	        return GTI_ERROR;
	    }
	}

	//==Consistency: not more than one out of: wrapp-everywhere, wrapp-across, and wrapp-down!
	if ((int)wrappEverywhere + (int)wrappAcross + (int)wrappDown > 1)
	{
	    std::cerr
            << "| |-->Error: a function node uses more than one of the exclusive options \"wrapp-across\", \"wrapp-everywhere\" and \"wrapp-down\". A function may only be one of the three. Function name:"
            << name
            << ", wrapp-across=" << wrappAcrossString << ", wrapp-everywhere=" << wrappEverywhereString << ", wrapp-down=" << wrappDownString << "."
            << std::endl;
	    return GTI_ERROR;
	}

	//==Attribute: is-finalizer (Optional)
	if (node.getAttribute(
			"is-finalizer",
			&isFinalizerString))
	{
		if (isFinalizerString == "yes")
			isFinalizer=true;

		if (isFinalizerString != "yes" &&
		    isFinalizerString != "no")
		{
			std::cerr
			<< "| |-->Error: a function node uses the \"is-finalizer\" attribute with the unknown value \""
			<< isFinalizerString
			<< "\"; valid values are \"yes\" and \"no\"."
			<< std::endl;
			return GTI_ERROR;
		}
	}

	//==Attribute: is-local-finalizer (Optional)
	if (node.getAttribute(
	        "is-local-finalizer",
	        &isFinalizerString))
	{
	    if (isFinalizerString == "yes")
	        isLocalFinalizer=true;

	    if (isFinalizerString != "yes" &&
	            isFinalizerString != "no")
	    {
	        std::cerr
	        << "| |-->Error: a function node uses the \"is-local-finalizer\" attribute with the unknown value \""
	        << isFinalizerString
	        << "\"; valid values are \"yes\" and \"no\"."
	        << std::endl;
	        return GTI_ERROR;
	    }
	}

    //==Attribute: notify-finalize (Optional)
    if (node.getAttribute(
            "notify-finalize",
            &isNotifyFinalizeString))
    {
        if (isNotifyFinalizeString == "yes")
            isNotifyFinalize=true;

        if (isNotifyFinalizeString != "yes" &&
                isNotifyFinalizeString != "no")
        {
            std::cerr
            << "| |-->Error: a function node uses the \"notify-finalize\" attribute with the unknown value \""
            << isNotifyFinalizeString
            << "\"; valid values are \"yes\" and \"no\"."
            << std::endl;
            return GTI_ERROR;
        }
    }

    //==Attribute: out-of-order (Optional)
    if (node.getAttribute(
            "out-of-order",
            &isOutOfOrderString))
    {
        if (isOutOfOrderString == "yes")
            isOutOfOrder=true;

        if (isOutOfOrderString != "yes" &&
                isOutOfOrderString != "no")
        {
            std::cerr
            << "| |-->Error: a function node uses the \"out-of-order\" attribute with the unknown value \""
            << isOutOfOrderString
            << "\"; valid values are \"yes\" and \"no\"."
            << std::endl;
            return GTI_ERROR;
        }
    }
        
    //==Attribute: callback (Optional)
    if (node.getAttribute(
            "callback",
            &isCallbackString))
    {
        if (isCallbackString == "yes")
            isCallback=true;

        if (isCallbackString != "yes" &&
                isCallbackString != "no")
        {
            std::cerr
            << "| |-->Error: a function node uses the \"callback\" attribute with the unknown value \""
            << isCallbackString
            << "\"; valid values are \"yes\" and \"no\"."
            << std::endl;
            return GTI_ERROR;
        }
    }

    //==Attribute: hook (Optional)
    if (node.getAttribute(
            "hook",
            &isHookString))
    {
        if (isHookString == "yes")
            isHook=true;

        if (isHookString != "yes" &&
                isHookString != "no")
        {
            std::cerr
            << "| |-->Error: a function node uses the \"hook\" attribute with the unknown value \""
            << isHookString
            << "\"; valid values are \"yes\" and \"no\"."
            << std::endl;
            return GTI_ERROR;
        }
    }

	//==Create call
	Call* call = new Call (
	        name,
	        group,
	        returnType,
	        wrappEverywhere,
	        isFinalizer,
	        isLocalFinalizer,
	        wrappAcross,
	        wrappDown,
	        isNotifyFinalize,
	        isOutOfOrder,
                isCallback,
	        isHook);
	group->registerCall(call);

	//==Child: function-arguments
	child = node.findChildNodeNamedOrErr(
			"function-arguments",
			"| |-->Error: a function node has no \"function-arguments\" child.");
	if (!child)	return GTI_ERROR;

	//1) read function-argument nodes
	SpecificationNode argumentNode = child.findChildNodeNamed("function-argument");
	while (argumentNode)
	{
		Argument* argument;
		int order;

		if (readArgument (argumentNode, &argument, &order) != GTI_SUCCESS)
			return GTI_ERROR;

		argMap.insert (std::make_pair(order, argument));

		//next
		argumentNode = argumentNode.findSiblingNamed("function-argument");
	}

	//2) read function-array-argument nodes
	//   (Done after reading the regular arguments to test
	//    whether specified length-arguments really exist)
	argumentNode = child.findChildNodeNamed("function-array-argument");
	while (argumentNode)
	{
		Argument* argument;
		int order;

		if (readArrayArgument (argumentNode, &argument, &order, argMap) != GTI_SUCCESS)
			return GTI_ERROR;

		argMap.insert (std::make_pair(order, argument));

		//next
		argumentNode = argumentNode.findSiblingNamed("function-array-argument");
	}

	//3) apply argument map to call
	int nextOrder = 0;
	std::map<int, Argument*>::iterator i;
	for (i = argMap.begin(); i != argMap.end(); i++)
	{
		//check integrity of orders!
		if (i->first != nextOrder)
		{
			std::cerr
				<< "| |-->Error: arguments of call \"" << name << "\" are incorrect, you specified a maximum order of "
				<< argMap.rbegin()->first
				<< " while not specifying an argument for order "
				<< nextOrder;
			return GTI_ERROR;
		}

		//add the argument
		call->addArgument(i->second);

		//next
		nextOrder++;
	}

	/*
	 * It is important to read operations first,
	 * otherwise we can't check whether analysis
	 * inputs of the operation type are valid or not.
	 */
	//==Child: operations (and operation sub-childs)
	child = node.findChildNodeNamedOrErr(
			"operations",
			"| |-->Error: a function node has no \"operations\" child node.");
	if (!child) return GTI_ERROR;

	child = child.findChildNodeNamed("operation");
	int operationIndex = 0;
	while (child)
	{
		//read operation
		if (readOperation (child, call, operationIndex) != GTI_SUCCESS)
			return GTI_ERROR;

		//next
		child = child.findSiblingNamed("operation");
		operationIndex++;
	}

	//==Child: analyses (and analysis sub-childs)
	child = node.findChildNodeNamedOrErr(
			"analyses",
			"| |-->Error: a function node has no \"analyses\" child node.");
	if (!child) return GTI_ERROR;

	child = child.findChildNodeNamed("analysis");
	int analysisIndex = 0;
	while (child)
	{
		//read analysis
		if (readAnalysis (child, call, analysisIndex) != GTI_SUCCESS)
			return GTI_ERROR;

		//next
		child = child.findSiblingNamed("analysis");
		analysisIndex++;
	}

	//4)Verify ArrayArgumentOps (has the op a mapping for the call?)
	std::vector<Argument*> callArgs = call->getArguments();
	for (size_t i = 0; i < callArgs.size(); i++)
	{
		if (!callArgs[i]->isArray())
			continue;

		if (!callArgs[i]->isArrayWithLengthOp())
			continue;

		ArrayArgumentOp* arrayArgOp = (ArrayArgumentOp*) callArgs[i];

		if (!arrayArgOp->getLengthOp()->hasMappingForCall (call->getName(), call->getGroup()->getApiName(), arrayArgOp->getMappingId()))
		{
			std::cerr << "| |-->Error: call \"" << call->getName() << "\" has argument named \"" << callArgs[i]->getName() << "\" which uses operation \"" << arrayArgOp->getLengthOp()->getName() << "\" as length argument, however, this operation was not mapped to this function." << std::endl;
			return GTI_ERROR;
		}
	}

	//==Child: created-by (and by sub-childs)
	child = node.findChildNodeNamed(
	        "created-by");
	if (!child) return GTI_SUCCESS; //If we have no such node we are done

	child = child.findChildNodeNamed("by");
	while (child)
	{
	    //read the "by" node
	    std::string modGroup, modName;

	    if (!child.getAttributeOrErr(
	            "group",
	            "| |-->Error: a \"by\" node has no \"group\" attribute.",
	            &modGroup))
	        return GTI_ERROR;

	    if (!child.getAttributeOrErr(
	            "name",
	            "| |-->Error: a \"by\" node has no \"name\" attribute.",
	            &modName))
	        return GTI_ERROR;

	    AnalysisModule* module = Analyses::getInstance()->findAnalysisModule(modName,modGroup);

	    if (!module)
	    {
	        //Do we know the group?
	        if (Analyses::getInstance()->hasGroup(modGroup))
	        {
                std::cerr << "| |-->Error: call \"" << call->getName() << "\" uses a created-by module of name " << modName << " from group " << modGroup << " that could not be found even though the named group was loaded." << std::endl;
                return GTI_ERROR;
	        }
	    }
	    else
	    {
	        module->addCallToCreate(call);
	    }

	    //next
	    child = child.findSiblingNamed("by");
	}

	return GTI_SUCCESS;
}

//=============================
// readArgument
//=============================
GTI_RETURN ApiCalls::readArgument (SpecificationNode node, Argument **ppOutArgument, int *pOutOrder)
{
	assert (ppOutArgument && pOutOrder);

	//attributes to read
	std::string name;
	std::string type;
	ArgumentIntent intent;
	std::string typeAfterArg;

	//read attributes
	if (!readArgumentAttributes (
			node,
			"function-argument",
			&name,
			&type,
			&intent,
			pOutOrder,
			&typeAfterArg))
		return GTI_ERROR;

	//create argument
	*ppOutArgument = new Argument (name, type, intent, typeAfterArg);

	return GTI_SUCCESS;
}

//=============================
// readArrayArgument
//=============================
GTI_RETURN ApiCalls::readArrayArgument (SpecificationNode node, Argument **ppOutArgument, int *pOutOrder, std::map<int, Argument*> argMap)
{
	assert (ppOutArgument && pOutOrder);

	//attributes to read
	std::string name;
	std::string type;
	ArgumentIntent intent;
	std::string typeAfterArg;

	//read attributes
	if (!readArgumentAttributes (
			node,
			"function-array-argument",
			&name,
			&type,
			&intent,
			pOutOrder,
			&typeAfterArg))
		return GTI_ERROR;

	//child: length-argument
	SpecificationNode child = node.findChildNodeNamedOrErr(
			"length-argument",
			"| |-->Error: a function-array-argument node has no \"length-argument\" child.");
	if (!child) return GTI_ERROR;

	std::string typeString;
	if (!child.getAttributeOrErr(
			"type",
			"| |-->Error: a length-argument node has no \"type\" child.",
			&typeString))
		return GTI_ERROR;

	if (typeString == "argument")
	{
		std::string lenArgName;
		SpecificationNode lenNode;
		Argument *lenArg = NULL;

		lenNode = child.findChildNodeNamedOrErr(
				"call-arg-name",
				"| |-->Error: a length-argument node of the argument type has no \"call-arg-name\" child.");
		if (!lenNode) return GTI_ERROR;

		lenArgName = lenNode.getNodeContent();

		std::map<int, Argument*>::iterator iter;
		for (iter = argMap.begin(); iter != argMap.end(); iter++)
		{
			if (iter->second->getName() == lenArgName)
			{
				lenArg = iter->second;
				break;
			}
		}

		if (lenArg == NULL)
		{
			std::cerr<< "| |-->Error: the length argument named \"" << lenArgName << "\" could not be found for the array argument \"" << name << "\"." << std::endl;
			return GTI_ERROR;
		}

		//create arg array argument
		*ppOutArgument = new ArrayArgument (name, type, intent, lenArg, typeAfterArg);
	}
	else if (typeString == "operation" ||
			 typeString == "operation-len")
	{
		std::string opName, opGroup, idString;
		int id;
		SpecificationNode opNode;
		Operation *lenOp = NULL;

		opNode = child.findChildNodeNamedOrErr(
				"op-name",
				"| |-->Error: a length-argument node of the operation type has no \"op-name\" child.");
		if (!opNode) return GTI_ERROR;

		opName = opNode.getNodeContent();

		if (!opNode.getAttributeOrErr(
				"group",
				"| |-->Error: a op-name node has no \"group\" attribute.",
				&opGroup))
			return GTI_ERROR;

		if (!opNode.getAttributeOrErr(
				"id",
				"| |-->Error: a op-name node has no \"id\" attribute.",
				&idString))
			return GTI_ERROR;
		if (sscanf (idString.c_str(), "%d", &id) != 1)
		{
			std::cerr << "| |-->Error: an op-name node specified an invalid id attribute; specified was: \""<<idString<<"\", which is not an integer."<<std::endl;
			return GTI_ERROR;
		}

		lenOp = Analyses::getInstance()->findOperation (opName, opGroup);

		if (lenOp == NULL)
		{
			std::cerr << "| |-->Error: the length determining operation named \"" << opName << "\" could not be found for the array argument \"" << name << "\"." << std::endl;
			return GTI_ERROR;
		}

		//create arg array argument
		if (typeString == "operation")
		{
			*ppOutArgument = new ArrayArgumentOp (name, type, intent, lenOp, id, typeAfterArg);
		}
		else
		{
			if (!lenOp->hasArrayReturn())
			{
				std::cerr << "| |-->Error: a length-argument node uses the operation-len type with operation \"" << lenOp->getName() << "\" which returns no array and is thus invalid for this type." << std::endl;
				return GTI_ERROR;
			}
			*ppOutArgument = new ArrayArgumentOp (name, type, intent, lenOp, id, true, typeAfterArg);
		}
	}
	else
	{
		std::cerr << "| |-->Error: the type attribute of a length-argument node is invalid, valid values are \"argument\", \"operation\", and \"operation-len\"; specification was \"" << typeString << "\"." << std::endl;
		*ppOutArgument = NULL;
		return GTI_ERROR;
	}

	return GTI_SUCCESS;
}

//=============================
// readArgumentAttributes
//=============================
bool ApiCalls::readArgumentAttributes (
		SpecificationNode node,
		std::string nodeName,
		std::string *pOutName,
		std::string *pOutType,
		ArgumentIntent *pOutIntent,
		int *pOutOrder,
		std::string *pOutTypeAfterArg)
{
	std::string intentString, orderString, typeAfterArg = "";

	//name
	if (!node.getAttributeOrErr(
			"name",
			"| |-->Error: a " + nodeName + " node has no \"name\" attribute.",
			pOutName))
		return false;

	//type
	if (!node.getAttributeOrErr(
			"type",
			"| |-->Error: a " + nodeName + " node has no \"type\" attribute.",
			pOutType))
		return false;

	//type
	node.getAttribute(
			"typeAfterArg",
			&typeAfterArg);
	if (pOutTypeAfterArg)
		*pOutTypeAfterArg = typeAfterArg;

	//intent
	if (!node.getAttributeOrErr(
			"intent",
			"| |-->Error: a " + nodeName + " node has no \"intent\" attribute.",
			&intentString))
		return false;

	if (intentString == "in")
	{
		*pOutIntent = INTENT_IN;
	}
	else if (intentString == "out")
	{
		*pOutIntent = INTENT_OUT;
	}
	else if (intentString == "inout")
	{
		*pOutIntent = INTENT_INOUT;
	}
	else
	{
		std::cerr << "| |-->Error: a " + nodeName + " node has an invalid \"intent\" attribute, valid values are \"in\", \"out\", and \"inout\"; you specified: \"" << intentString << "\"." << std::endl;
		return false;
	}

	//order
	if (!node.getAttributeOrErr(
			"order",
			"| |-->Error: a " + nodeName + " node has no \"order\" attribute.",
			&orderString))
		return false;
	*pOutOrder = atoi(orderString.c_str());

	return true;
}

void ApiCalls::listAvailableAnalysisFunctions(std::string group, std::ostream &out)
{
	std::list<Analysis*> knownAnalyses;

	out << "| |-->Info: Available analysis functions for group " << group << ": ";

	knownAnalyses = Analyses::getInstance()->getAnalyses(group);
	for (std::list<Analysis*>::iterator it = knownAnalyses.begin(); it!= knownAnalyses.end(); it++) 
	{
		if (it != knownAnalyses.begin())
			out << ", ";
		out << (*it)->getName();
	}
	out << std::endl;
	knownAnalyses.clear();
}

//=============================
// readAnalysis
//=============================
GTI_RETURN ApiCalls::readAnalysis (SpecificationNode node, Call *call, int analysisIndex)
{
	CalculationOrder order;
	std::string group;
	std::string name;

	std::map <int, Input*> inputMap;

	SpecificationNode child;
	Analysis* analysis;
	Mapping* mapping;

	//==Attributes
	if (!readAnalysisOperationAttributes (
			node, "analysis", &group, &name, &order))
		return GTI_ERROR;

	//==Find the Analysis
	if (name.find_first_of(':') != name.npos)
	{
		//Name is a fully qualified name of the form "<ModuleName>:<AnalysisFunctionName>"
		analysis = Analyses::getInstance()->findAnalysis(name,group);
		if (!analysis)
		{
			if (Analyses::getInstance()->hasGroup(group))
			{
				//Analysis group loaded, but not found -> critical
				std::cerr << "| |-->Error: an analysis for function \"" << call->getName() << "\" was not found, even though its analysis group was loaded (analysis-name: \"" << name << "\", group: \"" << group << "\")." << std::endl;
				listAvailableAnalysisFunctions(group, VERBOSE(2));
				return GTI_ERROR;
			}
			else
			{
				//named analysis group not loaded -> ok, just verbose output
				VERBOSE (2) << "| |-->Info: for function \"" << call->getName() << "\" analysis \"" << name << "\" of group \"" << group << "\" was not loaded as this analysis group was not loaded." << std::endl;
				return GTI_SUCCESS;
			}
		}
	}
	else
	{
		//Name is of the form "<ModuleName>"
		// If the named module exists and has only one analysis->use first analysis and warn
		// Else error
		AnalysisModule *module = Analyses::getInstance()->findAnalysisModule(name,group);
		if (!module)
		{
			if (Analyses::getInstance()->hasGroup(group))
			{
				//Analysis group loaded, but not found -> critical
				std::cerr << "| |-->Error: an analysis for function \"" << call->getName() << "\" was not found, even though its analysis group was loaded (analysis-name: \"" << name << "\", group: \"" << group << "\"). Warning: this was a partial analysis name that was not of the form <AnalysisModuleName>:<AnalysisFunctionName>." << std::endl;
				return GTI_ERROR;
			}
			else
			{
				//named analysis group not loaded -> ok, just verbose output
				VERBOSE (2) << "| |-->Info: for function \"" << call->getName() << "\" analysis \"" << name << "\" of group \"" << group << "\" was not loaded as this analysis group was not loaded. Warning: this was a partial analysis name that was not of the form <AnalysisModuleName>:<AnalysisFunctionName>." << std::endl;
				return GTI_SUCCESS;
			}
		}
		else
		{
			//Got the analysis module
			std::list<Analysis*> modAnalyses = module->getAnalyses();
			if (modAnalyses.empty() || modAnalyses.size() > 1)
			{
				//Analysis group loaded, but not found -> critical
				std::cerr << "| |-->Error: an analysis for function \"" << call->getName() << "\" was was not of the form <AnalysisModuleName>:<AnalysisFunctionName> -- assuming you just specified the name of the analysis module -- and there were either more than one or no analysis functions at all in this module; specify the full analysis with the column format! (analysis-name: \"" << name << "\", group: \"" << group << "\")." << std::endl;
				return GTI_ERROR;
			}

			std::cerr << "| |-->Warning: an analysis for function \"" << call->getName() << "\" was was not of the form <AnalysisModuleName>:<AnalysisFunctionName> -- assuming you just specified the name of the analysis module; consider using the full format! (analysis-name: \"" << name << "\", group: \"" << group << "\")." << std::endl;
			analysis = modAnalyses.front ();
		}

	}

		// If call is a callback default to order PRE
		// If call is set as POST, give out a warning
        if( call->isCallback() && order == ORDER_POST )
        {
            std::cerr
				<< "| |-->Warning: order of analysis \""
				<< name
				<< "\" is POST in "
                                << call->getName()
                                << ", defaulting to PRE."
				<< std::endl;
            order = ORDER_PRE;
        }
        
	//==Create the mapping
	std::list<Mapping*> mappings = analysis->getMappingsForCall(call);
	mapping = new Mapping (call, order, mappings.size(), analysisIndex);
	assert (mapping);

	//==Child: analysis-arguments (and sub-childs analysis-argument)
	child = node.findChildNodeNamedOrErr(
			"analysis-arguments",
			"| |-->Error: an analysis node has no \"analysis-arguments\" child.");
	if (!child) return GTI_ERROR;

	child = child.findChildNodeNamed("analysis-argument");
	while (child)
	{
		Input* pInput = NULL;
		int argOrder;

		//read the input argument
		if (readAnalysisInputArgument (child, call, &argOrder, &pInput) != GTI_SUCCESS)
			return GTI_ERROR;

		//add input to temporary map
		inputMap.insert (std::make_pair(argOrder, pInput));

		//next
		child = child.findSiblingNamed("analysis-argument");
	}

	//Add inputs to mapping
	std::map<int, Input*>::iterator i;
	int nextOrder = 0;
	for (i = inputMap.begin(); i != inputMap.end(); i++)
	{
		if (i->first != nextOrder)
		{
			std::cerr
				<< "| |-->Error: mapping of analysis \""
				<< name
				<< "\" to function \""
				<< call->getName()
				<< "\" specification uses a maxium order of "
				<< inputMap.rbegin()->first
				<< " though no argument for order "
				<< nextOrder
				<< " was given."
				<< std::endl;
			return GTI_ERROR;
		}

		mapping->addArgumentInput(i->second);

		//next
		nextOrder++;
	}

	//==Add the mapping to the analysis
	if (!analysis->addCallMapping(mapping))
		return GTI_ERROR;

	return GTI_SUCCESS;
}

//=============================
// readAnalysisOperationAttributes
//=============================
bool ApiCalls::readAnalysisOperationAttributes (SpecificationNode node, std::string nodeName, std::string *pGroup, std::string *pName, CalculationOrder *pOrder)
{
	std::string orderString;

	//==Attribute: group
	if (!node.getAttributeOrErr(
			"group",
			"| |-->Error: an " + nodeName + " node has no \"group\" attribute.",
			pGroup))
		return false;

	//==Attribute: name
	if (!node.getAttributeOrErr(
			"name",
			"| |-->Error: an " + nodeName + " node has no \"name\" attribute.",
			pName))
		return false;

	//==Attribute: order (pre/post)
	if (!node.getAttributeOrErr(
			"order",
			"| |-->Error: an " + nodeName + " node has no \"order\" attribute.",
			&orderString))
		return false;

	if (orderString == "pre")
	{
		*pOrder = ORDER_PRE;
	}
	else if (orderString == "post")
	{
		*pOrder = ORDER_POST;
	}
	else
	{
		std::cerr << "| |-->Error: an " + nodeName + " node uses an invalid value for its order attribute, valid are \"pre\" and \"post\"; the specification used \"" << orderString << "\"." << std::endl;
		return false;
	}

	return true;
}

//=============================
// readAnalysisInputArgument
//=============================
GTI_RETURN ApiCalls::readAnalysisInputArgument (
		SpecificationNode node,
		Call *call,
		int *pOutOrder,
		Input **ppInput)
{
	std::string type, orderString;
	int order;

	//==Attribute: type
	if (!node.getAttributeOrErr(
			"type",
			"| |-->Error: an analysis-argument node has no \"type\" attribute",
			&type))
		return GTI_ERROR;

	//==Attribute: analysis-arg-order
	if (!node.getAttributeOrErr(
			"analysis-arg-order",
			"| |-->Error: an analysis-argument node has no \"analysis-arg-order\" attribute",
			&orderString))
		return GTI_ERROR;
	*pOutOrder = order = atoi (orderString.c_str());

	//==Child: call-arg-name | op-name
	if (type == "mapped")
	{
		*ppInput = readInputOfArgumentType (
				node, "analysis-argument", call);
		if (!*ppInput) return GTI_ERROR;
	}
	else if (type == "operation" ||
			 type == "operation-len")
	{
		std::string opName, opGroup, idString;
		int id;
		Operation* op = NULL;

		SpecificationNode child = node.findChildNodeNamedOrErr(
				"op-name",
				"| |-->Error: an analysis-argument node of the " + type + " type has no \"op-name\" child.");
		if (!child) return GTI_ERROR;

		opName = child.getNodeContent();

		if (!child.getAttributeOrErr(
				"group",
				"| |-->Error: an op-name node has no \"group\" attribute.",
				&opGroup))
			return GTI_ERROR;

		if (!child.getAttributeOrErr(
				"id",
				"| |-->Error: a op-name node has no \"id\" attribute.",
				&idString))
			return GTI_ERROR;
		if (sscanf (idString.c_str(), "%d", &id) != 1)
		{
			std::cerr << "| |-->Error: an op-name node specified an invalid id attribute; specified was: \""<<idString<<"\", which is not an integer."<<std::endl;
			return GTI_ERROR;
		}

		//find operation
		op = Analyses::getInstance()->findOperation(opName, opGroup);

		if (!op)
		{
			std::cerr << "| |-->Error: an analysis-argument node of the " << type << " type uses an invalid op-name specification, could not find the operation named \"" << opName << "\" in group \"" << opGroup << "\"." << std::endl;
			return GTI_ERROR;
		}

		//check whether the operation has a valid mapping for this call
		if (!op->hasMappingForCall (call->getName(), call->getGroup()->getApiName(), id))
		{
			std::cerr << "| |-->Error: an analysis-argument of call \"" << call->getName() << "\" uses the operation \"" << op->getName() << "\" as input that is not mapped to that call with the given id \"" << id << "\"." << std::endl;
			return GTI_ERROR;
		}

		//create input
		if (type == "operation")
		{
			*ppInput = new OperationInput (op, id);
		}
		else
		{
			if (!op->hasArrayReturn())
			{
				std::cerr << "| |-->Error: an analysis argument node uses the operation-len type with operation \"" << op->getName() << "\" which returns no array and is thus invalid for this type." << std::endl;
				return GTI_ERROR;
			}
			*ppInput = new OperationInput (op, id, true);
		}
		assert (*ppInput);
	}
	else
	{
		std::cerr << "| |-->Error: an analysis-argument uses an invalid value for its \"type\" attribute, valid are \"mapped\", \"operation\", and \"operation-len\"; specified was \"" << type << "\"." << std::endl;
		return GTI_ERROR;
	}

	return GTI_SUCCESS;
}

//=============================
// readOperation
//=============================
GTI_RETURN ApiCalls::readOperation (SpecificationNode node, Call* call, int operationIndex)
{
	CalculationOrder order;
	std::string group;
	std::string name;
	std::string idString;
	int id;

	std::map <int, Input*> inputMap;

	SpecificationNode child;
	Operation* operation;
	Mapping* mapping;

	//==Attributes
	if (!readAnalysisOperationAttributes (
			node, "operation", &group, &name, &order))
		return GTI_ERROR;

	//read id attribute
	if (!node.getAttributeOrErr(
			"id",
			"| |-->Error: an operation node has no \"id\" attribute.",
			&idString))
		return GTI_ERROR;
	if (sscanf (idString.c_str(), "%d", &id) != 1)
	{
		std::cerr << "| |-->Error: given id for an operation node is invalid, speciefied was: \"" << idString << "\" which is not an integer." << std::endl;
		return GTI_ERROR;
	}

	//==Find the Operation
	operation = Analyses::getInstance()->findOperation(name,group);
	if (!operation)
	{
		if (Analyses::getInstance()->hasGroup(group))
		{
			//Analysis group loaded, but not found -> critical
			std::cerr << "| |-->Error: an operation for function \"" << call->getName() << "\" was not found, even though its analysis group was loaded (operation-name: \"" << name << "\", group: \"" << group << "\")." << std::endl;
			return GTI_ERROR;
		}
		else
		{
			//named analysis group not loaded -> ok, just verbose output
			VERBOSE (2) << "| |-->Info: for function \"" << call->getName() << "\" operation \"" << name << "\" of group \"" << group << "\" was not loaded as this analysis group was not loaded." << std::endl;
			return GTI_SUCCESS;
		}
	}

	//==Create the mapping
	mapping = new Mapping (call, order, id, operationIndex);
	assert (mapping);

	//==Child: operation-arguments (and sub-childs operation-argument)
	child = node.findChildNodeNamedOrErr(
			"operation-arguments",
			"| |-->Error: an operation node has no \"operation-arguments\" child.");
	if (!child) return GTI_ERROR;

	child = child.findChildNodeNamed("operation-argument");
	while (child)
	{
		Input* pInput = NULL;
		int argOrder;
		std::string argOrderString;

		//read the input argument
		//Attribute: op-arg-order
		if (!child.getAttributeOrErr(
				"op-arg-order",
				"| |-->Error: an operation-argument node has no \"op-arg-order\" attribute.",
				&argOrderString))
			return GTI_ERROR;
		argOrder = atoi (argOrderString.c_str());

		pInput = readInputOfArgumentType (child, "operation-argument", call);
		if (!pInput)
			return GTI_ERROR;

		//add input to temporary map
		inputMap.insert (std::make_pair(argOrder, pInput));

		//next
		child = child.findSiblingNamed("operation-argument");
	}

	//Add inputs to mapping
	std::map<int, Input*>::iterator i;
	int nextOrder = 0;
	for (i = inputMap.begin(); i != inputMap.end(); i++)
	{
		if (i->first != nextOrder)
		{
			std::cerr
			<< "| |-->Error: mapping of operation \""
			<< name
			<< "\" to function \""
			<< call->getName()
			<< "\" specification uses a maxium order of "
			<< inputMap.rbegin()->first
			<< " though no argument for order "
			<< nextOrder
			<< " was given."
			<< std::endl;
			return GTI_ERROR;
		}

		mapping->addArgumentInput(i->second);

		//next
		nextOrder++;
	}

	//==Add the mapping to the operation
	if (!operation->addCallMapping(mapping))
		return GTI_ERROR;

	return GTI_SUCCESS;
}

//=============================
// readInputOfArgumentType
//=============================
Input* ApiCalls::readInputOfArgumentType (SpecificationNode parent, std::string parentName, Call* call)
{
	std::string argName;
	Argument* arg = NULL;
	Input* ret = NULL;

	SpecificationNode child = parent.findChildNodeNamedOrErr(
			"call-arg-name",
			"| |-->Error: an " + parentName + " node of the mapped type has no \"call-arg-name\" child.");
	if (!child) return NULL;

	argName = child.getNodeContent();

	if (argName == "USE_CALLNAME")
	{
		ret = new CallNameInput (call);
	}
    else
    if (argName == "USE_CALLRETURN")
    {
        ret = new CallReturnInput (call);
    }
    else
    if (argName == "USE_CALLID")
    {
        ret = new CallIdInput (call);
    }
	else
	{
		arg = call->findArgument(argName);

		if (!arg)
		{
			std::cerr << "| |-->Error: an " + parentName + " node of the mapped type uses an invalid call-arg-name specification, could not find the argument named \"" << argName << "\"." << std::endl;
			return NULL;
		}

		ret = new ArgumentInput (arg);
	}

	assert (ret);
	return ret;
}

//=============================
// writeFunctionsAsDot
//=============================
GTI_RETURN ApiCalls::writeFunctionsAsDot (std::string baseFileName)
{
	for (size_t i = 0; i < myApis.size(); i++)
	{
		std::ofstream out ((baseFileName + myApis[i]->getApiName() + ".dot").c_str());

		std::list<Call*> calls = myApis[i]->getCalls();
		std::list<Call*>::iterator iter;

		out
			<< "digraph ApiCallsOverview" << std::endl
			<< "{" << std::endl
			<< "   subgraph cluster1" << std::endl
			<< "   {" << std::endl;

		for (iter = calls.begin(); iter != calls.end(); iter++)
		{
			Call *c = *iter;
			out << "      " << c->toDotNode () << std::endl;
		}

		out
			<< "      color=black;" << std::endl
			<< "      style=rounded;" << std::endl
			<< "      label=\"Functions in group: " << myApis[i]->getApiName() << "\";" << std::endl
			<< "   }" << std::endl
			<< "}" << std::endl;

		out.close ();
	}

	return GTI_SUCCESS;
}

//=============================
// getAllApiHeaders
//=============================
std::list<std::string> ApiCalls::getAllApiHeaders (void)
{
	std::list<std::string> ret;

	for (size_t i = 0; i < myApis.size(); i++)
	{
		std::list<std::string> temp = myApis[i]->getHeaders();
		std::list<std::string>::iterator iter;

		for (iter = temp.begin(); iter != temp.end(); iter++)
		{
			ret.push_back(*iter);
		}
	}

	return ret;
}

//=============================
// getFinalizerCalls
//=============================
std::list<Call*> ApiCalls::getFinalizerCalls (void)
{
	std::list<Call*> ret;

	for (size_t i = 0; i < myApis.size(); i++)
	{
		std::list<Call*> calls;
		std::list<Call*>::iterator iter;

		calls = myApis[i]->getCalls();

		for (iter = calls.begin(); iter != calls.end(); iter++)
		{
			Call* c = *iter;

			if (!c->isFinalizer())
				continue;

			ret.push_back (c);
		}
	}

	return ret;
}

/*EOF*/
