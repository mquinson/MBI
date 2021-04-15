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
 * @file Gti.cpp
 * 		@see gti::weaver::Gti
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <iostream>

#include "Gti.h"
#include "Verbose.h"

using namespace gti;
using namespace gti::weaver::modules;

Gti* Gti::myInstance = NULL;

//=============================
// Gti
//=============================
Gti::Gti ( )
	: myCommStrategies (), myCommStrategiesIntra (),
	  myCommProtocols (), myPlaces (),
	  myEnums (),
	  myLibDir (""), myIncDir ("")
{
	/*Nothing to do*/
}

//=============================
// ~Gti
//=============================
Gti::~Gti ( )
{
	std::vector<EnumList*>::iterator l;
	for (l = myEnums.begin(); l != myEnums.end(); l++)
		delete (*l);
	myEnums.clear();

	std::vector<CommStrategy*>::iterator i;
	for (i = myCommStrategies.begin(); i != myCommStrategies.end(); i++)
		delete (*i);
	myCommStrategies.clear();

	std::vector<CommStrategyIntra*>::iterator m;
	for (m = myCommStrategiesIntra.begin(); m != myCommStrategiesIntra.end(); m++)
	    delete (*m);
	myCommStrategiesIntra.clear();

	std::vector<CommProtocol*>::iterator j;
	for (j = myCommProtocols.begin(); j != myCommProtocols.end(); j++)
		delete (*j);
	myCommProtocols.clear();

	std::vector<Place*>::iterator k;
	for (k = myPlaces.begin(); k != myPlaces.end(); k++)
		delete (*k);
	myPlaces.clear();
}

//=============================
// getInstance
//=============================
Gti* Gti::getInstance (void)
{
	if (!myInstance)
		myInstance = new Gti ();
	assert (myInstance);
	return myInstance;
}

//=============================
// addCommStrategy
//=============================
void Gti::addCommStrategy ( CommStrategy * add_object )
{
	myCommStrategies.push_back(add_object);
}

//=============================
// removeCommStrategy
//=============================
void Gti::removeCommStrategy ( CommStrategy * remove_object )
{
	int i, size = myCommStrategies.size();
	for ( i = 0; i < size; ++i)
	{
		CommStrategy * item = myCommStrategies.at(i);
		if(item == remove_object)
		{
			std::vector<CommStrategy *>::iterator it = myCommStrategies.begin() + i;
			myCommStrategies.erase(it);
			return;
		}
	}
}

//=============================
// addCommStrategyIntra
//=============================
void Gti::addCommStrategyIntra ( CommStrategyIntra * add_object )
{
    myCommStrategiesIntra.push_back(add_object);
}

//=============================
// removeCommStrategyIntra
//=============================
void Gti::removeCommStrategyIntra ( CommStrategyIntra * remove_object )
{
    int i, size = myCommStrategiesIntra.size();
    for ( i = 0; i < size; ++i)
    {
        CommStrategyIntra * item = myCommStrategiesIntra.at(i);
        if(item == remove_object)
        {
            std::vector<CommStrategyIntra *>::iterator it = myCommStrategiesIntra.begin() + i;
            myCommStrategiesIntra.erase(it);
            return;
        }
    }
}

//=============================
// addCommProtocol
//=============================
void Gti::addCommProtocol ( CommProtocol * add_object )
{
	myCommProtocols.push_back(add_object);
}

//=============================
// removeCommProtocol
//=============================
void Gti::removeCommProtocol ( CommProtocol * remove_object )
{
	int i, size = myCommProtocols.size();
	for ( i = 0; i < size; ++i)
	{
		CommProtocol * item = myCommProtocols.at(i);
		if(item == remove_object)
		{
			std::vector<CommProtocol *>::iterator it = myCommProtocols.begin() + i;
			myCommProtocols.erase(it);
			return;
		}
	}
}

//=============================
// addPlace
//=============================
void Gti::addPlace ( Place * add_object )
{
	myPlaces.push_back(add_object);
}

//=============================
// removePlace
//=============================
void Gti::removePlace ( Place * remove_object )
{
	int i, size = myPlaces.size();
	for ( i = 0; i < size; ++i)
	{
		Place * item = myPlaces.at(i);
		if(item == remove_object)
		{
			std::vector<Place *>::iterator it = myPlaces.begin() + i;
			myPlaces.erase(it);
			return;
		}
	}
}


//=============================
// addEnum
//=============================
void Gti::addEnum (EnumList *addEnum)
{
	myEnums.push_back(addEnum);
}

//=============================
// removeEnum
//=============================
void Gti::removeEnum (EnumList *removeEnum)
{
	int i, size = myEnums.size();
	for ( i = 0; i < size; ++i)
	{
		EnumList * item = myEnums.at(i);
		if(item == removeEnum)
		{
			std::vector<EnumList *>::iterator it = myEnums.begin() + i;
			myEnums.erase(it);
			return;
		}
	}
}

//=============================
// findEnumForId
//=============================
EnumList* Gti::findEnumForId (int id)
{
	for (int i = 0; i < myEnums.size(); ++i)
	{
		EnumList * item = myEnums.at(i);
		if (item->getId() == id)
			return item;
	}

	return NULL;
}

//=============================
// load
//=============================
GTI_RETURN Gti::load (std::string gtiSpecificationXml )
{
	//Open the input XML
	//=================================
	VERBOSE(1) << "Loading GTI ..." << std::endl;
	xmlDocPtr document;
	SpecificationNode currentPointer;

	document = xmlParseFile(gtiSpecificationXml.c_str());

	if (document == NULL )
	{
		std::cerr << "Error loading input XML (" << gtiSpecificationXml << ")" << "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
		return GTI_ERROR;
	}

	currentPointer = xmlDocGetRootElement(document);

	if (currentPointer == NULL ||
	    (xmlStrcmp(currentPointer()->name, (const xmlChar *) "gti-specification") != 0))
	{
		std::cerr
			<< "Error: Document does not contains the root node (\"gti-specification\")"
			<< "("<<__FILE__<<":"<<__LINE__<<")" << std::endl;
		if (currentPointer)
			std::cerr << "Found \"" << currentPointer()->name << "\" instead!" << std::endl;
		xmlFreeDoc(document);
		return GTI_ERROR;
	}

	//Read basic information from the root node
	//=================================
#define NUM_GTI_ROOT_ATTRIBUTES 3
	const char attributes[NUM_GTI_ROOT_ATTRIBUTES][128]
        = {"path-to-libs", "include-path", "path-to-execs"};
	std::string* storage[NUM_GTI_ROOT_ATTRIBUTES]
        = {&myLibDir, &myIncDir, &myExecDir};

	for (int i = 0; i < NUM_GTI_ROOT_ATTRIBUTES; i++)
	{
		if (!currentPointer.getAttributeOrErr (
				attributes[i],
				(std::string)"Error: the GTI specification root node has no \"" + attributes[i] + "\" attribute.",
				storage[i]))
		{
			xmlFreeDoc(document);
			return GTI_ERROR;
		}
	}

	VERBOSE (2) << "|--> Root node attributes: ";
	for (int i = 0; i < NUM_GTI_ROOT_ATTRIBUTES; i++)
		VERBOSE (2) << attributes[i] << "=" << *(storage[i]) << "; ";
	VERBOSE (2) << std::endl;

	//Read all enums first (even though they are at the end of the XML)
	//=================================
	VERBOSE(1) << "|-> Loading Enums ... " << std::endl;
	if (readEnums (currentPointer) != GTI_SUCCESS)
	{
		xmlFreeDoc(document);
		return GTI_ERROR;
	}
	VERBOSE(1) << "|-> Loaded Enums (" << myEnums.size() << " in total)." << std::endl;

	//Loop over all CommProtocols
	//=================================
	VERBOSE(1) << "|-> Loading Comm Protocols ... " << std::endl;
	if (readCommProtocols (currentPointer) != GTI_SUCCESS)
	{
		xmlFreeDoc(document);
		return GTI_ERROR;
	}
	VERBOSE(1) << "|-> Loaded Comm Protocols (" << myCommProtocols.size() << " in total)." << std::endl;

	//Loop over all CommStrategies
	//=================================
	VERBOSE(1) << "|-> Loading Comm Strategies ... " << std::endl;
	if (readCommStrategies (currentPointer) != GTI_SUCCESS)
	{
		xmlFreeDoc(document);
		return GTI_ERROR;
	}
	VERBOSE(1) << "|-> Loaded Comm Strategies (" << myCommStrategies.size() << " inter, " << myCommStrategiesIntra.size() << " intra in total)." << std::endl;

	//Loop over all Places
	//=================================
	VERBOSE(1) << "|-> Loading Places ... " << std::endl;
	if (readPlaces (currentPointer) != GTI_SUCCESS)
	{
		xmlFreeDoc(document);
		return GTI_ERROR;
	}
	VERBOSE(1) << "|-> Loaded Places (" << myPlaces.size() << " in total)." << std::endl;

	VERBOSE(1) << "--> SUCCESS" << std::endl;

	//free document
	xmlFreeDoc(document);

	return GTI_SUCCESS;
}

//=============================
// readEnums
//=============================
GTI_RETURN Gti::readEnums (SpecificationNode root)
{
	SpecificationNode iter = root.findChildNodeNamedOrErr (
			"enums",
			"Error: GTI specification has no \"enums\" node!");

	if (!iter)
		return GTI_ERROR;

	SpecificationNode subiter = iter.findChildNodeNamed ("enum");

	while (subiter)
	{
		int id;
		std::list<std::string> entries;
		std::string idString;

		//Read attribute: id
		if (!subiter.getAttributeOrErr (
				"id",
				"Error: an enum node has no \"id\" attribute.",
				&idString))
			return GTI_ERROR;

		id = atoi (idString.c_str());

		//Read all entries
		SpecificationNode subsubiter = subiter.findChildNodeNamed ("entry");

		while (subsubiter)
		{
			entries.push_back (subsubiter.getNodeContent ());
			subsubiter = subsubiter.findSiblingNamed("entry");
		}/*Loop over all childs of enum node*/

		//Create EnumList
		this->addEnum(new EnumList (id, entries));
		VERBOSE(2) << "|--> New Enum: " << *(myEnums.back ()) << std::endl;

		subiter = subiter.findSiblingNamed ("enum");
	}//loop over all enum nodes
	return GTI_SUCCESS;
}

//=============================
// readCommProtocols
//=============================
GTI_RETURN Gti::readCommProtocols (SpecificationNode root)
{
	//find comm-protocols node
	SpecificationNode iter = root.findChildNodeNamedOrErr(
			"comm-protocols",
			"Error: GTI specification has no \"comm-protocols\" node!");

	if (!iter)
		return GTI_ERROR;

	//loop over all comm-protocol child nodes
	SpecificationNode subiter = iter.findChildNodeNamed("comm-protocol");

	while (subiter)
	{
		if (readCommProtocol (subiter) != GTI_SUCCESS)
			return GTI_ERROR;

		subiter = subiter.findSiblingNamed("comm-protocol");
	}

	return GTI_SUCCESS;
}

//=============================
// readCommProtocol
//=============================
GTI_RETURN Gti::readCommProtocol (SpecificationNode node)
{
	//Variables we will use to initialize a comm protocol
	//===================================================
	std::string moduleName;
	std::string configName;
	std::list<SettingsDescription*> settings;
	std::list<Module*> prependModules;
	std::list<std::string> apis;
	bool supportsIntraComm = false;

	//Attribute: name
	//===================================================
	if (!node.getAttributeOrErr(
			"name",
			"Error: a comm-protocol node has no \"name\" attribute.",
			&moduleName))
		return GTI_ERROR;

	//Attribute: supports-intra (optional)
	//===================================================
	std::string temp;
	if (node.getAttribute("supports-intra", &temp))
	{
	    if (temp != "no" && temp != "yes")
	    {
	        std::cerr << "Error: a comm-protocol node uses an invalid value for the supports-intra attribute: \"" << temp << "\"; Valid are \"yes\" and \"no\"" << std::endl;
	        return GTI_ERROR;
	    }

	    if (temp == "yes")
	        supportsIntraComm = true;
	}

	//Child-node: module-name
	//===================================================
	SpecificationNode iter = node.findChildNodeNamedOrErr (
			"module-name",
			"Error: A comm-protocol node has no \"module-name\" child.");

	if (!iter)
		return GTI_ERROR;

	configName = iter.getNodeContent();

	//Child-node: settings
	//===================================================
	iter = iter.findSiblingNamedOrErr(
			"settings",
			"Error: A comm-protocol node has no \"settings\" child.");
	if (!iter)
		return GTI_ERROR;

	if (readSettings (iter, &settings) != GTI_SUCCESS)
		return GTI_ERROR;

	//Child-node: prepend-modules
	//===================================================
	iter = iter.findSiblingNamedOrErr(
			"prepend-modules",
			"Error: A comm-protocol node has no \"prepend-modules\" child.");
	if (!iter)
		return GTI_ERROR;

	if (readPrependModules (iter, &prependModules) != GTI_SUCCESS)
		return GTI_ERROR;

	//Child-node: required-apis
	//===================================================
	iter = iter.findSiblingNamedOrErr(
			"required-apis",
			"Error: A comm-protocol node has no \"required-apis\" child.");
	if (!iter)
		return GTI_ERROR;

	if (readRequiredApis (iter, &apis) != GTI_SUCCESS)
		return GTI_ERROR;

	//Create the comm protocol
	//===================================================
	this->addCommProtocol(
			new CommProtocol (
					moduleName,
					configName,
					settings,
					prependModules,
					apis,
					supportsIntraComm
					));

	//Verbose output
 	VERBOSE(2) << "|--> Read CommProtocol=" << *(myCommProtocols.back()) << std::endl;

	return GTI_SUCCESS;
}

//=============================
// readCommStrategies
//=============================
GTI_RETURN Gti::readCommStrategies (SpecificationNode root)
{
	SpecificationNode iter = root.findChildNodeNamedOrErr(
			"comm-strategies",
			"Error: GTI specification root node has no \"comm-strategies\" node!");
	if (!iter)
		return GTI_ERROR;

	//We are looking for either a regular or an intra comm strategy
	std::string strategyType;
	std::list<std::string> types;
	types.push_back("comm-strategy");
	types.push_back("comm-strategy-intra");

	iter = iter.findChildNodeNamed(types, &strategyType);

	while (iter)
	{
	    bool isIntra = false;
		std::string moduleNameUp;
		std::string configNameUp;
		std::string moduleNameDown; //only for regular strategies
		std::string configNameDown; //only for regular strategies
		std::list<SettingsDescription*> settings;

		if (strategyType == "comm-strategy-intra")
		    isIntra = true;

		//==Attribute: name-up/name
		std::string searchName = "name-up";
		if (isIntra)
		    searchName="name";
		if (!iter.getAttributeOrErr(
				searchName,
				(std::string)"Error: a comm-strategy node has no\"" + searchName + "\" attribute!",
				&moduleNameUp))
			return GTI_ERROR;

		//==Attribute: name-down
		if (!isIntra)
		{
            if (!iter.getAttributeOrErr(
                    "name-down",
                    "Error: a comm-strategy node has no \"name-down\" attribute!",
                    &moduleNameDown))
                return GTI_ERROR;
		}

		//==Child: module-name-up
		searchName = "module-name-up";
		if (isIntra)
		    searchName = "module-name";

		SpecificationNode temp = iter.findChildNodeNamedOrErr(
		        searchName,
				(std::string)"Error: a comm-strategy node has no \"" + searchName + "\" child!");
		if (!temp)
			return GTI_ERROR;
		configNameUp = temp.getNodeContent();

		//==Child: module-name-down
		if (!isIntra)
		{
            temp = iter.findChildNodeNamedOrErr(
                    "module-name-down",
                    "Error: a comm-strategy node has no \"module-name-down\" child!");
            if (!temp)
                return GTI_ERROR;
            configNameDown = temp.getNodeContent();
		}

		//==Child: settings
		temp = iter.findChildNodeNamedOrErr(
				"settings",
				(std::string)"Error: a \"" + strategyType + "\" node has no \"settings\" child!");
		if (!temp)
			return GTI_ERROR;
		if (readSettings(temp, &settings) != GTI_SUCCESS)
			return GTI_ERROR;

		//==Construct The Object
		if (isIntra)
		{
		    addCommStrategyIntra(
		            new CommStrategyIntra (
		                    moduleNameUp,
		                    configNameUp,
		                    settings
		            ));
		}
		else
		{
		    addCommStrategy(
		            new CommStrategy (
		                    moduleNameUp,
		                    configNameUp,
		                    moduleNameDown,
		                    configNameDown,
		                    settings
		            ));
		}

		//Verbose Output
		if (isIntra)
		    VERBOSE (2) << "Read CommStrategyIntra={" << *(myCommStrategiesIntra.back()) << "}" << std::endl;
		else
		    VERBOSE (2) << "Read CommStrategy={" << *(myCommStrategies.back()) << "}" << std::endl;

		//next comm-strategy
		iter = iter.findSiblingNamed(types, &strategyType);
	}

	return GTI_SUCCESS;
}

//=============================
// readPlaces
//=============================
GTI_RETURN Gti::readPlaces (SpecificationNode root)
{
	SpecificationNode iter = root.findChildNodeNamedOrErr(
			"places",
			"Error: GTI specification root node has no \"places\" node!");
	if (!iter)
		return GTI_ERROR;

	iter = iter.findChildNodeNamed("place");

	while (iter)
	{
		std::string name;
		std::string configName;
		bool isModule = false;
		bool isApp = false;
		std::string executableName;
		std::list<Module*> prependModules;
		std::list<std::string> apis;
		std::list<SettingsDescription*> settings;

		//==Attribute: name
		if (!iter.getAttributeOrErr(
				"name",
				"Error: a place node has no \"name\" attribute!",
				&name))
			return GTI_ERROR;

		//==Child: prepend-modules
		SpecificationNode temp = iter.findChildNodeNamedOrErr(
				"prepend-modules",
				"Error: A place node has no \"prepend-modules\" child.");
		if (!temp)
			return GTI_ERROR;
		if (readPrependModules (temp, &prependModules) != GTI_SUCCESS)
			return GTI_ERROR;

		//==Child: instance
		temp = iter.findChildNodeNamedOrErr(
				"instance",
				"Error: A place node has no \"instance\" child.");
		if (!temp)
			return GTI_ERROR;

		if (readInstance (temp, &isModule, &isApp, &configName, &executableName) != GTI_SUCCESS)
			return GTI_ERROR;

		//==Child: required-apis
		temp = iter.findChildNodeNamedOrErr(
				"required-apis",
				"Error: A place node has no \"required-apis\" child.");
		if (!temp)
			return GTI_ERROR;

		if (readRequiredApis (temp, &apis) != GTI_SUCCESS)
			return GTI_ERROR;

		//==Child: settings
		temp = iter.findChildNodeNamedOrErr(
				"settings",
				"Error: A place node has no \"settings\" child.");
		if (!temp)
			return GTI_ERROR;

		if (readSettings (temp, &settings) != GTI_SUCCESS)
			return GTI_ERROR;

		//==Construct the object
		if (isModule)
			addPlace(new Place (
					name,
					configName,
					prependModules,
					apis,
					settings));
		else if (isApp)
			addPlace(new Place (
					name,
					apis,
					settings));
		else
			addPlace(new Place (
					executableName,
					prependModules,
					apis,
					settings));

		//Verbose output
		VERBOSE(2) << "Read Place={" << *(myPlaces.back()) << "}" << std::endl;

		//next place
		iter = iter.findSiblingNamed("place");
	}/*loop over all place nodes*/

	return GTI_SUCCESS;
}

//=============================
// readInstance
//=============================
GTI_RETURN Gti::readInstance (
			SpecificationNode node,
			bool *pIsModule,
			bool *pIsApp,
			std::string *pModuleName,
			std::string *pExecutableName)
{
	std::string typeString;

	if (!node.getAttributeOrErr(
			"type",
			"Error: an instance node has no \"type\" attribute!",
			&typeString))
		return GTI_ERROR;

	if (typeString != "executable" &&
		typeString != "module" && typeString != "app")
	{
		std::cerr << "Error: an instance node specifies an invalid type (\""<< typeString << "\")!" << std::endl;
		return GTI_ERROR;
	}

	if (typeString == "module")
		*pIsModule = true;
	else
		*pIsModule = false;

	if (typeString == "app")
	{
		*pIsApp = true;
		return GTI_SUCCESS;
	}
	else
		*pIsApp = false;

	SpecificationNode instName = NULL;
	std::string *pTarget;
	if (*pIsModule)
	{
		instName = node.findChildNodeNamedOrErr(
				"module-name",
				"Error: an instance node of the module type has no \"module-name\" child!");
		pTarget = pModuleName;
		*pExecutableName = "";
	}
	else
	{
		instName = node.findChildNodeNamedOrErr(
				"executable-name",
				"Error: an instance node of the executable type has no \"executable-name\" child!");
		pTarget = pExecutableName;
		*pModuleName = "";
	}

	if (!instName)
		return GTI_ERROR;

	*pTarget = instName.getNodeContent();

	return GTI_SUCCESS;
}

//=============================
// readSettings
//=============================
GTI_RETURN Gti::readSettings (SpecificationNode node, std::list<SettingsDescription*> *settings)
{
	//loop over all the setting sub-modules
	SpecificationNode iter = node.findChildNodeNamed("setting");

	while (iter != NULL)
	{
		SettingsDescription *setting = NULL;
		if (readSetting (iter, &setting) != GTI_SUCCESS)
			return GTI_ERROR;

		if (settings && setting)
			settings->push_back (setting);

		iter = iter.findSiblingNamed("setting");
	}

	return GTI_SUCCESS;
}

//=============================
// readSetting
//=============================
GTI_RETURN Gti::readSetting (SpecificationNode node, SettingsDescription **setting)
{
	std::string settingName;
	std::string description;
	std::string defaultValue;
	bool hasRange;
	std::string rangeMin;
	std::string rangeMax;
	SettingType type;
	FilePathSettingIntention intention;
	int enumId;
	bool selectionRequired;

	//==sanity
	assert (setting);

	//==Attribute: name
	if (!node.getAttributeOrErr(
			"name",
			"Error: a setting node has no \"name\" attribute.",
			&settingName))
		return GTI_ERROR;

	//==Child node: description
	SpecificationNode iter = node.findChildNodeNamedOrErr (
			"description",
			"Error: A setting node has no \"description\" child.");
	if (!iter)
		return GTI_ERROR;

	description = iter.getNodeContent ();

	//==Child node: value
	iter = iter.findSiblingNamedOrErr(
			"value",
			"Error: A setting node has no \"value\" child.");
	if (!iter)
		return GTI_ERROR;

	if (readValue (
		iter,
		&type,
		&defaultValue,
		&hasRange,
		&rangeMin,
		&rangeMax,
		&intention,
		&enumId,
		&selectionRequired
		) != GTI_SUCCESS)
	return GTI_ERROR;

	//==Construct setting
	switch (type)
	{
	case BOOL_SETTING:
	case STRING_SETTING:
	case PATH_SETTING:
		*setting = new SettingsDescription (
				settingName,
				description,
				defaultValue,
				type);
		break;
	case FLOAT_SETTING:
	case INTEGER_SETTING:
		if (!hasRange)
		{
			*setting = new SettingsDescription (
					settingName,
					description,
					defaultValue,
					type);
		}
		else
		{
			*setting = new SettingsDescription (
					settingName,
					description,
					defaultValue,
					rangeMin,
					rangeMax,
					type);
		}
		break;
	case ENUM_SETTING:
	case ENUM_SELECTION_SETTING:
	{
		EnumList *item = findEnumForId (enumId);
		if (!item)
		{
			std::cerr << "Error: Found a value node for an enum or enumselection with an invalid id, the given id is:" << enumId << std::endl;
			return GTI_ERROR;
		}

		if (type == ENUM_SETTING)
			*setting = new SettingsDescription (
					settingName,
					description,
					defaultValue,
					item,
					type);
		else
			*setting = new SettingsDescription (
					settingName,
					description,
					defaultValue,
					item,
					selectionRequired,
					type);
		break;
	}
	case FILE_PATH_SETTING:
		*setting = new SettingsDescription (
				settingName,
				description,
				defaultValue,
				intention,
				type);
		break;
	case UNKNOWN_SETTING:
		std::cerr << "Error: Found a setting of type UNKNOWN_SETTING" << std::endl;
		return GTI_ERROR;
	}

	return GTI_SUCCESS;
}

//=============================
// readValue
//=============================
GTI_RETURN Gti::readValue (
		    SpecificationNode node,
			SettingType *pType,
			std::string *pDefaultValue,
			bool *pHasRange,
			std::string *pRangeMin,
			std::string *pRangeMax,
			FilePathSettingIntention *pIntention,
			int *pEnumId,
			bool *pSelectionRequired
			)
{
	//== sanity & setup
	assert (pType && pDefaultValue && pHasRange && pRangeMin && pRangeMax && pIntention && pEnumId && pSelectionRequired);
	*pHasRange = false;
	*pRangeMin = "";
	*pRangeMax = "";
	*pIntention = UNKNOWN_INTENTION;
	*pEnumId = -1;
	*pSelectionRequired = false;

	//== Attribute: type
	std::string typeString;
	if (!node.getAttributeOrErr(
			"type",
			"Error: a value node has no \"type\" attribute.",
			&typeString))
		return GTI_ERROR;

	if (typeString == "bool")
	{
		*pType = BOOL_SETTING;
	}
	else if (typeString == "enum")
	{
		*pType = ENUM_SETTING;
	}
	else if (typeString == "enumselection")
	{
		*pType = ENUM_SELECTION_SETTING;
	}
	else if (typeString == "float")
	{
		*pType = FLOAT_SETTING;
	}
	else if (typeString == "integer")
	{
		*pType = INTEGER_SETTING;
	}
	else if (typeString == "filepath")
	{
		*pType = FILE_PATH_SETTING;
	}
	else if (typeString == "path")
	{
		*pType = PATH_SETTING;
	}
	else if (typeString == "string")
	{
		*pType = STRING_SETTING;
	}
	else
	{
		*pType = UNKNOWN_SETTING;
		std::cerr << "Error: a value node uses an unknown type: \"" << typeString << "\"!" << std::endl;
		return GTI_ERROR;
	}

	//== Child node: default
	SpecificationNode iter = node.findChildNodeNamedOrErr(
			"default",
			"Error: A value node has no \"default\" child.");
	if (!iter)
		return GTI_ERROR;

	*pDefaultValue = iter.getNodeContent();

	//== Optional child nodes: range, enumid, one-required, io
	//=range
	SpecificationNode rangeIter = iter.findSiblingNamed("range");
	if (rangeIter)
	{
		*pHasRange = true;

		//read min attribute
		if (!rangeIter.getAttributeOrErr(
				"min",
				"Error: a range node has no \"min\" attribute.",
				pRangeMin))
			return GTI_ERROR;

		//read max attribute
		if (!rangeIter.getAttributeOrErr(
				"max",
				"Error: a range node has no \"max\" attribute.",
				pRangeMax))
			return GTI_ERROR;
	}//has Range

	//=enumid
	SpecificationNode enumidIter = iter.findSiblingNamed("enumid");
	if (enumidIter)
	{
		*pEnumId = atoi(enumidIter.getNodeContent().c_str());
	}

	//=one-required
	SpecificationNode oneRequiredIter = iter.findSiblingNamed("one-required");
	if (oneRequiredIter)
	{
		std::string temp;

		temp = oneRequiredIter.getNodeContent();

		if (temp != "0" &&
			temp != "1")
		{
			std::cerr << "Error: The content of a one-required node is neither  \"0\" nor \"1\"; speciefied was: \"" << temp << "\"." << std::endl;
			return GTI_ERROR;
		}

		if (atoi(temp.c_str()) == 0)
			*pSelectionRequired = false;
		else
			*pSelectionRequired = true;
	}

	//=io
	SpecificationNode ioIter = iter.findSiblingNamed("io");
	if (ioIter)
	{
		std::string temp;
		temp = ioIter.getNodeContent();

		if (temp == "in")
		{
			*pIntention = IN_INTENTION;
		}
		else if (temp == "out")
		{
			*pIntention = OUT_INTENTION;
		}
		else if (temp == "in_out")
		{
			*pIntention = IN_OUT_INTENTION;
		}
		else
		{
			*pIntention = UNKNOWN_INTENTION;
			std::cerr << "Error: an io node has invalid content, allowed are \"in\", \"out\", and \"in_out\", you specified: "<< temp <<"." << std::endl;
			return GTI_ERROR;
		}
	}

	return GTI_SUCCESS;
}

//=============================
// readPrependModules
//=============================
GTI_RETURN Gti::readPrependModules (SpecificationNode node, std::list<Module*> *prependModules)
{
	//Navigate to first "prepend-module" node
	SpecificationNode iter = node.findChildNodeNamed("prepend-module");

	while (iter)
	{
		//==read the prepend-module
		std::string name;
		std::string modName;

		//=Attribute: name
		if (!iter.getAttributeOrErr(
				"name",
				"Error: a prepend-module node has no \"name\" attribute!",
				&name))
			return GTI_ERROR;

		//=Child: module-name
		SpecificationNode modNameNode = iter.findChildNodeNamedOrErr(
				"module-name",
				"Error: a prepend-module has no \"module-name\" child!");
		if (!modNameNode)
			return GTI_ERROR;

		modName = modNameNode.getNodeContent();

		//==Save the prepend Module
		prependModules->push_back(new Module (name,modName));

		//==next
		iter = iter.findSiblingNamed("prepend-module");
	}/*while a "prepend-module" node was */

	return GTI_SUCCESS;
}

//=============================
// readRequiredApis
//=============================
GTI_RETURN Gti::readRequiredApis (SpecificationNode node, std::list<std::string> *apis)
{
	SpecificationNode iter = node.findChildNodeNamed("required-api");

	while (iter)
	{
		//read
		apis->push_back(iter.getNodeContent());

		//next
		iter = iter.findSiblingNamed("required-api");
	}//while a required-api node is present

	return GTI_SUCCESS;
}

//=============================
// findPlace
//=============================
Place*  Gti::findPlace (std::string name)
{
	for (int i = 0; i < myPlaces.size(); i++)
	{
		if (myPlaces[i]->getName() == name)
			return myPlaces[i];
	}

	return NULL;
}

//=============================
// findStrategy
//=============================
CommStrategy* Gti::findStrategy (std::string name)
{
	for (int i = 0; i < myCommStrategies.size(); i++)
	{
		if (myCommStrategies[i]->getModuleName() == name)
			return myCommStrategies[i];
	}

	return NULL;
}

//=============================
// findStrategyIntra
//=============================
CommStrategyIntra* Gti::findStrategyIntra (std::string name)
{
    for (int i = 0; i < myCommStrategiesIntra.size(); i++)
    {
        if (myCommStrategiesIntra[i]->getModuleName() == name)
            return myCommStrategiesIntra[i];
    }

    return NULL;
}

//=============================
// findProtocol
//=============================
CommProtocol* Gti::findProtocol (std::string name)
{
	for (int i = 0; i < myCommProtocols.size(); i++)
	{
		if (myCommProtocols[i]->getModuleName() == name)
			return myCommProtocols[i];
	}

	return NULL;
}

/*EOF*/
