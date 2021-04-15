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
 * @file ModConfGen.cpp
 *		@see gti::codegen::ModConfGen
 *
 * @author Tobias Hilbrich
 * @date 02.11.2010
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <assert.h>

#include "Verbose.h"

#include "ModConfGen.h"

using namespace gti::codegen;

/**
 * Module configuration generator (modconfgen) main function.
 */
int main (int argc, char** argv)
{
	//=========== 0 read input or print help ===========
	//print help if requested
	if ((argc == 2) &&
		(
				(strncmp(argv[1], "--help", strlen("--help")) == 0 ) ||
				(strncmp(argv[1], "-help", strlen("--help")) == 0 ) ||
				(strncmp(argv[1], "-h", strlen("--help")) == 0 )
		)
	)
	{
		printUsage (argv[0], std::cout);
		return 0;
	}

	//enough arguments ?
	if (argc < 2)
	{
		std::cerr << "Error: Not enough arguments!" << std::endl << std::endl;
		printUsage (argv[0], std::cerr);
		return 1;
	}

	std::string fileName = argv[1];
	int retVal = 0;
	std::string mergeFile = "";
	if (argc > 2)
		mergeFile = argv[2];

	ModConfGen generator (fileName, mergeFile, &retVal);

	return retVal;
}

//=============================
// printUsage
//=============================
void printUsage (std::string execName, std::ostream &out)
{
	out
		<< "Usage: "
		<< execName
		<< " <ModuleConfigurationInputXML> [<MergeFile>]" << std::endl
		<< std::endl
		<< "E.g.: " << execName << " buildGenIn.xml"
		<< std::endl;
}

//=============================
// ModConfGen
//=============================
ModConfGen::ModConfGen (
		std::string inputFile,
		std::string mergeFile,
		int* retVal)
{
	if (retVal)
		*retVal = 1;

	VERBOSE(1) << "Processing input file " << inputFile << " ..." << std::endl;

	//Open the input XML
	//=================================
	SpecificationNode currentPointer;
	SpecificationNode child, subchild;
	xmlDocPtr document;

	document = xmlParseFile(inputFile.c_str());

	if (document == NULL )
	{
		std::cerr << "Error loading input XML (" << inputFile << ")" << "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
		return;
	}

	currentPointer = xmlDocGetRootElement(document);

	if (currentPointer == NULL ||
		(xmlStrcmp(currentPointer()->name, (const xmlChar *) "module-configuration") != 0))
	{
		std::cerr
			<< "Error: Document does not contains the root node (\"module-configuration\")"
			<< "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
		if (currentPointer)
			std::cerr << "Found \"" << currentPointer()->name << "\" instead!" << std::endl;
		xmlFreeDoc(document);
		return;
	}

	//Read: settings
	//=================================
	child = currentPointer.findChildNodeNamedOrErr(
			"settings",
			"|--> Error: root node has no \"settings\" child.");
	if (!child) return;

	subchild = child.findChildNodeNamedOrErr(
			"output-dir",
			"|--> Error: the settings node has no \"output-dir\" child.");
	if (!subchild) return;

	myOutputDir = subchild.getNodeContent();

	subchild = child.findChildNodeNamedOrErr(
				"config-file-base-name",
				"|--> Error: the settings node has no \"config-file-base-name\" child.");
	if (!subchild) return;

	myBaseFileName = subchild.getNodeContent();

	//Process module conf input
	//=================================
	child = currentPointer.findChildNodeNamedOrErr(
			"levels",
			"|--> Error: root node has no \"levels\" child.");
	if (!child) return;

	if (!readModConf (child))
		return;

	xmlFreeDoc(document);

	//If Verbose: print as dot
	//=================================
	if (Verbose::getInstance()->getLevel() >= 2)
	{
		std::list<std::list<Level*> >::iterator iter;
		std::string fName ("verbose-module-conf.dot");
		std::ofstream dotOut (fName.c_str());
		int index = 2;
		std::string deps;

		dotOut
			<< "digraph ModuleConfiguration" << std::endl
			<< "{" << std::endl
			<< "subgraph cluster1" << std::endl
			<< "{" << std::endl
			<< "\tlegende_analysis [label=\"Analysis\", shape=Mrecord, fillcolor=orange, style=filled];" << std::endl
			<< "\tlegende_wrapper [label=\"Wrapper\", shape=Mrecord, fillcolor=orchid, style=filled];" << std::endl
			<< "\tlegende_receival [label=\"Receival\", shape=Mrecord, fillcolor=palegoldenrod, style=filled];" << std::endl
			<< "\tlegende_place [label=\"Place\", shape=Mrecord, fillcolor=olivedrab2, style=filled];" << std::endl
			<< "\tlegende_protocol [label=\"Protocol\", shape=Mrecord, fillcolor=lightslateblue, style=filled];" << std::endl
			<< "\tlegende_strategy [label=\"Strategy\", shape=Mrecord, fillcolor=mediumpurple, style=filled];" << std::endl
			<< std::endl
			<< "\tlegende_analysis->legende_wrapper[style=invis];" << std::endl
			<< "\tlegende_wrapper->legende_receival[style=invis];" << std::endl
			<< "\tlegende_receival->legende_place[style=invis];" << std::endl
			<< "\tlegende_place->legende_protocol[style=invis];" << std::endl
			<< "\tlegende_protocol->legende_strategy[style=invis];" << std::endl
			<< std::endl
			<< "color=grey74;" << std::endl
			<< "style=filled;" << std::endl
			<< "label=\"Legend\";" << std::endl
			<< "}" << std::endl;

		for (iter = myMergedLevels.begin(); iter != myMergedLevels.end(); iter++)
		{
			std::list<Level*>::iterator levelIter;

			for (levelIter = iter->begin(); levelIter != iter->end(); levelIter++)
			{
				(*levelIter)->printAsDot (dotOut, &index, &deps);
			}
		}

		dotOut
			<< deps
			<< "}" << std::endl;

		std::cout << "|-->Info: printed module configuration DOT graph as \"" << fName << "\"." << std::endl;
	}

	//Open & Process merge file
	//=================================
	if (mergeFile != "")
	{
		document = xmlParseFile(mergeFile.c_str());

		if (document == NULL )
		{
			std::cerr << "Error loading input XML (" << mergeFile << ")" << "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
			return;
		}

		currentPointer = xmlDocGetRootElement(document);

		if (currentPointer == NULL ||
				(xmlStrcmp(currentPointer()->name, (const xmlChar *) "level-merge") != 0))
		{
			std::cerr
				<< "Error: Document does not contains the root node (\"level-merge\")"
				<< "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
			if (currentPointer)
				std::cerr << "Found \"" << currentPointer()->name << "\" instead!" << std::endl;
			xmlFreeDoc(document);
			return;
		}

		if (!readAndApplyMerge (currentPointer))
			return;

		xmlFreeDoc(document);
	}

	//Generate configuration files
	//=================================
	if (!generateConfigurations ())
		return;

	//All successful
	VERBOSE(1) << "--> SUCCESS" << std::endl;
	if (retVal)
		*retVal = 0;
}

//=============================
// ~ModConfGen
//=============================
ModConfGen::~ModConfGen (void)
{
	std::list<std::list<Level*> >::iterator iter;
	std::list<Level*>::iterator levelIter;

	for (iter = myMergedLevels.begin(); iter != myMergedLevels.end(); iter++)
	{
		for (levelIter = iter->begin(); levelIter != iter->end(); levelIter++)
		{
			Level *l = *levelIter;

			if (l)
				delete l;
		}
	}

	myMergedLevels.clear();
}

//=============================
// readModConf
//=============================
bool ModConfGen::readModConf (SpecificationNode node)
{
	SpecificationNode child, subchild, subsubchild;

	child = node.findChildNodeNamed("level");

	while (child)
	{
		std::string strIndex, strSize;
		int index, size;
		Level *l;

		//==Attribute: index
		if (!child.getAttributeOrErr("index", "|-->Error: a level node has no \"index\" attribute.", &strIndex))
			return false;
		index = atoi (strIndex.c_str());

		//==Attribute: size
		if (!child.getAttributeOrErr("size", "|-->Error: a level node has no \"index\" attribute.", &strSize))
			return false;
		size = atoi (strSize.c_str());

		//==Create level
		l = new Level (size, index);

		//==Child: modules and module sub childs
		subchild = child.findChildNodeNamedOrErr("modules", "|-->Error: a level node has no \"modules\" child.");
		if (!subchild) return false;

		subsubchild = subchild.findChildNodeNamed("module");

		while (subsubchild)
		{
			if (!readModule(subsubchild, l))
				return false;

			//next
			subsubchild = subsubchild.findSiblingNamed("module");
		}

		//==Child: relationships and relationship sub childs
		subchild = child.findChildNodeNamedOrErr("relationships", "|-->Error: a level node has no \"relationships\" child.");
		if (!subchild) return false;

		subsubchild = subchild.findChildNodeNamed("relationship");

		while (subsubchild)
		{
			if (!readRelationship(subsubchild, l))
				return false;

			//next
			subsubchild = subsubchild.findSiblingNamed("relationship");
		}

		//==Store level
		std::list<Level*> listLevel = std::list<Level*> ();
		listLevel.push_back (l);
		myMergedLevels.push_back (listLevel);

		//next
		child = child.findSiblingNamed("level");
	}

	return true;
}

//=============================
// readModule
//=============================
bool ModConfGen::readModule (SpecificationNode node, Level* l)
{
	SpecificationNode child, subchild;
	std::string strType, name, libName, strToLevel;
	bool hasToLevel = false;
	int toLevel = 0;
	MODULE_TYPE type = MODULE_UNKNOWN;
	bool isDown = false;
	bool isIntra = false;

	//==Attribute: type
	if (!node.getAttributeOrErr("type", "|-->Error: a module node has no \"type\" attribute.", &strType))
		return false;

	if (strType == "analysis")
	{
		type = MODULE_ANALYSIS;
	}
	else if (strType == "wrapper")
	{
		type = MODULE_WRAPPER;
	}
	else if (strType == "receival")
	{
		type = MODULE_RECEIVAL;
	}
	else if (strType == "place")
	{
		type = MODULE_PLACE;
	}
	else if (strType == "protocol-up")
	{
		type = MODULE_PROTOCOL;
		isDown = false;
	}
	else if (strType == "protocol-down")
	{
		type = MODULE_PROTOCOL;
		isDown = true;
	}
	else if (strType == "protocol-intra")
	{
	    type = MODULE_PROTOCOL;
	    isIntra = true;
	}
	else if (strType == "strategy")
	{
		type = MODULE_STRATEGY;
	}

	if (type == MODULE_UNKNOWN)
	{
		std::cerr << "|-->Error: unknown module type was specified: " << strType << " see module-config.dtd for valid types." << std::endl;
		return false;
	}

	//==Attribute: name
	if (!node.getAttributeOrErr("name", "|-->Error: a module node has no \"name\" attribute.", &name))
		return false;

	//==Attribute: pnmpi-module-name
	if (!node.getAttributeOrErr("pnmpi-module-name", "|-->Error: a module node has no \"pnmpi-module-name\" attribute.", &libName))
		return false;

	//==Attribute: to-level
	if (node.getAttribute("to-level", &strToLevel))
	{
		hasToLevel = true;
		toLevel = atoi (strToLevel.c_str());
	}

	//==Create/Query the module
	Module *m = l->getModuleNamed(libName);

	if (!m)
	{
		m = new Module (libName, type);
		if (!l->addModule(m))
			return false;
	}
	else
	{
		if (m->getType() != type)
		{
			std::cerr << "Warning: the input specification lists the module with pnmpi conf name \"" << libName << "\" with two different types. Using the first type I found ..." << std::endl;
		}
	}

	//==Create the instance
	char temp[256];
	sprintf (temp, "level%d_%s", l->getOrder(), name.c_str());

	Instance *instance = new Instance (l, temp, hasToLevel, toLevel, isDown, isIntra);

	if (!m->addInstance(instance))
		return false;

	//==Child: data and "setting" sub childs
	child = node.findChildNodeNamedOrErr("data", "|-->Error: a module has no \"data\" child node.");
	if (!child) return false;

	subchild = child.findChildNodeNamed("setting");

	while (subchild)
	{
		std::string key, value;

		//==Attribute: key
		if (!subchild.getAttributeOrErr("key", "|-->Error: a setting node has no \"key\" attribute.", &key))
			return false;

		//==Content (as value)
		value = subchild.getNodeContent();

		//==Add (key, value) data pair to instance
		if (!instance->addData (key, value))
			return false;

		//next
		subchild = subchild.findSiblingNamed("setting");
	}

	return true;
}

//=============================
// readRelationship
//=============================
bool ModConfGen::readRelationship (SpecificationNode node, Level* l)
{
	std::string user,used;
	char tempUser[256], tempUsed[256];
	Module *userMod, *usedMod;
	Instance *userInstance, *usedInstance;

	//==Attribute: user
	if (!node.getAttributeOrErr("user", "|-->Error: a relationship node has no \"user\" attribute.", &user))
		return false;

	//==Node content (used)
	used = node.getNodeContent();

	sprintf (tempUser, "level%d_%s", l->getOrder(), user.c_str());
	sprintf (tempUsed, "level%d_%s", l->getOrder(), used.c_str());

	//==Add relationship
	if (!l->findUniqueInstanceName (tempUser, &userMod, &userInstance))
	{
		std::cerr << "A relationship node uses an unknown name for the using instance \"" << user << "\"." << std::endl;
		return false;
	}

	if (!l->findUniqueInstanceName (tempUsed, &usedMod, &usedInstance))
	{
		std::cerr << "A relationship node uses an unknown name for the used instance \"" << used << "\"." << std::endl;
		return false;
	}

	if (!userInstance->addUsedInstance(usedMod->getLibName(), usedInstance->getUniqueName()))
		return false;

	return true;
}

//=============================
// generateConfigurations
//=============================
bool ModConfGen::generateConfigurations (void)
{
	std::list<std::list<Level*> >::iterator iter;

	for (iter = myMergedLevels.begin(); iter != myMergedLevels.end(); iter++)
	{
		std::list<Level*> levels = *iter;
		std::list<Level*>::iterator iter2;
		std::map<int, Level*> orderMap;
		std::map<int, Level*>::iterator mapIter;

		//put levels into map
		for (iter2 = levels.begin(); iter2 != levels.end(); iter2++)
		{
			orderMap.insert (std::make_pair ((*iter2)->getOrder(), *iter2));
		}

		//create output file name
		char filePath[512];
		sprintf(filePath, "%s/%s", myOutputDir.c_str(), myBaseFileName.c_str());

		for (mapIter = orderMap.begin(); mapIter != orderMap.end(); mapIter++)
		{
			std::string temp = filePath;
			sprintf (filePath, "%s.%d", temp.c_str(), mapIter->first);
		}

		//create conf file
		std::ofstream out (filePath);

		for (mapIter = orderMap.begin(); mapIter != orderMap.end(); mapIter++)
		{
 			if (mapIter->first > 0)
 			{
				out << "stack level_" << mapIter->first << std::endl;
 			}

			Level *l = mapIter->second;

			if (!l->printToConfig(out))
				return false;

			out << std::endl;
 			if (mapIter->first == 0)
 			{
				out << "stack level_" << mapIter->first << std::endl;
 			}
		}

		out.close();
	}

	return true;
}

//=============================
// readAndApplyMerge
//=============================
bool ModConfGen::readAndApplyMerge (SpecificationNode node)
{
	SpecificationNode child, subchild;

	//==Childs: set
	child = node.findChildNodeNamed("set");

	std::map <int,bool> usedOrders;

	while (child)
	{
		//==Subchilds: order
		subchild = child.findChildNodeNamed("order");

		std::map<int,int> setOrders;

		while (subchild)
		{
			//==Content
			std::string strOrder = subchild.getNodeContent();
			int order = atoi (strOrder.c_str());

			setOrders.insert (std::make_pair (order,order));

			if (usedOrders.find(order) != usedOrders.end())
			{
				std::cerr << "|--> Error: the level order \"" << order << "\" is used multiple times in merge file." << std::endl;
				return false;
			}

			usedOrders[order] = true;

			//==next
			subchild = subchild.findSiblingNamed("order");
		}

		//==next
		child = child.findSiblingNamed("set");

		//==evaluate orders in set (if at least two levels in set)
		if (setOrders.size() < 2)
			continue;

		//find level with minimum order
		int minOrder = setOrders.begin()->first;
		std::list<std::list<Level*> >::iterator mergeList;
		if (!findIterForOrder (minOrder, &mergeList))
			return false;

		//Loop over all remaining levels in the merge set
		//Merge, add level into the new list
		//Remove the old list of the level
		std::map<int,int>::iterator orderIter;
		for (orderIter = setOrders.begin(); orderIter != setOrders.end(); orderIter++)
		{
			//first level already handled
			if (orderIter == setOrders.begin())
				continue;

			//Find level
			std::list<std::list<Level*> >::iterator iToLevel;
			if (!findIterForOrder (orderIter->first, &iToLevel))
				return false;

			Level *lToMerge = *(iToLevel->begin());

			//Merge
			std::list<Level*>::iterator iter;
			for (iter = mergeList->begin(); iter != mergeList->end(); iter++)
			{
				Level *l = *iter;

				if (!l->mergeInLevel(lToMerge))
					return false;
			}

			//Add to merge list
			mergeList->push_back (lToMerge);

			//Remove old list of level
			myMergedLevels.erase (iToLevel);

			//Refresh iterator to merged list
			if (!findIterForOrder (minOrder, &mergeList))
				return false;
		}
	}

	return true;
}

//=============================
// findIterForOrder
//=============================
bool ModConfGen::findIterForOrder (int order, std::list<std::list<Level*> >::iterator *pOutIter)
{
	std::list<std::list<Level*> >::iterator ret;

	for (ret = myMergedLevels.begin(); ret != myMergedLevels.end(); ret++)
	{
		std::list<Level*> list = *ret;
		assert (list.begin() != list.end()); //Must not happen, ther should always be one level in a list

		if ((*list.begin())->getOrder() == order)
			break;
	}

	if (ret == myMergedLevels.end())
	{
		std::cerr << "|--> Error: no level with order \"" << order << "\" is present in the module configuration input, it appeared in the merge file" << std::endl;
		return false;
	}

	if (pOutIter)
		*pOutIter = ret;

	return true;
}

//=============================
//-----------------------------
//=============================

//=============================
// Level:: Static attributes
//=============================
std::map<int, Level*> 				Level::ourLevels = std::map<int, Level*> ();
std::map<std::pair<int,int>, int> 	Level::ourCommIds = std::map<std::pair<int,int>, int> ();
int 								Level::ourNextCommId = 0;
int                                Level::ourNextIntraCommId = 1024;

//=============================
// Level
//=============================
Level::Level (int size, int order)
: mySize (size),
  myOrder (order),
  myModules (),
  myWrapper (NULL)
{
	if (ourLevels.find (myOrder) == ourLevels.end())
	{
		ourLevels.insert (std::make_pair(myOrder, this));
	}
	else
	{
		std::cerr << "ERROR: dupplicate level with order id \"" << myOrder << "\"." << std::endl;
		exit(1);
	}
}

//=============================
// ~Level
//=============================
Level::~Level (void)
{
	std::list<Module*>::iterator i;

	for (i = myModules.begin();i != myModules.end(); i++)
	{
		if (*i)
			delete (*i);
	}
	myModules.clear();
}

//=============================
// getWrapper
//=============================
Module* Level::getWrapper (void)
{
	return myWrapper;
}

//=============================
// getSize
//=============================
int Level::getSize (void)
{
	return mySize;
}

//=============================
// getOrder
//=============================
int Level::getOrder (void)
{
	return myOrder;
}

//=============================
// addModule
//=============================
bool Level::addModule (Module* module)
{
	std::list<Module*>::iterator i;

	//already added ?
	for (i = myModules.begin();i != myModules.end(); i++)
	{
		Module *m = *i;
		if (m->getLibName() == module->getLibName())
		{
			std::cerr << "Warning: tried to add a module with a library name already used by another module of this level, this is a no-op." << std::endl;
			return false;
		}
	}

	//add
	myModules.push_back (module);

	if (module->getType() == MODULE_WRAPPER)
		myWrapper = module;

	return true;
}

//=============================
// getModuleNamed
//=============================
Module* Level::getModuleNamed (std::string libName)
{
	std::list<Module*>::iterator i;
	for (i = myModules.begin();i != myModules.end(); i++)
	{
		Module *m = *i;
		if (m->getLibName() == libName)
			return m;
	}

	return NULL;
}

//=============================
// printToConfig
//=============================
bool Level::printToConfig (std::ostream& out)
{
	std::list<Module*>::iterator i;
	for (i = myModules.begin();i != myModules.end(); i++)
	{
		Module *m = *i;
		if (!m->printToConfig(out))
			return false;
	}

	return true;
}

//=============================
// mergeInLevel
//=============================
bool Level::mergeInLevel (Level* levelToMergeIn)
{
	std::list<Module*>::iterator i,j;
	for (i = levelToMergeIn->myModules.begin();i != levelToMergeIn->myModules.end(); i++)
	{
		Module * otherM = *i;
		for (j = myModules.begin(); j != myModules.end(); j++)
		{
			Module * thisM = *j;
			if (otherM->getLibName() == thisM->getLibName())
			{
				if (!thisM->mergeInModule(otherM))
					return false;
			}
		}
	}

	return true;
}

//=============================
// getLevel
//=============================
Level* Level::getLevel (int order)
{
	if (ourLevels.find(order) != ourLevels.end())
		return ourLevels[order];
	return NULL;
}

//=============================
// getCommId
//=============================
int Level::getCommId (int fromOrder, int toOrder, bool isIntra)
{
	std::pair<int, int> p (fromOrder, toOrder);

	if (ourCommIds.find (p) != ourCommIds.end())
		return ourCommIds[p];

	int id;

	if (!isIntra)
	{
	    ourNextCommId++;
	    id = ourNextCommId;

	}
	else
	{
	    ourNextIntraCommId++;
	    id = ourNextIntraCommId;
	}

	ourCommIds.insert (std::make_pair(p, id));



	return id;
}

//=============================
// findUniqueInstanceName
//=============================
bool Level::findUniqueInstanceName (
		std::string name,
		Module **pOutModule,
		Instance **pOutInstance)
{
	std::list<Module*>::iterator i;
	for (i = myModules.begin();i != myModules.end(); i++)
	{
		Module *m = *i;

		Instance *instance = m->findInstanceNamed(name);

		if (instance != NULL)
		{
			if (pOutModule)
				*pOutModule = m;
			if (pOutInstance)
				*pOutInstance = instance;
			return true;
		}
	}

	return false;
}

//=============================
// printAsDot
//=============================
bool Level::printAsDot (std::ofstream &out, int *clusterIndex, std::string *deps)
{
	std::list<Module*>::iterator i;

	out
		<< "subgraph cluster" << *clusterIndex << std::endl
		<< "{" << std::endl;
	*clusterIndex = *clusterIndex + 1;

	for (i = myModules.begin(); i != myModules.end(); i++)
	{
		Module *m = *i;

		if (!m->printAsDot (out, clusterIndex, deps))
			return false;
	}

	out
		<< "\tcolor=black;" << std::endl
		<< "\tstyle=rounded;" << std::endl
		<< "\tlabel=\"Level " << myOrder << " (p=" << mySize <<  ")\";" << std::endl
		<< "}" << std::endl;

	return true;
}

//=============================
// findProtocolName
//=============================
bool Level::findProtocolName (int toOrder, std::string *pOutName)
{
	std::list<Module*>::iterator i;

	for (i = myModules.begin(); i != myModules.end(); i++)
	{
		Module *m = *i;

		if (m->findProtocolName (toOrder, pOutName))
			return true;
	}

	return false;
}

//=============================
//-----------------------------
//=============================

//=============================
// Module
//=============================
Module::Module (std::string libName, MODULE_TYPE type)
: myLibName (libName),
  myType (type),
  myWasEmptiedAtMerge (false),
  myInstances (),
  myFirstInstance (NULL)
{
	/*Nothing to do*/
}

//=============================
// ~Module
//=============================
Module::~Module (void)
{
	std::list<Instance*>::iterator i;

	for (i = myInstances.begin(); i != myInstances.end(); i++)
	{
		if (*i)
			delete (*i);
	}
	myInstances.clear();

	myFirstInstance = NULL;
}

//=============================
// addInstance
//=============================
bool Module::addInstance (Instance* instance)
{
	std::list<Instance*>::iterator i;

	if (myWasEmptiedAtMerge)
	{
		std::cerr << "ERROR: trying to add an instance to a module that was already merged into another instance. Not adding the instance." << std::endl;
		return false;
	}

	//already have an instance with this name ?
	for (i = myInstances.begin(); i != myInstances.end(); i++)
	{
		Instance* oldInstance = *i;

		if (oldInstance->getUniqueName() == instance->getUniqueName())
		{
			std::cerr << "ERROR: tried to add an instance to a module which already has an instance with the same name \"" << instance->getUniqueName() << "\"." << std::endl;
			return false;
		}
	}

	//add
	if (myInstances.size () == 0)
		myFirstInstance = instance;

	myInstances.push_back (instance);

	return true;
}

//=============================
// printToConfig
//=============================
bool Module::printToConfig (std::ostream& out)
{
	//print module name
	out
		<< "module " << myLibName << std::endl;

	//If place print name of module to use
	if (myType == MODULE_PLACE)
	{
		out << "argument instanceToUse " << myFirstInstance->getUniqueName() << std::endl;
	}

	//Nothing else to do for emptied modules
	if (myWasEmptiedAtMerge)
		return true;

	//print rest
	out
		<< "argument moduleName " << myLibName << std::endl
		<< "argument numInstances " << myInstances.size() << std::endl;

	std::list<Instance*>::iterator i;
	int index = 0;
	for (i = myInstances.begin(); i != myInstances.end(); i++, index++)
	{
		Instance* instance = *i;

		if (!instance->printToConfig(out, index, myType))
			return false;
	}

	return true;
}

//=============================
// getLibName
//=============================
std::string Module::getLibName (void)
{
	return myLibName;
}

//=============================
// getType
//=============================
MODULE_TYPE Module::getType (void)
{
	return myType;
}

//=============================
// mergeInModule
//=============================
bool Module::mergeInModule (Module* module)
{
	if (module->getLibName() != myLibName)
	{
		std::cerr << "ERROR: trying to merge two modules of different library name! (A=\"" << myLibName << "\", B=\"" << module->getLibName() << "\")." << std::endl;
		return false;
	}

	if (module->getType() != myType)
	{
		std::cerr << "WARNING: trying to merge two modules with equal library name but different type, just keeping this modules type." << std::endl;
	}

	//loop over instances of the other module and steal them
	std::list<Instance*>::iterator i;
	for (i = module->myInstances.begin(); i != module->myInstances.end(); i++)
	{
		Instance* instance = *i;
		if (!addInstance (instance))
			return false;
	}

	//remove instances in the other module and set it as emptied
	module->myInstances.clear();
	module->myWasEmptiedAtMerge = true;

	return true;
}

//=============================
// findInstanceNamed
//=============================
Instance* Module::findInstanceNamed (std::string name)
{
	std::list<Instance*>::iterator i;

	for (i = myInstances.begin(); i != myInstances.end(); i++)
	{
		Instance* instance = *i;
		if (instance->getUniqueName() == name)
			return instance;
	}

	return NULL;
}

//=============================
// printAsDot
//=============================
bool Module::printAsDot (std::ofstream &out, int *clusterIndex, std::string *deps)
{
	std::list<Instance*>::iterator i;

	out
		<< "\tsubgraph cluster" << *clusterIndex << std::endl
		<< "\t{" <<std::endl;
	*clusterIndex = *clusterIndex + 1;

	for (i = myInstances.begin(); i != myInstances.end(); i++)
	{
		Instance *inst = *i;

		if (!inst->printAsDot (out, clusterIndex, deps))
			return false;
	}

	out
		<< "\t\tcolor=black;" << std::endl
		<< "\t\tstyle=filled;" << std::endl
		<< "\t\tshape=rounded;" << std::endl
		<< "\t\tlabel=\"" << myLibName << "\";" << std::endl
		<< "\t\tfillcolor=";

	switch (myType)
	{
	case MODULE_ANALYSIS:
		out << "orange";
		break;
	case MODULE_WRAPPER:
		out << "orchid";
		break;
	case MODULE_RECEIVAL:
		out << "palegoldenrod";
		break;
	case MODULE_PLACE:
		out << "olivedrab2";
		break;
	case MODULE_PROTOCOL:
		out << "lightslateblue";
		break;
	case MODULE_STRATEGY:
		out << "mediumpurple";
		break;
	case MODULE_UNKNOWN:
		out << "red";
		break;
	}

	out
		<< ";" << std::endl
		<< "\t}" << std::endl;

	return true;
}

//=============================
// findProtocolName
//=============================
bool Module::findProtocolName (int toOrder, std::string *pOutName)
{
	std::list<Instance*>::iterator i;

	for (i = myInstances.begin(); i != myInstances.end(); i++)
	{
		Instance *inst = *i;

		if (inst->isToLevel (toOrder))
		{
			*pOutName = inst->getUniqueName();
			return true;
		}
	}

	return false;
}

//=============================
//-----------------------------
//=============================

//=============================
// Instance
//=============================
Instance::Instance (
		Level* pLevel,
		std::string uniqueName,
		bool isToLevel,
		int toLevel,
		bool isDown,
		bool isIntra)
: myUniqueName (uniqueName),
  myIsToLevel (isToLevel),
  myToLevel (toLevel),
  myData (),
  myLevel (pLevel),
  myUsedInstances (),
  myIsDown (isDown),
  myIsIntra (isIntra)
{
	/*Nothing to do.*/
}

//=============================
// ~Instance
//=============================
Instance::~Instance (void)
{
	myData.clear();
	myUsedInstances.clear();
	myLevel = NULL;
}

//=============================
// addData
//=============================
bool Instance::addData (std::string key, std::string value)
{
	if (myData.find (key) != myData.end())
	{
		std::cerr << "ERROR: tried to add data to an instance that already had data with the same key \"" << key << "\"." << std::endl;
		return false;
	}

	myData.insert (std::make_pair(key, value));

	return true;
}

//=============================
// printToConfig
//=============================
bool Instance::printToConfig (std::ostream& out, int instanceIndex, MODULE_TYPE myType)
{
	//Instance name
	out
		<< "argument instance" << instanceIndex << " " << myUniqueName << std::endl;

	//Data
	if (!myData.empty() || myType == MODULE_PROTOCOL || myType == MODULE_PLACE)
	{
		std::map<std::string, std::string>::iterator i;
		bool forceSep = false;

		out << "argument instance" << instanceIndex << "Data ";

		if (!myIsIntra && !myIsDown && myType == MODULE_PROTOCOL)
		{
			int commId = Level::getCommId(myLevel->getOrder(), myToLevel);
			out << "comm_id=" << commId << ",side=b,tier_size=" << myLevel->getSize();
			forceSep = true;
		}
		else
		if (!myIsIntra && myType == MODULE_PROTOCOL)
		{
			int commId = Level::getCommId(myToLevel, myLevel->getOrder());
			out << "comm_id=" << commId << ",side=t,tier_size=" << myLevel->getSize();
			forceSep = true;
		}
		else
		if (myIsIntra && myType == MODULE_PROTOCOL)
		{
		    int commId = Level::getCommId(myToLevel, myLevel->getOrder(), true /*isIntra*/);
		    out << "comm_id=" << commId << ",is_intra=1,tier_size=" << myLevel->getSize();
		    forceSep = true;
		}
		else
		if (myType == MODULE_PLACE)
		{
			out << "otf_proc_id=" << myLevel->getOrder()*2000;
			forceSep = true;
		}

		for (i = myData.begin(); i != myData.end(); i++)
		{
			if (i != myData.begin() || forceSep)
				out << ",";

			out << i->first << "=" << i->second;
		}

		out << std::endl;
	}

	//Sub modules
	if (!myUsedInstances.empty())
	{
		std::list<std::pair<std::string, std::string> >::iterator i;

		out << "argument instance" << instanceIndex << "SubMods ";

		for (i = myUsedInstances.begin(); i != myUsedInstances.end(); i++)
		{
			std::string modName = i->first,
						instName = i->second;

			if (i != myUsedInstances.begin())
				out << ",";

			out << modName << ":" << instName;
		}

		out << std::endl;
	}

	//information on wrapper if this is an analysis
	if ((myType == MODULE_ANALYSIS || myType == MODULE_PROTOCOL || myType == MODULE_STRATEGY) && myLevel->getWrapper())
		out << "argument instance" << instanceIndex << "Wrapper " << myLevel->getWrapper()->getLibName() << std::endl;

	return true;
}

//=============================
// getUniqueName
//=============================
std::string Instance::getUniqueName (void)
{
	return myUniqueName;
}

//=============================
// addUsedInstance
//=============================
bool Instance::addUsedInstance (std::string moduleName, std::string instanceName)
{
	myUsedInstances.push_back (std::make_pair(moduleName, instanceName));
	return true;
}

//=============================
// printAsDot
//=============================
bool Instance::printAsDot (std::ofstream &out, int *clusterIndex, std::string *deps)
{
	out
		<< "\t\t\t" << myUniqueName << "[label=\"" << myUniqueName << "\",  shape=Mrecord];" << std::endl;

	std::list<std::pair<std::string, std::string> >::iterator iter;

	for (iter = myUsedInstances.begin (); iter != myUsedInstances.end(); iter++)
	{
		//Assumes that instance names are globally unique ...
		*deps = *deps + "\t\t\t" + myUniqueName + "->" + iter->second + ";\n";
	}

	//For protocols line for protocol connection
	if (myIsToLevel && !myIsDown)
	{
		Level *l = Level::getLevel(myToLevel);
		if (l)
		{
			std::string toName;

			if (l->findProtocolName (myLevel->getOrder(), &toName))
			{
				*deps = *deps + "\t\t\t" + myUniqueName + "->" + toName + " [style=dashed];\n";
			}
		}
	}

	return true;
}

//=============================
// isToLevel
//=============================
bool Instance::isToLevel (int order)
{
	if (!myIsToLevel)
		return false;

	if (order == myToLevel)
		return true;

	return false;
}

/*EOF*/
