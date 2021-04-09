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
 * @file Analyses.cpp
 * 		@see gti::weaver::Analyses
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <map>
#include <iostream>
#include <fstream>

#include "Analyses.h"
#include "Verbose.h"

using namespace gti;
using namespace gti::weaver::analyses;

Analyses* Analyses::myInstance = NULL;

//=============================
// Analyses
//=============================
Analyses::Analyses (void)
 : myGroups ()
{
	/*Nothing to do*/
}

//=============================
// Analyses
//=============================
Analyses::~Analyses (void)
{
	for (size_t i = 0; i < myGroups.size(); i++)
	{
		if (myGroups[i])
			delete myGroups[i];
	}
	myGroups.clear ();
}

//=============================
// addGroup
//=============================
void Analyses::addGroup ( AnalysisGroup * add_object )
{
  myGroups.push_back(add_object);
}

//=============================
// removeGroup
//=============================
void Analyses::removeGroup ( AnalysisGroup * remove_object )
{
	int i, size = myGroups.size();
	for ( i = 0; i < size; ++i)
	{
		AnalysisGroup * item = myGroups.at(i);
		if(item == remove_object)
		{
			std::vector<AnalysisGroup *>::iterator it = myGroups.begin() + i;
			if (*it)
				delete (*it);
			myGroups.erase(it);
			return;
		}
	}
}

//=============================
// getInstance
//=============================
Analyses* Analyses::getInstance (void)
{
	if (myInstance == NULL)
		myInstance = new Analyses ();
	assert(myInstance);
	return myInstance;
}

//=============================
// load
//=============================
GTI_RETURN Analyses::load (std::list<std::string> analysisSpecificationXmls)
{
	VERBOSE(1) << "Loading Analyses ..." << std::endl;

	//Loop over all input XMLs
	//=================================
	xmlDocPtr document;
	SpecificationNode currentPointer;
	std::list<std::string>::iterator i;

	for (i = analysisSpecificationXmls.begin(); i != analysisSpecificationXmls.end(); i++)
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
			(xmlStrcmp(currentPointer()->name, (const xmlChar *) "analysis-specification") != 0))
		{
			std::cerr
				<< "| |-->"
				<< "Error: Document does not contains the root node (\"analysis-specification\")"
				<< "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
			if (currentPointer)
				std::cerr << "Found \"" << currentPointer()->name << "\" instead!" << std::endl;
			xmlFreeDoc(document);
			return GTI_ERROR;
		}

		if (readAnalysisSpecification (currentPointer) != GTI_SUCCESS)
			return GTI_ERROR;

		VERBOSE(1) << "| |--> SUCCESS" << std::endl;
	}

	if (addInternalOperations () != GTI_SUCCESS)
		return GTI_ERROR;

	VERBOSE(1) << "--> SUCCESS" << std::endl;

	return GTI_SUCCESS;
}

//=============================
// readAnalysisSpecification
//=============================
GTI_RETURN Analyses::readAnalysisSpecification (SpecificationNode root)
{
	std::string libPath, incPath, groupName;

	//==Attribute: path-to-libs
	if (!root.getAttributeOrErr(
			"path-to-libs",
			"| |-->Error: analysis-specification root node had no \"path-to-libs\" attribute.",
			&libPath))
		return GTI_ERROR;

	//==Attribute: include-path
	if (!root.getAttributeOrErr(
			"include-path",
			"| |-->Error: analysis-specification root node had no \"include-path\" attribute.",
			&incPath))
		return GTI_ERROR;

	//==Attribute: group-name
	if (!root.getAttributeOrErr(
			"group-name",
			"| |-->Error: analysis-specification root node had no \"group-name\" attribute.",
			&groupName))
			return GTI_ERROR;

	//==Create the analysis group
	AnalysisGroup* group = new AnalysisGroup (libPath, incPath, groupName);
	addGroup (group);

	//==Child: analyses
	SpecificationNode iter = root.findChildNodeNamedOrErr(
			"analyses",
			"| |-->Error: analysis-specification root node had no \"analyses\" child.");
	if (!iter)
		return GTI_ERROR;

	//loop over analysis sub nodes
	std::map<std::string, std::list<Dependency> > allDependencies; /* <AnalysisName, LIST-OF-DEPENDENCIES<GroupName, AnalysisName> > (if GroupName is empty the analysis is in this group)*/
	std::map<std::string, std::list<Dependency> > allReductions; /* <AnalysisName, LIST-OF-DEPENDENCIES<GroupName, AnalysisName> > (if GroupName is empty the analysis is in this group)*/

	iter = iter.findChildNodeNamed("analysis");
	while (iter)
	{
		//read analysis
		std::list<Dependency>	dependencies; //(groupName, analysisName)
		std::list<Dependency>	reductions; //(groupName, analysisName)
		std::string analysisName;

		if (readAnalysis (iter, group, &analysisName, &dependencies, &reductions) != GTI_SUCCESS)
			return GTI_ERROR;

		allDependencies.insert (std::make_pair (analysisName, dependencies));
		allReductions.insert (std::make_pair (analysisName, reductions));

		//next
		iter = iter.findSiblingNamed("analysis");
	}

	//==Apply the dependencies between the Analyses
	std::map<std::string, std::list<Dependency> >::iterator i;
	for (i = allDependencies.begin(); i != allDependencies.end(); i++)
	{
		AnalysisModule* analysis = group->findAnalysisModule(i->first);
		assert (analysis); //Implementation error if this fails

		std::list<Dependency>::iterator j;
		for (j = i->second.begin(); j != i->second.end(); j++)
		{
			//get the right group
			AnalysisGroup* dependGroup = group;
			if (j->groupName != "")
				dependGroup = getGroup (j->groupName);

			if (!dependGroup)
			{
				std::cerr << "| |-->Error: an analysis dependency within group \"" << group->getGroupName() << "\" specifies a dependency to an analysis in group \"" << j->groupName << "\" this group was not yet loaded, is the group name correct ? Is the group specification file order correct ?" << std::endl;
				return GTI_ERROR;
			}

			AnalysisModule* analysisDepend = dependGroup->findAnalysisModule(j->moduleName);

			if (!analysisDepend)
			{
				std::cerr << "| |-->Error: an analysis dependency of analysis \"" << i->first << "\" within group \"" << group->getGroupName() << "\" specifies the unknown analysis \"" << j->moduleName << "\" of group \"" << dependGroup->getGroupName() << "\"." << std::endl;
				return GTI_ERROR;
			}

			analysis->addDependency(analysisDepend, j->isSoft);
		}
	}

	//==Apply the reduction dependencies
	for (i = allReductions.begin(); i != allReductions.end(); i++)
	{
		AnalysisModule* analysis = group->findAnalysisModule(i->first);
		assert (analysis); //Implementation error if this fails

		std::list<Dependency>::iterator j;
		for (j = i->second.begin(); j != i->second.end(); j++)
		{
			//get the right group
			AnalysisGroup* dependGroup = group;
			if (j->groupName != "")
				dependGroup = getGroup (j->groupName);

			if (!dependGroup)
			{
				std::cerr << "| |-->Warning: an analysis dependency within group \"" << group->getGroupName() << "\" specifies a reduction dependency to an analysis in group \"" << j->groupName << "\" this group was not yet loaded, Is the group specification file order correct ?" << std::endl;
				continue;
			}

			AnalysisModule* analysisDepend = dependGroup->findAnalysisModule(j->moduleName);

			if (!analysisDepend)
			{
				std::cerr << "| |-->Warning: a reduction dependency of analysis \"" << i->first << "\" within group \"" << group->getGroupName() << "\" specifies the unknown analysis \"" << j->moduleName << "\" of group \"" << dependGroup->getGroupName() << "\" not able to use this reduction, did you forget to load an extra analysis specification ?" << std::endl;
				continue;
			}

			if (!analysisDepend->isReduction())
			{
				std::cerr << "| |-->Error: a reduction dependency of analysis \"" << i->first << "\" within group \"" << group->getGroupName() << "\" specifies the analysis \"" << j->moduleName << "\" of group \"" << dependGroup->getGroupName() << "\" which is not a reduction!" << std::endl;
				return GTI_ERROR;
			}

			if (!analysis->addSupportedReduction(analysisDepend))
				return GTI_ERROR;
		}
	}

	//==Child: operations
	iter = root.findChildNodeNamedOrErr(
			"operations",
			"| |-->Error: analysis-specification root node had no \"operations\" child.");
	if (!iter)
		return GTI_ERROR;

	//loop over operation sub nodes
	iter = iter.findChildNodeNamed("operation");
	while (iter)
	{
		//read operation
		if (readOperation (iter, group) != GTI_SUCCESS)
			return GTI_ERROR;

		//next
		iter = iter.findSiblingNamed("operation");
	}

	//==Verbose output
	VERBOSE (2) << "| |-> Loaded group: " << *group << std::endl;

	return GTI_SUCCESS;
}

//=============================
// readAnalysis
//=============================
GTI_RETURN Analyses::readAnalysis (
		SpecificationNode node,
		AnalysisGroup *group,
		std::string *pOutAnalysisName,
		std::list<Dependency> *pOutDependencies,
		std::list<Dependency> *pOutReductions)
{
	std::string name, subGroupName, integrityString;
	std::string registeredModuleName;
	std::string configModuleName;
	std::string interfaceType;
	std::string headerName;
	std::string listensToTimeoutsStr;
	std::string continuousStr;
	std::string isAddedAutomagicallyStr;
	bool isGlobal;
	std::string isGlobalString;
	bool isProcessGlobal;
	bool isLocalIntegrity = false;
	bool isReduction = false;
	bool listensToTimeouts = false;
	bool continuous = false;
	bool isAddedAutomagically = false;
	std::string reductionString;
	std::string isProcessGlobalString;
	std::string analysisFunctionName;
	std::vector<InputDescription*> argumentSpec;

	SpecificationNode child;

	//==Attribute: name
	if (!node.getAttributeOrErr(
			"name",
			"| |-->Error: analysis node has no \"name\" attribute.",
			&name))
		return GTI_ERROR;

	assert (pOutAnalysisName);
	*pOutAnalysisName = name;

	//==Attribute: sub-group (Optional)
	if (!node.getAttribute("sub-group",&subGroupName))
		subGroupName = "";

	//==Attribute: local-integrity (Optional)
	if (node.getAttribute("local-integrity",&integrityString))
	{
		if (integrityString == "yes")
			isLocalIntegrity = true;

		if (integrityString != "yes" && integrityString != "no")
		{
			std::cerr << "| |-->Error: an analysis node uses the \"local-integrity\" attribute with an invalid value (\"" << integrityString << "\"); valid are \"yes\" and \"no\". " << std::endl;
			return GTI_ERROR;
		}
	}

	//==Attribute: reduction (Optional)
	if (node.getAttribute("reduction",&reductionString))
	{
		if (reductionString == "yes")
			isReduction = true;

		if (reductionString != "yes" && reductionString != "no")
		{
			std::cerr << "| |-->Error: an analysis node uses the \"reduction\" attribute with an invalid value (\"" << reductionString << "\"); valid are \"yes\" and \"no\". " << std::endl;
			return GTI_ERROR;
		}
	}

	//==Attribute: listens-to-timeouts (Optional)
	if (node.getAttribute("listens-to-timeouts",&listensToTimeoutsStr))
	{
	    if (listensToTimeoutsStr == "yes")
	        listensToTimeouts = true;

	    if (listensToTimeoutsStr != "yes" && listensToTimeoutsStr != "no")
	    {
	        std::cerr << "| |-->Error: an analysis node uses the \"listens-to-timeout\" attribute with an invalid value (\"" << listensToTimeoutsStr << "\"); valid are \"yes\" and \"no\". " << std::endl;
	        return GTI_ERROR;
	    }
	}

	//==Attribute: listens-to-timeouts (Optional)
	if (node.getAttribute("continuous",&continuousStr))
	{
		if (continuousStr == "yes")
			continuous = true;

		if (continuousStr != "yes" && continuousStr != "no")
		{
			std::cerr << "| |-->Error: an analysis node uses the \"continuous\" attribute with an invalid value (\"" << continuousStr << "\"); valid are \"yes\" and \"no\". " << std::endl;
			return GTI_ERROR;
		}
	}

	//==Attribute: added-automagically (Optional)
	if (node.getAttribute("added-automagically",&isAddedAutomagicallyStr))
	{
	    if (isAddedAutomagicallyStr == "yes")
	        isAddedAutomagically = true;

	    if (isAddedAutomagicallyStr != "yes" && isAddedAutomagicallyStr != "no")
	    {
	        std::cerr << "| |-->Error: an analysis node uses the \"added-automagically\" attribute with an invalid value (\"" << isAddedAutomagicallyStr << "\"); valid are \"yes\" and \"no\". " << std::endl;
	        return GTI_ERROR;
	    }
	}

	//==Child: registered-name
	child = node.findChildNodeNamedOrErr(
			"registered-name",
			"| |-->Error: analysis node has no \"registered-name\" child.");
	if (!child) return GTI_ERROR;

	registeredModuleName = child.getNodeContent();

	//==Child: module-name
	child = node.findChildNodeNamedOrErr(
			"module-name",
			"| |-->Error: analysis node has no \"module-name\" child.");
	if (!child) return GTI_ERROR;

	configModuleName = child.getNodeContent();

	//==Child: header-name
	child = node.findChildNodeNamedOrErr(
			"header-name",
			"| |-->Error: analysis node has no \"header-name\" child.");
	if (!child) return GTI_ERROR;

	headerName = child.getNodeContent();

	//==Child: interface-type
	child = node.findChildNodeNamedOrErr(
			"interface-type",
			"| |-->Error: analysis node has no \"interface-type\" child.");
	if (!child) return GTI_ERROR;

	interfaceType = child.getNodeContent();

	//==Child: is-global
	child = node.findChildNodeNamedOrErr(
			"is-global",
			"| |-->Error: analysis node has no \"is-global\" child.");
	if (!child) return GTI_ERROR;

	isGlobalString = child.getNodeContent();

	if (isGlobalString != "0" && isGlobalString != "1")
	{
		std::cerr << "| |-->Error: is-global node has unknown value, valid are \"0\" and \"1\", \""<< isGlobalString <<"\" was specified." << std::endl;
		return GTI_ERROR;
	}

	if (isGlobalString == "0")
		isGlobal = false;
	else
		isGlobal = true;

	//==Child: is-process-global
	child = node.findChildNodeNamedOrErr(
			"is-process-global",
			"| |-->Error: analysis node has no \"is-process-global\" child.");
	if (!child) return GTI_ERROR;

	isProcessGlobalString = child.getNodeContent();

	if (isProcessGlobalString != "0" && isProcessGlobalString != "1")
	{
		std::cerr << "| |-->Error: is-process-global node has unknown value, valid are \"0\" and \"1\", \""<< isProcessGlobalString <<"\" was specified." << std::endl;
		return GTI_ERROR;
	}

	if (isProcessGlobalString == "0")
		isProcessGlobal = false;
	else
		isProcessGlobal = true;

	//==Child: dependencies
	child = node.findChildNodeNamedOrErr(
			"dependencies",
			"| |-->Error: analysis node has no \"dependencies\" child.");
	if (!child) return GTI_ERROR;

	assert (pOutDependencies);
	if (readDependencies (child, pOutDependencies) != GTI_SUCCESS)
		return GTI_ERROR;

	//==Child: reductions
	child = node.findChildNodeNamed("reductions");
	if (child)
	{
		assert (pOutReductions);
		if (readDependencies (child, pOutReductions) != GTI_SUCCESS)
			return GTI_ERROR;
	}

	//==Create the Analysis object
	AnalysisModule *analysisModule;
	if (subGroupName == "")
	{
		analysisModule = new AnalysisModule (
				name,
				registeredModuleName,
				configModuleName,
				interfaceType,
				headerName,
				group->getIncDir(),
				isGlobal,
				isProcessGlobal,
				isLocalIntegrity,
				isReduction,
				listensToTimeouts,
				continuous,
				isAddedAutomagically);
	}
	else
	{
		analysisModule = new AnalysisModule (
				name,
				registeredModuleName,
				configModuleName,
				interfaceType,
				headerName,
				group->getIncDir(),
				isGlobal,
				isProcessGlobal,
				isLocalIntegrity,
				isReduction,
				listensToTimeouts,
				continuous,
				isAddedAutomagically,
				subGroupName);
	}
	assert (analysisModule);
	if (!group->addAnalysisModule(analysisModule))
		return GTI_ERROR;

	//==Child: analysis-function
	child = node.findChildNodeNamedOrErr(
			"analysis-function",
			"| |-->Error: analysis node has not a single \"analysis-function\" child.");
	if (!child) return GTI_ERROR;

	while (child)
	{
		bool needsChannelId;

		//read analysis function
		if (readAnalysisFunction(
				child,
				&analysisFunctionName,
				&argumentSpec,
				&needsChannelId) != GTI_SUCCESS)
			return GTI_ERROR;

		//create new analysis function
		if (!analysisModule->addAnalysis (
				analysisFunctionName,
				argumentSpec,
				group,
				needsChannelId))
			return GTI_ERROR;

		group->addCalculation(analysisModule->findAnalysis(analysisFunctionName));

		argumentSpec.clear ();

		//next
		child = child.findSiblingNamed("analysis-function");
	}

	return GTI_SUCCESS;
}

//=============================
// readOperation
//=============================
GTI_RETURN Analyses::readOperation (SpecificationNode node, AnalysisGroup *group)
{
	std::string name;
	std::vector<InputDescription*> argumentSpec;
	std::string returnType;
	std::list<std::string> extraHeaders;
	std::string sourceTemplate;
	std::string cleanupTemplate = "";
	bool isArrayOp = false;
	std::string arrayLenType = "";

	SpecificationNode child;

	//==Attribute: name
	if (!node.getAttributeOrErr(
			"name",
			"| |-->Error: a operation node has no \"name\" attribute.",
			&name))
		return GTI_ERROR;

	//==Attribute: return-type
	if (!node.getAttributeOrErr(
			"return-type",
			"| |-->Error: an operation node has no \"return-type\" attribute.",
			&returnType))
		return GTI_ERROR;

	//==Child: extra-headers
	child = node.findChildNodeNamedOrErr(
			"extra-headers",
			"| |-->Error: an operation node has no \"extra-headers\" child node.");
	if (!child) return GTI_ERROR;
	//loop over header sub-children
	child = child.findChildNodeNamed("header");
	while (child)
	{
		extraHeaders.push_back (child.getNodeContent());

		//next header
		child = child.findSiblingNamed("header");
	}

	//==Child: operation-arguments
	child = node.findChildNodeNamedOrErr(
			"operation-arguments",
			"| |-->Error: a operation node has no \"operation-arguments\" child.");
	if (!child) return GTI_ERROR;

	if (readArguments (
			child,
			"operation-argument",
			name,
			&argumentSpec) != GTI_SUCCESS)
		return GTI_ERROR;

	//==Child: source-template
	child = node.findChildNodeNamedOrErr(
			"source-template",
			"| |-->Error: an operation node has no \"source-template\" child.");
	if (!child) return GTI_ERROR;
	sourceTemplate = child.getNodeContent();

	//==Optional child: source-template
	child = node.findChildNodeNamed("cleanup-template");

	if (child)
	{
		cleanupTemplate = child.getNodeContent();
	}

	//==Optional child: return-is-array
	child = node.findChildNodeNamed("return-is-array");

	if (child)
	{
		isArrayOp = true;
		arrayLenType = child.getNodeContent();
	}

	//==Create the Operation object
	if (!isArrayOp)
	{
		group->addCalculation(new Operation(
				name,
				argumentSpec,
				group,
				returnType,
				extraHeaders,
				sourceTemplate,
				cleanupTemplate));
	}
	else
	{
		group->addCalculation(new Operation(
				name,
				argumentSpec,
				group,
				returnType,
				extraHeaders,
				sourceTemplate,
				cleanupTemplate,
				arrayLenType));
	}

	return GTI_SUCCESS;
}

//=============================
// readDependencies
//=============================
GTI_RETURN Analyses::readDependencies (SpecificationNode node, std::list<Dependency> *pOutDependencies)
{
	Dependency depValue;
	assert (pOutDependencies);

	SpecificationNode depend = node.findChildNodeNamed("analysis-depend");

	while (depend)
	{
		std::string groupName = "";
		bool isSoft = false;
		std::string isSoftStr;

		//==Optional attribute group
		depend.getAttribute("group", &groupName);
		depValue.groupName = groupName;

		//==Optional attribute "soft"
		if (depend.getAttribute("soft", &isSoftStr))
		{
			if (isSoftStr != "yes" && isSoftStr != "no")
			{
				std::cerr << "Error: the \"soft\" attribute of a analysis-depend node is neither \"yes\" nor \"no\", the specified value was \"" << isSoftStr << "\"." << std::endl;
				return GTI_ERROR;
			}

			if (isSoftStr == "yes")
				isSoft = true;
		}
		depValue.isSoft = isSoft;

		//==Name of the dependent module
		depValue.moduleName = depend.getNodeContent();

		//read dependency
		pOutDependencies->push_back (depValue);

		//next
		depend = depend.findSiblingNamed("analysis-depend");
	}

	return GTI_SUCCESS;
}

//=============================
// readAnalysisFunction
//=============================
GTI_RETURN Analyses::readAnalysisFunction(
		SpecificationNode node,
		std::string *pOutFunctionName,
		std::vector<InputDescription*> *pOutArgumentSpec,
		bool *pOutNeedsChannelId)
{
	assert (pOutFunctionName && pOutArgumentSpec);
	std::string needsChannelIdStr;
	bool needsChannelId = false;

	//==Attribute: name
	if (!node.getAttributeOrErr(
			"name",
			"| |-->Error: a analysis-function node has no \"name\" attribute.",
			pOutFunctionName))
		return GTI_ERROR;

	//==Attribute: needs-channel-id
	if (node.getAttribute(
			"needs-channel-id",
			&needsChannelIdStr))
	{
		if (needsChannelIdStr != "no" && needsChannelIdStr != "yes")
		{
			std::cerr << "| |->Error: an analysis-function node has an invalid value for the \"needs-channel-id\" attribute. Valid values are \"no\" and \"yes\", \"" <<needsChannelIdStr << "\" was specified instead." << std::endl;
			return GTI_ERROR;
		}

		if (needsChannelIdStr == "yes")
			needsChannelId = true;
	}

	if (pOutNeedsChannelId)
		*pOutNeedsChannelId = needsChannelId;

	//==Children: analysis-argument
	return readArguments (node, "analysis-argument", *pOutFunctionName, pOutArgumentSpec);
}

//=============================
// readArguments
//=============================
GTI_RETURN Analyses::readArguments (
		SpecificationNode node,
		std::string argumentNodeName,
		std::string calculationName,
		std::vector<InputDescription*> *pOutArgumentSpec)
{
	//==Children: 'argumentNodeName'
	SpecificationNode argument = node.findChildNodeNamed(argumentNodeName);
	pOutArgumentSpec->clear ();

	while (argument)
	{
		std::string type,orderString, nameString, name ="";
		int order;

		//Attribute: order
		if (!argument.getAttributeOrErr(
				"order",
				"| |-->Error: an " + argumentNodeName + " node has no \"order\" attribute.",
				&orderString))
			return GTI_ERROR;
		order = atoi(orderString.c_str());

		if (((int) pOutArgumentSpec->size() > order) && (*pOutArgumentSpec)[order] != NULL) {
			std::cerr << "| |-->Error: an " + argumentNodeName + " node has more than one analysis-argument with attribute \"order\" = "
					<< order << "." << std::endl;
			return GTI_ERROR;
		}

		//Attribute: type
		if (!argument.getAttributeOrErr(
				"type",
				"| |-->Error: an " + argumentNodeName + " node has no \"type\" attribute.",
				&type))
			return GTI_ERROR;

		//Attribute: type
		if (argument.getAttribute("name", &nameString))
		{
			name = nameString;
		}
		else
		{
			name = "arg" + orderString;
		}

		//Add to vector
		if ((int) pOutArgumentSpec->size() < order + 1)
			pOutArgumentSpec->resize (order + 1, NULL);

		(*pOutArgumentSpec)[order] = new InputDescription (type, name);

		//next
		argument = argument.findSiblingNamed(argumentNodeName);
	}

	//check sanity of analysis-argument specification
	for (size_t i = 0; i < pOutArgumentSpec->size(); i++)
	{
		if ((*pOutArgumentSpec)[i] == NULL)
		{
			std::cerr
				<< "| |--> Error: specification of arguments in operation/analysis \""
				<< calculationName
				<< "\" is erroneous, you specified a maximum order of "
				<< pOutArgumentSpec->size() - 1
				<< " but no argument with order "
				<< i
				<< "!"
				<< std::endl;
			return GTI_ERROR;
		}
	}

	return GTI_SUCCESS;
}

//=============================
// writeAnalysesAsDot
//=============================
GTI_RETURN Analyses::writeAnalysesAsDot (std::string fileNameBase)
{
	for (size_t i = 0; i < myGroups.size(); i++)
	{
		std::ofstream out ((fileNameBase + (myGroups[i])->getGroupName() + ".dot").c_str());

		out << "digraph AnalysesOverview" << std::endl
				<< "{" << std::endl
				<< "   subgraph cluster1" << std::endl
				<< "   {" << std::endl
				<< "      unsichtbar1 [label=\"\", shape=circle, linecolor=white, style=invis];" << std::endl;

		//print operations
		std::list<Operation*> ops = myGroups[i]->getOperations();
		std::list<Operation*>::iterator opIter;
		for (opIter = ops.begin(); opIter != ops.end(); opIter++)
		{
			out << (*opIter)->toDotNode() << std::endl;
		}

		out << "     color=black;" << std::endl
				<< "     style=rounded;" << std::endl
				<< "     label=\"Operations\";" << std::endl
				<< "   }" << std::endl
				<< std::endl
				<< "   subgraph cluster2" << std::endl
				<< "   {" << std::endl
				<< "      unsichtbar2 [label=\"\", shape=circle, linecolor=white, style=invis];" << std::endl;

		//print analyses
		std::list<AnalysisModule*> analyses = myGroups[i]->getAnalysisModules();
		std::list<AnalysisModule*>::iterator analysisIter;
		std::map<std::string, std::list<AnalysisModule*> > subGroupMap; //maps sub-group names to list of analyses in it
		std::map<std::string, std::list<AnalysisModule*> >::iterator subGroupIter;
		std::map<AnalysisModule*, int> anModuleToClusterId; //Maps the id of its cluster to each analysis module

		//sort analyses into sub groups
		for (analysisIter = analyses.begin(); analysisIter != analyses.end(); analysisIter++)
		{
			std::string subGroup = (*analysisIter)->getSubGroupName();

			if (subGroupMap.find (subGroup) == subGroupMap.end())
			{
				std::list<AnalysisModule*> l;
				l.push_back (*analysisIter);
				subGroupMap.insert(std::make_pair(subGroup, l));
			}
			else
			{
				subGroupMap[subGroup].push_back(*analysisIter);
			}
		}

		//walk over the map
		int clusterid = 3;
		for (subGroupIter = subGroupMap.begin(); subGroupIter != subGroupMap.end(); subGroupIter++, clusterid++)
		{
			out
			<< "      subgraph cluster" << clusterid << std::endl
			<< "      {" << std::endl;

			for (analysisIter = subGroupIter->second.begin(); analysisIter != subGroupIter->second.end(); analysisIter++)
			{
				//Loop over all analysis functions in the analysis module
				std::list<Analysis*> anFunctions = (*analysisIter)->getAnalyses();
				std::list<Analysis*>::iterator fIter;

				clusterid++;
				out
				<< "         subgraph cluster" << clusterid << std::endl
				<< "         {" << std::endl;

				anModuleToClusterId.insert (std::make_pair((*analysisIter), clusterid));

				for (fIter = anFunctions.begin(); fIter != anFunctions.end(); fIter++)
				{
					out << "            " << (*fIter)->toDotNode() << std::endl;
				}

				out
				<< "            color=black;" << std::endl
				<< "            style=rounded;"<< std::endl
				<< "            label=\""<< (*analysisIter)->getName() << "\";" << std::endl
				<< "         }" << std::endl;
			}

			out
			<< "         color=black;" << std::endl
			<< "         style=rounded;"<< std::endl
			<< "         label=\""<< subGroupIter->first << "\";" << std::endl
			<< "      }" << std::endl;
		}

		//print dependencies
		std::list <AnalysisModule*> foreignGroupDepends;
		std::list <AnalysisModule*>::iterator foreignIter;
		std::string tempDependencies= "";

		for (int depKind = 0; depKind < 2;depKind++)
		{
			for (analysisIter = analyses.begin(); analysisIter != analyses.end(); analysisIter++)
			{
				std::list<AnalysisModule*> dependencies;
				std::list<AnalysisModule*>::iterator depIter;

				if (depKind == 0)
					dependencies = (*analysisIter)->getDependencies ();
				if (depKind == 1)
					dependencies = (*analysisIter)->getSupportedReductions();

				//Get cluster id of this module
				int id1 = anModuleToClusterId[*analysisIter];

				//Get an analysis of this module
				Analysis *x = NULL;
				if (!(*analysisIter)->getAnalyses().empty())
					x = (*analysisIter)->getAnalyses().front();

				for (depIter = dependencies.begin(); depIter != dependencies.end(); depIter++)
				{
					if ((*depIter)->getGroup() == (*analysisIter)->getGroup())
					{
						//Get cluster Ids of dependent module
						int id2 = anModuleToClusterId[*depIter];

						//Get an analysis of other module
						Analysis *y = NULL;
						if (!(*depIter)->getAnalyses().empty())
							y = (*depIter)->getAnalyses().front();

						if (x && y)
						{
							std::string extra = "";
							if (depKind == 1)
								extra = ", color=crimson, style=dashed";

							//intra group dependency
							out << "      " << x->getDotName () << "->" << y->getDotName() << "[ltail=cluster" << id1 << ", lhead=cluster" << id2 << extra << "];" << std::endl;
						}
					}
					else
					{
						if (x)
						{
							char tempId[32];
							sprintf (tempId, "%d", id1);

							std::string extra = ", color=azure3";
							if (depKind == 1)
								extra = ", color=crimson, style=dashed";

							//inter group dependency
							tempDependencies += (std::string)("      ") + x->getDotName() + (std::string)"->otherGroup_" + (*depIter)->getName() + (std::string)"[ltail=cluster" + (std::string)(tempId) + extra + "];\n";

							//Add to list of foreign analyses (if not already added)
							for (foreignIter = foreignGroupDepends.begin();foreignIter != foreignGroupDepends.end();foreignIter++)
							{
								if (*foreignIter == *depIter)
									break;
							}

							if (foreignIter == foreignGroupDepends.end())
							{
								foreignGroupDepends.push_back((*depIter));
							}
						}
					}
				}//For dependencies
			}//For analyses
		}//For dependency kinds

		out << "      color=black;" << std::endl
				<< "      style=rounded;"<< std::endl
				<< "      label=\"Analyses\";" << std::endl
				<< "   }" << std::endl
				<< std::endl;

		//print foreign analyses
		if (!foreignGroupDepends.empty())
		{
			out
			<< "   subgraph cluster" << clusterid << std::endl
			<< "   {" << std::endl;

			for (foreignIter = foreignGroupDepends.begin();foreignIter != foreignGroupDepends.end();foreignIter++)
			{
				out
				<< "      "
				<< "otherGroup_" + (*foreignIter)->getName()
				<< " [label=\"" << (*foreignIter)->getGroup()->getGroupName() << ":" << (*foreignIter)->getName() << "\", shape=Mrecord, fillcolor=azure3, style=filled];"
				<< std::endl;
			}

			out
			<< "      color=black;" << std::endl
			<< "      style=rounded;"<< std::endl
			<< "      label=\"Analyses in other groups\";" << std::endl
			<< "   }" << std::endl;

			out << tempDependencies;
		}//dependencies to other groups present

		out
		<< "   unsichtbar1->unsichtbar2 [style=invis];" << std::endl
		<< "}" << std::endl;

		out.close ();
	}
	return GTI_SUCCESS;
}

//=============================
// findOperation
//=============================
Operation* Analyses::findOperation (std::string opName, std::string groupName)
{
	AnalysisGroup* group = getGroup(groupName);
	if (group)
		return group->findOperation(opName);
	return NULL;
}

//=============================
// findAnalysis
//=============================
Analysis* Analyses::findAnalysis (std::string name, std::string groupName)
{
	AnalysisGroup* group = getGroup(groupName);
	if (group)
		return group->findAnalysis(name);
	return NULL;
}

//=============================
// findAnalysisModule
//=============================
AnalysisModule* Analyses::findAnalysisModule (std::string name, std::string groupName)
{
	AnalysisGroup* group = getGroup(groupName);
	if (group)
		return group->findAnalysisModule(name);
	return NULL;
}

//=============================
// getAnalyses
//=============================
std::list<Analysis*> Analyses::getAnalyses(std::string groupName)
{
    std::list<Analysis*> ret;

    //Are we looking for a specific group (compare returns != 0)
    if (groupName.compare("") != 0)
    {
        AnalysisGroup* group = getGroup(groupName);

        if (group)
            return group->getAnalyses();
        return ret;
    }

    //We are looking for all analyses in all groups
    for (size_t i = 0; i < myGroups.size(); i++)
    {
        std::list<Analysis*> analyses = (myGroups[i])->getAnalyses();
        ret.splice(ret.end(), analyses);
    }

	return ret;
}

//=============================
// hasGroup
//=============================
bool Analyses::hasGroup (std::string group)
{
	if (getGroup(group))
		return true;
	return false;
}

//=============================
// getGroup
//=============================
AnalysisGroup* Analyses::getGroup (std::string name)
{
	for (size_t i = 0; i < myGroups.size(); i++)
	{
		if (myGroups[i]->getGroupName() == name)
		{
			return myGroups[i];
		}
	}

	return NULL;
}

//=============================
// addTypeMissmatchWarning
//=============================
void Analyses::addTypeMissmatchWarning (std::string typeA, std::string typeB, std::string occurrenceDesc)
{
	std::string a,b;

	if (typeA < typeB)
	{
		a = typeA;
		b = typeB;
	}
	else
	{
		a = typeB;
		b = typeA;
	}

	if (myTypeWarnings.find(a) != myTypeWarnings.end())
	{
		//mismatch for type a already present
		myTypeWarnings[a].insert (std::make_pair(b, occurrenceDesc));
		return;
	}

	std::map<std::string, std::string> m;
	m.insert (std::make_pair(b,occurrenceDesc));
	myTypeWarnings.insert (
			std::make_pair(a, m));
}

//=============================
// printTypeMissmatchWarnings
//=============================
GTI_RETURN Analyses::printTypeMissmatchWarnings (std::ostream& out)
{
	std::map<std::string, std::map<std::string, std::string> >::iterator i;
	std::map<std::string, std::string>::iterator j;

	if (myTypeWarnings.empty())
		return GTI_SUCCESS;

	out << "Found the following potential type missmatches "
	    << "(This is a warning for developers and should be handled carefully):"
	    << std::endl;

	for (i = myTypeWarnings.begin(); i != myTypeWarnings.end (); i++)
	{
		for (j = i->second.begin(); j != i->second.end(); j++)
		{
			out << "\t-> " << i->first << "==" << j->first << "? (first occurence: " << j->second << ")" << std::endl;
		}
	}

	return GTI_SUCCESS;
}

//=============================
// writeMappingsAsDot
//=============================
GTI_RETURN Analyses::writeMappingsAsDot (std::string baseName)
{
	//====0) Create a legend
	{
		std::ofstream out ((baseName + "legend" + ".dot").c_str());

		out
			<< "digraph CallMappingLegend" << std::endl
			<< "{" << std::endl
			<< "   Call [label=\"Call\", shape=Mrecord, fillcolor=lightsalmon2, style=filled]" << std::endl
			<< "   Operation [label=\"Operation\", shape=Mrecord, fillcolor=olivedrab2, style=filled]" << std::endl
			<< "   Analysis [label=\"Analysis\", shape=Mrecord, fillcolor=lightblue2, style=filled]" << std::endl
			<< "   GlobalAnalysis [label=\"Global Analysis\", shape=Mrecord, fillcolor=deeppink2, style=filled]" << std::endl
			<< "   Reduction [label=\"Reduction\", shape=Mrecord, fillcolor=lightseagreen, style=filled]" << std::endl
			<< "   Invis1 [label=\"A\", shape=Mrecord]" << std::endl
			<< "   Invis2 [label=\"B\", shape=Mrecord]" << std::endl
			<< "   Invis3 [label=\"A\", shape=Mrecord]" << std::endl
			<< "   Invis4 [label=\"B\", shape=Mrecord]" << std::endl
			<< "" << std::endl
		    << "   Invis1->Invis2 [label=\"A is/returns input for B\"]" << std::endl
		    << "   Invis3->Invis4 [label=\"A is length argument for array B\", color=\"cadetblue\", style=dashed]" << std::endl
		    << "" << std::endl
		    << "   { rank = same; \"Call\"; \"Operation\"; \"Analysis\"; }" << std::endl
		    << "   { rank = same; \"Invis1\"; \"Invis2\"; }" << std::endl
		    << "   { rank = same; \"Invis3\"; \"Invis4\"; }" << std::endl
		    << "   Call->Invis1 [style=invis]" << std::endl
		    << "   Invis1->Invis3 [style=invis]" << std::endl
		    << "}" << std::endl;

		out.close();
	}

	//====1) Temp storage
	/*
	 * This map is used to prepare the data, it holds a list
	 * of calculations that are mapped for each API call with
	 * at least one mapping.
	 */
	std::map<Call*, std::list<Calculation*> > callToCalculations;

	//====2) Fill the map
	for (size_t i = 0; i < myGroups.size(); i++)
	{
		std::list<Analysis*> analyses = myGroups[i]->getAnalyses();
		std::list<Operation*> operations = myGroups[i]->getOperations();

		std::list<Calculation*> calculations;
		for (std::list<Analysis*>::iterator k = analyses.begin(); k != analyses.end(); k++) calculations.push_back(*k);
		for (std::list<Operation*>::iterator k = operations.begin(); k != operations.end(); k++) calculations.push_back(*k);

		std::list<Calculation*>::iterator iter;

		for (iter = calculations.begin(); iter != calculations.end(); iter++)
		{
			Calculation* a = *iter;

			std::vector<Mapping *> mappings = a->getCallMappings();
			for (size_t j = 0; j < mappings.size(); j++)
			{
				Mapping* m = mappings[j];

				if (callToCalculations.find(m->getApiCall()) == callToCalculations.end())
				{
					//add entry for call
					std::list<Calculation*> l;
					l.push_back (a);

					callToCalculations.insert (std::make_pair(m->getApiCall(), l));
				}
				else
				{
					//entry for call exists, add to list (avoid duplicates!)
					std::list<Calculation*>::iterator k;
					for (k = callToCalculations[m->getApiCall()].begin(); k != callToCalculations[m->getApiCall()].end(); k++)
					{
						if ((*k) == a)
							break;
					}

					if (k == callToCalculations[m->getApiCall()].end())
						callToCalculations[m->getApiCall()].push_back(a);
				}
			}//for mappings
		}//for analyses
	}//for analysis groups

	//====3) Loop over map and to the output

	//This loop creates two versions of the graphs, one with subgraphs for pre/post, one without
	for (int version = 0; version < 2; version++)
	{
		//Loop over all calls
		std::map<Call*, std::list<Calculation*> >::iterator iter;
		for (iter = callToCalculations.begin(); iter != callToCalculations.end(); iter++)
		{
			std::string versionStr = "_subgraphed";
			if (version == 1)
				versionStr = "";

			Call* c = iter->first;
			std::ofstream out ((baseName + c->getName() + versionStr + ".dot").c_str());

			//print header
			out
				<< "digraph CallMapping" << std::endl
				<< "{" << std::endl;

			//print call
			out << "   " << c->toDotNode() << std::endl << std::endl;

			//print two clusters for pre and post
			CalculationOrder orders[] = {ORDER_PRE, ORDER_POST};
			std::string orderNames[] = {("Pre"), ("Post")};
			int numOrders = 2;
			std::list<Calculation*>::iterator calcIter;

			for (int order = 0; order < numOrders; order++)
			{
				if (version == 0)
				{
					out
						<< "   subgraph cluster"<< order << std::endl
						<< "   {" << std::endl
						<< "      color=black;" << std::endl
						<< "      style=rounded;" << std::endl
						<< "      label=\"" << orderNames[order] <<"\";" << std::endl;
				}
				out
						<< "      temp" << orderNames[order] << "Node[style=invis, label=\"\", width=0, height=0];" << std::endl;

				//print calculations
				for (calcIter = iter->second.begin(); calcIter != iter->second.end(); calcIter++)
				{
					Calculation* calc = *calcIter;

					std::list<Mapping*> mappings = calc->getMappingsForCall(c);
					std::list<Mapping*>::iterator mappIter;

					for (mappIter = mappings.begin(); mappIter != mappings.end(); mappIter++)
					{
						if ((*mappIter)->getOrder() != orders[order])
							continue;

						if ((*mappIter)->getId() != -1)
							out << "      " << "id" << (*mappIter)->getId() << calc->toDotNodeBottomUp() << std::endl;
						else
							out << "      " << calc->toDotNodeBottomUp() << std::endl;
					}
				}

				if (version == 0)
					out << "   }" << std::endl;
				out << std::endl;
			}//nodes for pre and post checks

			//print mappings
			for (calcIter = iter->second.begin(); calcIter != iter->second.end(); calcIter++)
			{
				Calculation* calc = *calcIter;
				std::list<Mapping*> ms = calc->getMappingsForCall(c);
				std::list<Mapping*>::iterator msIter;
				for (msIter = ms.begin(); msIter != ms.end(); msIter++)
				{
					Mapping* m = *msIter;

					assert(m); //should not happen, due to map construction

					std::vector<Input*> inputs = m->getArgumentInputs();

					for (size_t x = 0; x < inputs.size(); x++)
					{
						Input* input = inputs[x];

						out
							<< "   "
							<< input->getDotInputNodeName (c->getName())
							<< "->";

						if (m->getId() != -1)
							out << "id" << m->getId();

						out
							<< calc->getDotName()
							<< ":Arg"
							<< x
							<< "[";

						if (input->isOpArrayLen())
							out << "label=\"len\"";

						out
							<< "];"
							<< std::endl;
					}
				}
			}

			//print length arguments and length operations!
			std::vector<Argument *> callArgs = c->getArguments();
			int tempCount = 0;
			for (size_t i = 0; i < callArgs.size(); i++)
			{
				Argument* arg = callArgs[i];
				std::string port = "";

				if (!arg)
					continue;

				if (!arg->isArray())
					continue;

				out
					<< "   "
					<< arg->getLengthVariableDotName (c->getName())
					<< "->";

				//special for call arg to call arg edges
				if (!arg->isArrayWithLengthOp())
				{
					out
						<< "tempNode" << tempCount
						<< "[color=\"cadetblue\", style=dashed, headport=w, arrowhead=none];"
						<< std::endl
						<< "   "
						<< "tempNode" << tempCount
						<< "->";
					port = ", tailport=e";
				}

				out
					<< arg->getArgumentDotName(c->getName())
					<< "[color=\"cadetblue\", headport=s, style=dashed" << port << "];"
					<<std::endl;

				if (!arg->isArrayWithLengthOp())
				{
					out << "   tempNode"<< tempCount <<"[style=invis, label=\"\", width=0, height=0];";
					tempCount++;
				}
			}

			if (tempCount != 0)
			{
				out << "tempNode0->tempPreNode[style=invis, weight=1000, lhead=cluster0];"<<std::endl
					<< "tempNode0->tempPostNode[style=invis, weight=1000, lhead=cluster1];"<<std::endl;
			}

			//print footer
			out << "}" << std::endl;

			out.close();
		}
	}//for versions

	return GTI_SUCCESS;
}

//=============================
// checkCorrectnessOfReductions
//=============================
GTI_RETURN Analyses::checkCorrectnessOfReductions (void)
{
	//Maps call and order to a reduction analysis, used to track whether multiple analyses are mapped on the same event
	std::map<std::pair<Call*, CalculationOrder>, Analysis*> reductionMappings;
	std::map<AnalysisModule*,bool> modulesToRemove;

	for (size_t i = 0; i < myGroups.size(); i++)
	{
		AnalysisGroup* group = myGroups[i];

		std::list<AnalysisModule*> anMods = group->getAnalysisModules();
		std::list<AnalysisModule*>::iterator iter;

		for (iter = anMods.begin(); iter != anMods.end(); iter++)
		{
			AnalysisModule* a = *iter;
			bool hasWrappEveryMapping = false;
			Call* wrappEveryMappingCall = NULL;

			//we only care about reductions
			if (!a->isReduction())
				continue;

			//Check:
			//  REMOVED: - Must be mapped to exactly one wrapp everywhere function
			//  - No API call must exist where more than one reduction is mapped to a pre or post
			std::list<Analysis*> analyses = a->getAnalyses();
			std::list<Analysis*>::iterator aIter;

			for (aIter = analyses.begin(); aIter != analyses.end();aIter++)
			{
				Analysis* analysis = *aIter;

				std::vector<Mapping*> mappings = analysis->getCallMappings();

				for (size_t j = 0; j < mappings.size(); j++)
				{
					Mapping *m = mappings[j];

					Call *c = m->getApiCall();
					CalculationOrder order = m->getOrder();

					if (c->isWrappedEverywhere())
					{
						//Too many mappings ?
						//if (hasWrappEveryMapping)
						//{
						//	std::cerr << " | --> Error: the reduction analysis \"" << a->getName() << "\" is mapped to more than one wrapp-everywhere calls, where it must be mapped to exactly one such call. Was mapped to \"" << c->getName() << "\" and \"" << wrappEveryMappingCall->getName() << "\"." << std::endl;
						//	return GTI_ERROR;
						//}

						hasWrappEveryMapping = true;
						wrappEveryMappingCall = c;
					} /*Is a mapping to a wrapp everywhere call*/

					//Store this mapping in list of mappings
					if (reductionMappings.find(std::make_pair(c,order)) == reductionMappings.end())
					{
						reductionMappings.insert (std::make_pair(std::make_pair(c,order),analysis));
					}
					else
					{
						Analysis *other = reductionMappings[std::make_pair(c,order)];
						if (other->getModule() != a)
							//This analysis module will be removed due to being mapped to a call used by another reduction analysis module already
							if (modulesToRemove.find(a) == modulesToRemove.end())
							{
								modulesToRemove.insert(std::make_pair (a, true));
								std::cerr << " | --> Warning: two reductions are mapped to the call \"" << c->getName() << "\" with the same order (pre/post), thus the weaver is going to keep the reduction \"" << other->getModule()->getName() << "\" and drop the reduction \"" << a->getName() << "\"!"<< std::endl;
							}
					}
				}/*For mappings of analysis*/
			}/*For analysis functions in analysis module*/

			//No mapping to wrapp everywhere ?
			//if (!hasWrappEveryMapping)
			//{
			//	std::cerr << " | --> Error: the reduction analysis \"" << a->getName() << "\" is not mapped to a wrapp-everywhere call! A reduction must be mapped to exactly one such call that is used to create the reduced representation of the reduced records." << std::endl;
			//	return GTI_ERROR;
			//}

		} /*For analysis modules in analysis group*/
	} /*For analysis groups*/

	//Remove all reductions that can't be used
	std::map<AnalysisModule*,bool>::iterator remIter;
	for (remIter = modulesToRemove.begin(); remIter != modulesToRemove.end(); remIter++)
	{
		AnalysisModule *a = remIter->first;

		//Loop over all analysis functions, get their group and call "removeCalculation" for this group and the analysis function
		//Delete the analysis afterwards
		std::list<Analysis*> analyses = a->getAnalyses();
		std::list<Analysis*>::iterator aIter;
		for (aIter = analyses.begin(); aIter != analyses.end(); aIter++)
		{
			Analysis* an = *aIter;

			an->getGroup()->removeCalculation(an);
			delete (an);
		}

		//Remove the analysis module from the analysis group
		//Delete the analysis module
		a->getGroup()->removeAnalysisModule(a);
		delete (a);
	}

	return GTI_SUCCESS;
}

//=============================
// addInternalOperations
//=============================
GTI_RETURN Analyses::addInternalOperations (void)
{
	/*
	 * Operation to initialize a channel ID
	 */
	AnalysisGroup* channGroup = new AnalysisGroup ("", "", "GTI_Internal");
	addGroup (channGroup);

	std::list<std::string> headers;
	headers.push_back("stdint.h");
	std::vector<InputDescription*> args;
	Operation *channOp = new Operation (
			"gtiChannelIdOp",
			args,
			channGroup,
			"uint64_t",
			headers,
			"uint64_t RETURN = 0;",
			"");
	channGroup->addCalculation(channOp);

	/**
	 * @todo this is not perfectly clean, it uses implicits from the wrapper generator implementation and the channel identifier implementation!
	 */
	Operation *channOpStrided = new Operation (
	        "gtiChannelIdOpStrided",
	        args,
	        channGroup,
	        "uint64_t",
	        headers,
	        "uint64_t RETURN = 0; if (place->myNextEventStride != 1) RETURN = (((uint64_t)place->myNextEventOffset) << 32) + (uint64_t)place->myNextEventStride; place->myNextEventOffset = 0; place->myNextEventStride = 1;",
	        "");
	channGroup->addCalculation(channOpStrided);

	return GTI_SUCCESS;
}

/*EOF*/
