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
 * @file Layout.cpp
 * 		@see gti::weaver::LevelGraph
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <set>
#include <iostream>
#include <fstream>

#include "Layout.h"
#include "Verbose.h"
#include "Gti.h"
#include "Analyses.h"
#include "ApiCalls.h"

using namespace gti::weaver::calls;
using namespace gti::weaver::layout;
using namespace gti::weaver::modules;

Layout* Layout::ourInstance = NULL;

//=============================
// Layout
//=============================
Layout::Layout ( )
 : myLevels (),
   myRoot (NULL),
   myDefaultComm (NULL),
   myCommunications (),
   myProfiling (false)
{
    if (getenv("GTI_PROFILE") != NULL)
    {
        if (atoi(getenv("GTI_PROFILE")) == 1)
            myProfiling = true;
    }
}

//=============================
// ~Layout
//=============================
Layout::~Layout ( )
{
	//free levels
	std::map<int, Level*>::iterator iter;
	for (iter=myLevels.begin(); iter != myLevels.end(); iter++)
	{
		if (iter->second)
			delete (iter->second);
	}
	myLevels.clear ();
	myRoot = NULL; //already freed as in level list

	//free communications
	std::list<Communication*>::iterator iter2;
	for (iter2 = myCommunications.begin(); iter2 != myCommunications.end(); iter2++)
	{
		if (*iter2)
			delete (*iter2);
	}
	myCommunications.clear();

	myDefaultComm = NULL; //already freed, as in communications list
}

//=============================
// getInstance
//=============================
Layout* Layout::getInstance (void)
{
	if (!ourInstance)
	{
		ourInstance = new Layout ();
		assert (ourInstance);
	}

	return ourInstance;
}

//=============================
// load
//=============================
GTI_RETURN Layout::load (std::string layoutXml)
{
	//Open the input XML
	//=================================
	VERBOSE(1) << "Loading Layout ..." << std::endl;
	xmlDocPtr document;
	SpecificationNode currentPointer, child, subchild;

	document = xmlParseFile(layoutXml.c_str());

	if (document == NULL )
	{
		std::cerr << "Error loading input XML (" << layoutXml << ")" << "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
		return GTI_ERROR;
	}

	currentPointer = xmlDocGetRootElement(document);

	if (currentPointer == NULL ||
		(xmlStrcmp(currentPointer()->name, (const xmlChar *) "layout-specification") != 0))
	{
		std::cerr
			<< "Error: Document does not contains the root node (\"layout-specification\")"
			<< "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
		if (currentPointer)
			std::cerr << "Found \"" << currentPointer()->name << "\" instead!" << std::endl;
		xmlFreeDoc(document);
		return GTI_ERROR;
	}

	//Child: levels, level sub-childs
	//=================================
	child = currentPointer.findChildNodeNamedOrErr("levels", "|->Error: root node has no \"levels\" node.");
	if (!child) return GTI_ERROR;

	subchild = child.findChildNodeNamed("level");
	while (subchild)
	{
		//read level node
		if (readLevel (subchild) != GTI_SUCCESS)
			return GTI_ERROR;

		//next
		subchild = subchild.findSiblingNamed("level");
	}

	if (!myRoot)
	{
		std::cerr << "|->Error: the layout specification has no level with order 0, which represents the application." <<std::endl;
		return GTI_ERROR;
	}

	//Child: communications
	//=================================
	child = currentPointer.findChildNodeNamedOrErr("communications", "|->Error: root node has no \"communications\" node.");
	if (!child) return GTI_ERROR;

	if (readCommunications (child) != GTI_SUCCESS)
		return GTI_ERROR;

	//==Verbose
	std::map<int, Level*>::iterator i;
	for (i = myLevels.begin(); i != myLevels.end(); i++)
		VERBOSE (2) << "|-> Loaded level: " << *(i->second) << std::endl;

	VERBOSE(1) << "--> SUCCESS" <<std::endl;

	return GTI_SUCCESS;
}

//=============================
// readCommunications
//=============================
GTI_RETURN Layout::readCommunications (SpecificationNode node)
{
	SpecificationNode child, protocolNode, strategyNode;

	//==Child: default
	if (myDefaultComm)
		delete (myDefaultComm);

	CommStrategy *pStrategy = NULL;
	CommProtocol *pProtocol = NULL;
	std::list <Setting*> stratSettings;
	std::list <Setting*> protSettings;

	child = node.findChildNodeNamedOrErr("default", "|->Error: a communications node has no \"default\" child.");
	if (!child) return GTI_ERROR;

	strategyNode = child.findChildNodeNamedOrErr("comm-strategy", "|->Error: a default node has no \"comm-strategy\" child.");
	protocolNode = child.findChildNodeNamedOrErr("comm-protocol", "|->Error: a default node has no \"comm-protocol\" child.");
	if (!strategyNode || !protocolNode) return GTI_ERROR;

	if (readCommStrategy(strategyNode, &pStrategy, NULL, &stratSettings) != GTI_SUCCESS)
		return GTI_ERROR;

	if (readCommProtocol(protocolNode, &pProtocol, &protSettings) != GTI_SUCCESS)
		return GTI_ERROR;

	myDefaultComm = new Communication (pStrategy, pProtocol);
	assert (myDefaultComm);
	myCommunications.push_back (myDefaultComm);

	std::list <Setting*>::iterator i;
	for (i = stratSettings.begin(); i != stratSettings.end(); i++)
	{
		if (!myDefaultComm->addStrategySetting(*i))
			return GTI_ERROR;
	}

	for (i = protSettings.begin(); i != protSettings.end(); i++)
	{
		if (!myDefaultComm->addProtocolSetting(*i))
			return GTI_ERROR;
	}

	//==Child: connection
	child = node.findChildNodeNamed("connection");

	while (child)
	{
		//read connection
		if (readConnection (child) != GTI_SUCCESS)
			return GTI_ERROR;

		//next
		child = child.findSiblingNamed("connection");
	}

	return GTI_SUCCESS;
}

//=============================
// readLevel
//=============================
GTI_RETURN Layout::readLevel (SpecificationNode node)
{
	int order;
	uint64_t size;
	std::string placeName, orderString, sizeString;
	Place* place = NULL;
	Level* level = NULL;

	SpecificationNode child, subchild;

	//==Attribute: order
	if (!node.getAttributeOrErr(
			"order",
			"|->Error: a level node has no \"order\" attribute.",
			&orderString))
		return GTI_ERROR;
	order = atoi (orderString.c_str ());

	//==Attribute: place-name
	if (order != 0) //order 0 is application and needs no place
	{
		if (!node.getAttributeOrErr(
				"place-name",
				"|->Error: a level node has no \"place-name\" attribute.",
				&placeName))
			return GTI_ERROR;

		place = Gti::getInstance()->findPlace (placeName);
		if (!place)
		{
			std::cerr << "|->Error: place with name \""<< placeName <<"\"" << " could not be found." << std::endl;
			return GTI_ERROR;
		}
	}else if (node.getAttribute("place-name", &placeName)){ //order 0 might be an application thread
		place = Gti::getInstance()->findPlace (placeName);
		if (!place)
		{
			std::cerr << "|->Error: place with name \""<< placeName <<"\"" << " could not be found." << std::endl;
			return GTI_ERROR;
		}
	}

	//==Attribute: size
	if (!node.getAttributeOrErr(
			"size",
			"|->Error: a level node has no \"size\" attribute.",
			&sizeString))
		return GTI_ERROR;
	sscanf (sizeString.c_str(), "%" SCNu64, &size);

	//==Create the level
	level = new Level (order, size, place);

	if (order == 0)
		myRoot = level;

	//==Child: analyses, subchild: analysis
	child = node.findChildNodeNamedOrErr("analyses", "|->Error: a level node has no \"analyses\" child.");
	if (!child) return GTI_ERROR;

	subchild = child.findChildNodeNamed("analysis");
	while (subchild)
	{
		std::string name, group;
		AnalysisModule *anModule;

		//=Read Attribute: name
		if (!subchild.getAttributeOrErr(
				"name",
				"|->Error: an analysis node has no \"name\" attribute.",
				&name))
			return GTI_ERROR;

		//=Read Attribute: group
		if (!subchild.getAttributeOrErr(
				"group",
				"|->Error: an analysis node has no \"group\" attribute.",
				&group))
			return GTI_ERROR;

		//=Search analysis
		anModule = Analyses::getInstance()->findAnalysisModule(name,group);
		if (!anModule)
		{
			std::cerr << "|->Error: an analysis node specified an unknown analysis of name \"" << name << "\" of group \"" << group << "\"." << std::endl;
			return GTI_ERROR;
		}

		if (anModule->isReduction())
		{
			std::cerr << "|->Error: the level with order \"" << orderString << "\" tried to place the reduction with name \"" << name << "\" of group \"" << group << "\". Reductions mut not be placed on levels, they are rather placed automatically by the weaver if possible." << std::endl;
			return GTI_ERROR;
		}

		//=Add analysis to level
		level->addAnalysisModule(anModule);

		//next
		subchild = subchild.findSiblingNamed("analysis");
	}

	//==Store the level in internal map
	if (myLevels.find(order) != myLevels.end())
	{
		std::cerr << "|->Error: two levels with order " << order << " exist." << std::endl;
		return GTI_ERROR;
	}
	myLevels.insert (std::make_pair(order, level));

	return GTI_SUCCESS;
}

//=============================
// readCommStrategy
//=============================
GTI_RETURN Layout::readCommStrategy(
		SpecificationNode node,
		CommStrategy **ppOutStrategy,
        CommStrategyIntra **ppOutStrategyIntra,
		std::list<Setting*> *pOutSettings)
{
	std::string name;

	assert ((ppOutStrategy || ppOutStrategyIntra) && pOutSettings);

	//Child: name
	if (!node.getAttributeOrErr(
			"name",
			"|->Error: a comm-strategy node has no \"name\" attribute.",
			&name))
		return GTI_ERROR;

	//find the stategy
	if (ppOutStrategy)
	{
	    *ppOutStrategy = Gti::getInstance()->findStrategy(name);

	    if (!*ppOutStrategy)
	    {
	        std::cerr << "|->Error: could not find communication strategy with name \"" << name << "\" in GTI." << std::endl;
	        return GTI_ERROR;
	    }
	}
	else if (ppOutStrategyIntra)
	{
	    *ppOutStrategyIntra = Gti::getInstance()->findStrategyIntra(name);

	    if (!*ppOutStrategyIntra)
	    {
	        std::cerr << "|->Error: could not find intra communication strategy with name \"" << name << "\" in GTI." << std::endl;
	        return GTI_ERROR;
	    }
	}
	else
	{
	    return GTI_ERROR;
	}

	//read the settings
	SpecificationNode child = node.findChildNodeNamedOrErr(
			"settings",
			"|->Error: a comm-strategy node has no \"settings\" child.");
	if (!child) return GTI_ERROR;

	if (readSettings (child, pOutSettings) != GTI_SUCCESS)
		return GTI_ERROR;

	return GTI_SUCCESS;
}

//=============================
// readCommProtocol
//=============================
GTI_RETURN Layout::readCommProtocol(
		SpecificationNode node,
		CommProtocol **ppOutProtocol,
		std::list<Setting*> *pOutSettings)
{
	std::string name;

	assert (ppOutProtocol && pOutSettings);

	//Child: name
	if (!node.getAttributeOrErr(
			"name",
			"|->Error: a comm-protocol node has no \"name\" attribute.",
			&name))
		return GTI_ERROR;

	//find the stategy
	*ppOutProtocol = Gti::getInstance()->findProtocol(name);
	if (!*ppOutProtocol)
	{
		std::cerr << "|->Error: could not find communication protocol with name \"" << name << "\" in GTI." << std::endl;
		return GTI_ERROR;
	}

	//read the settings
	SpecificationNode child = node.findChildNodeNamedOrErr(
			"settings",
			"|->Error: a comm-protocol node has no \"settings\" child.");
	if (!child) return GTI_ERROR;

	if (readSettings (child, pOutSettings) != GTI_SUCCESS)
		return GTI_ERROR;

	return GTI_SUCCESS;
}

//=============================
// readCommProtocol
//=============================
GTI_RETURN Layout::readSettings (SpecificationNode node, std::list<Setting*> *pOutSettings)
{
	SpecificationNode child;

	child = node.findChildNodeNamed("setting");

	while (child)
	{
		std::string name, value;
		Setting *setting;

		//Attribute: name
		if (!child.getAttributeOrErr(
				"name",
				"|->Error: a setting node has no \"name\" attribute.",
				&name))
			return GTI_ERROR;

		//Attribute: value
		if (!child.getAttributeOrErr(
				"value",
				"|->Error: a setting node has no \"value\" attribute.",
				&value))
			return GTI_ERROR;

		//create and add setting to list
		setting = new Setting (name, value);
		pOutSettings->push_back (setting);

		//next
		child = child.findSiblingNamed("setting");
	}

	return GTI_SUCCESS;
}

//=============================
// readConnection
//=============================
GTI_RETURN Layout::readConnection (SpecificationNode node)
{
	std::string topString, bottomString;
	int top, bottom;
	Communication *comm = NULL;
	DistributionType distrib = DISTRIBUTION_UNIFORM;
	std::string distribString, blockString;
	int blocksize = 0;

	SpecificationNode stratNode, protNode;
	Level *topLevel, *bottomLevel;

	//==Attribute: bottom-level
	if (!node.getAttributeOrErr(
			"bottom-level",
			"|->Error: a connection node has no \"bottom-level\" attribute.",
			&bottomString))
		return GTI_ERROR;
	sscanf (bottomString.c_str(), "%d", &bottom);

	//find the bottom level
	if (myLevels.find (bottom) == myLevels.end())
	{
		std::cerr << "|->Error: a connection uses an unknown order id for its bottom level: " << bottom << "." << std::endl;
		return GTI_ERROR;
	}
	bottomLevel = myLevels[bottom];

	//==Attribute: top-level
	if (!node.getAttributeOrErr(
			"top-level",
			"|->Error: a connection node has no \"top-level\" attribute.",
			&topString))
		return GTI_ERROR;
	sscanf (topString.c_str(), "%d", &top);

	//find the top level
	if (myLevels.find (top) == myLevels.end())
	{
		std::cerr << "|->Error: a connection uses an unknown order id for its top level: " << top << "." << std::endl;
		return GTI_ERROR;
	}
	topLevel = myLevels[top];

	//==Attribute: distribution [optional]
	if (node.getAttribute("distribution", &distribString))
	{
	    if (distribString != "uniform" && distribString != "by-block")
	    {
	        std::cerr << "|->Error: a connection uses an unknown distribution type, valid values are \"uniform\" and \"by-block\"; Specified was: " << distribString << "!" << std::endl;
	        return GTI_ERROR;
	    }

	    if (distribString == "by-block")
	    {
	        distrib = DISTRIBUTION_BY_BLOCK;

	        if (!node.getAttributeOrErr(
	                "blocksize",
	                "|->Error: a connection uses a \"by-block\" distribution but specifies no \"blocksize\" argument! A blocksize needs to be given for this distribution type.",
	                &blockString))
	            return GTI_ERROR;

	        sscanf (blockString.c_str(), "%d", &blocksize);

	        if (blocksize <= 0)
	        {
	            std::cerr << "|->Error: an invalid blocksize argument was specified in a connection node. The specified value was \"" << blocksize << "\", which must be a value > 0!" << std::endl;
	            return GTI_ERROR;
	        }
	    }
	}

	//==Childs: comm-strategy, comm-protocol (optional)
	stratNode = node.findChildNodeNamed("comm-strategy");
	protNode = node.findChildNodeNamed("comm-protocol");

	//No default for intra communication
	if ((!stratNode || !protNode) &&
	    bottom == top)
	{
	    std::cerr << "| -> ERROR: a intra layer connection speciefied no strategy+protocol, intra connections must not use a default communication! (intra connection of layer " << top << ")" << std::endl;
	    return GTI_ERROR;
	}

	//Evaluate communication specification
	if (!stratNode && !protNode)
	{
		//nothing specified -> use default
		comm = myDefaultComm;
	}
	else
	{
		//something specified
	    CommStrategyIntra *stratIntra = NULL;
		CommStrategy *strat = myDefaultComm->getCommStrategy();
		CommProtocol *prot = myDefaultComm->getCommProtocol();

		std::list<Setting*> stratSettings, protSettings;

		//strategy given ?
		if (stratNode)
		{
			//read strategy
		    if (top != bottom) //inter
		    {
                if (readCommStrategy(stratNode, &strat, NULL, &stratSettings) != GTI_SUCCESS)
                    return GTI_ERROR;
		    }
		    else //intra
		    {
		        if (readCommStrategy(stratNode, NULL, &stratIntra, &stratSettings) != GTI_SUCCESS)
		            return GTI_ERROR;
		    }
		}
		else
		{
			//use default strategy setting
			std::vector<Setting*> temp = myDefaultComm->getStrategySettings();
			for (int i = 0; i < temp.size(); i++)
			{
				stratSettings.push_back (temp[i]->clone());
			}
		}

		//protocol given
		if (protNode)
		{
			//read protocol
			if (readCommProtocol(protNode, &prot, &protSettings) != GTI_SUCCESS)
				return GTI_ERROR;
		}
		else
		{
			//use default protocol settings
			std::vector<Setting*> temp = myDefaultComm->getProtocolSettings();
			for (int i = 0; i < temp.size(); i++)
			{
				protSettings.push_back (temp[i]->clone());
			}
		}

		//Consistency, if we have a intra communication, we also need a protocol that
		//supports intra communication
		if (top == bottom && !prot->supportsIntraComm())
		{
		    std::cerr << "|->Error: an intra connection on layer " << top << " uses a communication protocol that does not supports intra communication." << std::endl;
		}

		//Consistency: No intra communiaction on the application layer
		if (top == 0 && bottom == 0)
		{
		    std::cerr << "|->Error: intra connection on the application layer is not allowed." << std::endl;
		}

		//add new communication
		if (top != bottom) //inter
		    comm = new Communication (strat, prot, stratSettings, protSettings);
		else //intra
		    comm = new Communication (stratIntra, prot, stratSettings, protSettings);

		assert (comm);
		myCommunications.push_back (comm);
	}

	if (top != bottom)
	{
        //==create adjacency
        Adjacency *adjOutList = new Adjacency (topLevel, comm, distrib, blocksize);
        Adjacency *adjInList = new Adjacency (bottomLevel, comm, distrib, blocksize);
        assert (adjOutList && adjInList);

        //==Add to levels (to in list, to out list of other level)
        topLevel->addInArc(adjInList);
        bottomLevel->addOutArc(adjOutList);
	}
	else
	{
	    if (topLevel->hasIntraCommunication())
	    {
	        std::cerr << "|->Error: two intra communication specifications are given for layer " << top << "." << std::endl;
	        return GTI_ERROR;
	    }

	    topLevel->setIntraCommunication(comm);
	}

	return GTI_SUCCESS;
}

//=============================
// writeLayoutAsDot
//=============================
GTI_RETURN Layout::writeLayoutAsDot (std::string fileName)
{
	std::ofstream out (fileName.c_str());
	std::map<int, Level*>::iterator i;

	//print header
	out
		<< "digraph Layout" << std::endl
		<< "{" << std::endl;

	//print nodes
	for (i = myLevels.begin(); i != myLevels.end(); i++)
	{
		std::string color = "lightblue2";
		if (i->first == 0)
			color = "olivedrab2";

		out
			<< "    " << i->second->getOrder()
			<< "[label=\"{Level " << i->second->getOrder()
			<< " @ "
			<< i->second->getSize();

		if (i->first == 0)
			out <<"| Application";
		else
			out <<"| Place: " << i->second->getPlace()->getName();

		std::list<AnalysisModule*> analyses = i->second->getAnalysisModules();
		std::list<AnalysisModule*>::iterator iter;

		if (analyses.size() != 0)
		{
			out << "|{ |{";

			for (iter = analyses.begin(); iter != analyses.end(); iter++)
			{
				if (iter != analyses.begin())
					out << "|";

				if ((*iter)->isReduction())
					out << "[Reduction] ";

				out << (*iter)->getName();
			}

			out << "}}";
		}

		out
			<< "}\", shape=Mrecord, fillcolor=" << color << ", style=filled];" << std::endl;
	}

	//print connections
	for (i = myLevels.begin(); i != myLevels.end(); i++)
	{
		 std::vector<Adjacency*> arcs = i->second->getOutList();

		 for (int j = 0; j < arcs.size(); j++)
		 {
			 out
				 << "    "
				 << i->second->getOrder()
				 << "->"
				 << arcs[j]->getTarget()->getOrder()
				 << "[label=\"{"
				 << arcs[j]->getComm()->getCommStrategy()->getModuleName()
				 << ";\\n"
				 << arcs[j]->getComm()->getCommProtocol()->getModuleName()
				 << "}\"];" << std::endl;
		 }
	}

	//print footer
	out
		<< "}"<< std::endl;

	out.close();

	return GTI_SUCCESS;
}

//=============================
// processLayout
//=============================
GTI_RETURN Layout::processLayout (void)
{
	VERBOSE(1) << "Processing Layout ..." << std::endl;

	int numReachable = -1;

	//==1) Validate that the level graph is acyclic
	if (!myRoot->isAcyclic (&numReachable))
	{
		std::cerr << "|-->Error: the level graph has cycles, remove these cyclic communications." << std::endl;
		return GTI_ERROR;
	}

	//==2) Validate that all levels are reachable
	if (numReachable != myLevels.size())
	{
		std::cerr << "|-->Error: not all levels in the level graph are reachable." << std::endl;
		return GTI_ERROR;
	}

	//==3) Create a tree overlay (if the communication network is a DAG but not a tree)
	std::map<int, Level*>::iterator iter;
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;
		if (l->getInList().size() > 1)
		{
			l->reduceToOneInArc ();
			std::cerr << "|-->Warning: the level layout is over-specified, the weaver will drop communication connections at random." << std::endl;
		}
	}

	//==3a) Calculate the format for the channel id
	if (calculateChannelIdFormat () != GTI_SUCCESS)
		return GTI_ERROR;

	//==3b) Place reductions
	if (myRoot->calculateReductionPlacement () != GTI_SUCCESS)
		return GTI_ERROR;

	//==3c) Check placement of any modules creating or using wrap-across calls
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
	    Level* l = iter->second;
	    if (l->checkWrapAcrossUsage () != GTI_SUCCESS)
	        return GTI_ERROR;
	}

	//==4) For all levels: Compute which arguments are used on the level
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;
		l->calculateUsedArgs ();
	}

	//==5) Start propagation of the used args to compute the args to recieve on each level
	myRoot->calculateArgsToReceive ();

	//==5a) Reverse propagation of the used args for wrapp-down calls
	myRoot->calculateArgsToReceiveReverse ();

	//==5c) Add arguments and operations for channel ID
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;
		if (l->addChannelIdData () != GTI_SUCCESS)
			return GTI_ERROR;
	}

	//==6) Determine unique Ids
	myRoot->calculateUIds ();
	myRoot->calculateUIdsReverse (); //For wrapp-down calls

	//==6b) Determine the wrap-across calls for which we need wrappers
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
	    Level* l = iter->second;
	    if (l->markWrapAcrossCallsToWrap () != GTI_SUCCESS)
	        return GTI_ERROR;
	}

	//==7) Determine required operations for each level and call
	//     (Note that calls that are wrapped on ALL levels may require
	//      operations on non application levels, thus this needs to run
	//      for all levels)
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;
		l->calculateNeededOperations ();
	}

	/**
	 * @todo I would like to have this as (5b) (as the comment below still highlights);
	 *            but it appeares we need information on the operations we will have here too;
	 *            From my current evaluation and our current usage we should be fine with
	 *            how we use it at this spot; But use cases could introduce additional operations
	 *            and forwardings due to automagic modules, so we might have to redo
	 *            some of the above calculations in fact. So this should be monitores in the future.
	 *            (Note that automagic mapping is marked as an explorative feature that might
	 *             change in the future)
	 */
	//==5b) For all levels: Automagically add modules for which all inputs are present
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
	    Level* l = iter->second;
	    l->addAutomagicModules();
	}

	//==VERBOSE) extra print of the level properties
	if (Verbose::getInstance()->getLevel() > 2)
	{
		for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
		{
			Level* l = iter->second;
			l->printCallProperties (VERBOSE(3));
		}
	}

	VERBOSE(1) << "-->SUCCESS" << std::endl;

	return GTI_SUCCESS;
}

//=============================
// generateWrapGenInput
//=============================
GTI_RETURN Layout::generateWrapGenInput (
		std::string baseName,
		std::string outputBaseName,
		std::list<std::pair<std::string, std::string> > *outXmlNames)
{
	std::map<int, Level*>::iterator iter;
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;
		char temp[256];

		sprintf(temp, "%s%d", baseName.c_str(), l->getOrder());
		std::string inputBase = temp;

		sprintf(temp, "%s%d", outputBaseName.c_str(), l->getOrder());
		std::string outputBase = temp;

		l->generateWrappGenInput (inputBase + ".xml", outputBase + ".cpp", outputBase + ".h", outputBase + ".xml");

		if (outXmlNames)
			outXmlNames->push_back(std::make_pair(inputBase + ".xml", outputBase + ".xml"));
	}

	return GTI_SUCCESS;
}

//=============================
// generateReceivalInput
//=============================
GTI_RETURN Layout::generateReceivalInput (
		std::string baseName,
		std::string outputBaseName,
		std::list<std::pair<std::string, std::string> > *outXmlNames)
{
	std::map<int, Level*>::iterator iter;
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;
		char temp[256];

		sprintf(temp, "%s%d", baseName.c_str(), l->getOrder());
		std::string inputBase = temp;

		sprintf(temp, "%s%d", outputBaseName.c_str(), l->getOrder());
		std::string outputBase = temp;

		l->generateReceivalInput (inputBase + ".xml", outputBase + ".cpp", outputBase + ".h", outputBase + ".xml");

		if (outXmlNames)
			outXmlNames->push_back(std::make_pair(inputBase + ".xml", outputBase + ".xml"));
	}

	return GTI_SUCCESS;
}

//=============================
// generateModuleConfigurationInput
//=============================
GTI_RETURN Layout::generateModuleConfigurationInput (
		std::string outFileName,
		std::string genOutputDir,
		std::string genBaseOutputFileName)
{
	//== write begining of XML
	std::ofstream out (outFileName.c_str());

	out
		<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		<< "<module-configuration>" << std::endl
		<< "\t<settings>" << std::endl
		<< "\t\t<output-dir>" << genOutputDir << "</output-dir>" << std::endl
		<< "\t\t<config-file-base-name>" << genBaseOutputFileName << "</config-file-base-name>" << std::endl
		<< "\t</settings>" << std::endl
		<< "\t<levels>" << std::endl;

	out.close();

	//== let the levels add to the XML
	std::map<int, Level*>::iterator iter;
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level* l = iter->second;

		l->generateModuleConfigurationInput(outFileName);
	}

	//== write footer to XML
	out.open (outFileName.c_str(), std::ios_base::out | std::ios_base::app);

	out
		<< "\t</levels>" << std::endl
		<< "</module-configuration>" << std::endl;

	out.close();

	return GTI_SUCCESS;
}

//=============================
// calculateChannelIdFormat
//=============================
GTI_RETURN Layout::calculateChannelIdFormat (void)
{
	int maxNumChannels = 0;
	int maxNumBits;

	//Determine maximum number of channels
	std::map<int, Level*>::iterator lIter;
	for (lIter = myLevels.begin(); lIter != myLevels.end(); lIter++)
	{
		Level* l = lIter->second;
		std::vector<Adjacency*> in = l->getInList();

		for (int i = 0; i < in.size(); i++)
		{
			Level* from = in[i]->getTarget();

			int numChannels = 0;
                        
			if (in[i]->getDistribution() == DISTRIBUTION_UNIFORM)
			{
                                if( l->getSize() )
                                    numChannels = (from->getSize()/l->getSize());
                                if (numChannels * l->getSize() != from->getSize())
                                    numChannels++;
			}
			else if (in[i]->getDistribution() == DISTRIBUTION_BY_BLOCK)
			{
			    numChannels = in[i]->getBlocksize();
			}

			if (numChannels > maxNumChannels)
				maxNumChannels = numChannels;
		}
	}

	//Reserve "0" as "from all channels" -> so we need one extra value
	maxNumChannels++;

	//Determine bits per one value in sub id (Each sub-id will need two values with this many bits!)
	for (maxNumBits = 0; maxNumChannels > 0; maxNumBits++)
		maxNumChannels = maxNumChannels >> 1;

	//Store Results
	myChannelIdBitsPerSubId = maxNumBits;
	myChannelIdNum64s = (maxNumBits * 2 * myLevels.size())/64;
	if (myChannelIdNum64s*64 < maxNumBits * 2 * myLevels.size())
		myChannelIdNum64s++;

	//Add one 64bit value For stride representations
	myChannelIdNum64s++;

	//Verbose output
	VERBOSE(1) << "| -> Note: Channel ID consists of " << myChannelIdNum64s << " 64 bit values, each sub id uses " << myChannelIdBitsPerSubId << " bits." << std::endl;

	return GTI_SUCCESS;
}

//=============================
// calculateChannelIdFormat
//=============================
void Layout::getChannelIdInfo (int *pOutNum64s, int *pOutNumBitsPerSubId)
{
	if (pOutNum64s)
		*pOutNum64s = myChannelIdNum64s;

	if (pOutNumBitsPerSubId)
		*pOutNumBitsPerSubId = myChannelIdBitsPerSubId;
}

//=============================
// getNumLevels
//=============================
int Layout::getNumLevels (void)
{
	return myLevels.size();
}

//=============================
// printInfo
//=============================
GTI_RETURN Layout::printInfo (std::string fileName)
{
	std::ofstream out (fileName.c_str());

	out
		<< "<layout-info num-layers=\"" << myLevels.size() << "\">" << std::endl;

	std::map<int, Level*>::iterator iter;
	for (iter = myLevels.begin(); iter != myLevels.end(); iter++)
	{
		Level *l = iter->second;
		int i = iter->first;

		out << "<layer id=\"" << i << "\" size=\"" << l->getSize();
		if (l->getPlace()) 
		    out << "\" place-name=\"" << l->getPlace()->getName();
                out << "\">";

		std::vector<Adjacency*> outgoing = l->getOutList();

		for (int i = 0; i < outgoing.size(); i++)
		{
			out << "<to>" << outgoing[i]->target->getOrder() << "</to>";
		}

		out << "</layer>" << std::endl;
	}

	out << "</layout-info>" << std::endl;

	return GTI_SUCCESS;
}

//=============================
// mapGtiImplicits
//=============================
GTI_RETURN Layout::mapGtiImplicits (void)
{
    //1) Add the shutdown handler and panic handler modules to all sinks in the level graph
    //    and add the PanicReceiver module to all layers
    //    and if we profile, add the MPI profiler module
    //    and, add the flood control to all non-application layers
    std::map<int, Level*>::iterator levelIter;

    for (levelIter = myLevels.begin(); levelIter != myLevels.end(); levelIter++)
    {
        Level* cur = levelIter->second;

        if (cur->getOrder() != 0)
        {
            //All non-application layers
            cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("PanicReceiver", "GTI_IMPLICIT"));
            cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("FloodControl", "GTI_IMPLICIT"));

            //Is it a sink?
            if (cur->getOutList().size() == 0)
            {
                cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("ShutdownHandler", "GTI_IMPLICIT"));
                cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("PanicHandler", "GTI_IMPLICIT"));
                cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("BreakManager", "GTI_IMPLICIT"));
            }
        }
        else
        {
            //Application layer
            cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("BreakEnforcer", "GTI_IMPLICIT"));
        }

        //All layers: profiler (if wanted)
        if (myProfiling)
            cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("ProfilerMpi", "GTI_IMPLICIT"));
    }

    //2) Add the shutdown receiver to the direct descendants of the application layer
    std::vector<Adjacency*> outs = myRoot->getOutList();

    for (int i = 0; i < outs.size(); i++)
    {
        Level* cur = outs[i]->getTarget();
        cur->addAnalysisModule(Analyses::getInstance()->findAnalysisModule("ShutdownReceiver", "GTI_IMPLICIT"));
    }

    //3) Map the shutdown handler to ALL finalizer calls
    std::list<Call*> fins = ApiCalls::getInstance()->getFinalizerCalls();
    std::list<Call*>::iterator finIter;

    Analysis *analysis = Analyses::getInstance()->findAnalysis("ShutdownHandler:notifyShutdown","GTI_IMPLICIT");

    for (finIter = fins.begin(); finIter != fins.end(); finIter++)
    {
        Call *c = *finIter;

        Mapping *mapping = new Mapping (c, ORDER_PRE, 1, 1);
        analysis->addCallMapping(mapping);
    }

    return GTI_SUCCESS;
}

//=============================
// printLayoutStatistics
//=============================
GTI_RETURN Layout::printLayoutStatistics (void)
{
    std::cout
        << "----------------------------------" << std::endl
        << "WEAVER STATISTICS BEGIN" << std::endl
        << "----------------------------------" << std::endl;
	
    std::set<Analysis*> allAns;
    std::set<AnalysisModule*> allMods;
	
    int level=0;
    std::map<int, Level*>::iterator levelIter;
    for (levelIter = myLevels.begin(); levelIter != myLevels.end(); levelIter++, level++)
    {
        Level* cur = levelIter->second;
		
        //== Analyses
        std::list<Analysis*> ans = cur->getAnalyses();
		
        //Add to all ans
        std::list<Analysis*>::iterator anIter;
        for (anIter = ans.begin(); anIter != ans.end(); anIter++)
        {
            allAns.insert(*anIter);
        }
		
        //== Modules
        std::list<AnalysisModule*> mods = cur->getAnalysisModules();
		
        //Add to all ans
        std::list<AnalysisModule*>::iterator modIter;
        for (modIter = mods.begin(); modIter != mods.end(); modIter++)
        {
            allMods.insert(*modIter);
        }
		
        //== Level Specific statistics
        if (level == 0)
        {
            std::cout
				<< "application modules: " << mods.size() << std::endl
				<< "application analyses: " << ans.size() << std::endl;
        }
        if (cur->getOutList().size() == 0)
        {
            std::cout
				<< "root modules: " << mods.size() << std::endl
				<< "root analyses: " << ans.size() << std::endl;
        }
    }
	
    std::cout
		<< "total distinct modules: " << allMods.size() << std::endl
		<< "total distinct analyses: " << allAns.size() << std::endl;
	
    int aggregationAnalyses = 0;
    std::set<Analysis*>::iterator anIter;
    for (anIter = allAns.begin(); anIter != allAns.end(); anIter++)
    {
        Analysis* a = *anIter;
        if (a->getModule()->isReduction())
            aggregationAnalyses++;
    }
	
    std::cout
		<< "total distinct aggregation analyses: " << aggregationAnalyses << std::endl;
	
    std::cout
		<< "----------------------------------" << std::endl
		<< "WEAVER STATISTICS END" << std::endl
		<< "----------------------------------" << std::endl;
	
    return GTI_SUCCESS;
}

/*EOF*/
