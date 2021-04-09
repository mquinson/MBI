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
 * @file Place.cpp
 * 		@see gti::weaver::Place
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Place.h"

using namespace gti::weaver::modules;


//=============================
// Place
//=============================
Place::Place (void)
	: Module (), Prepended(), RequiresApi (), Configurable ()
{
	/*Nothing to do*/
}

//=============================
// Place
//=============================
Place::Place (
			std::string name,
			std::list<std::string> apis,
			std::list<SettingsDescription*> settings)
	: Module (), Prepended (), RequiresApi(apis), Configurable (settings)
{
	myType = PLACE_TYPE_APP;
	myExecutableName = name;
}

//=============================
// Place
//=============================
Place::Place (
			std::string executableName,
			std::list<Module*> prependModules,
			std::list<std::string> apis,
			std::list<SettingsDescription*> settings)
	: Module (), Prepended (prependModules), RequiresApi(apis), Configurable (settings)
{
	myType = PLACE_TYPE_EXECUTABLE;
	myExecutableName = executableName;
}

//=============================
// Place
//=============================
Place::Place (
			std::string moduleName,
			std::string configName,
			std::list<Module*> prependModules,
			std::list<std::string> apis,
			std::list<SettingsDescription*> settings)
	: Module (moduleName, configName), Prepended (prependModules), RequiresApi(apis), Configurable (settings)
{
	myType = PLACE_TYPE_MODULE;
	myExecutableName = "";
}

//=============================
// Place
//=============================
Place::Place (
			std::string moduleName,
			std::string configName,
			std::string instanceType,
			std::string headerName,
			std::string incDir,
			std::list<Module*> prependModules,
			std::list<std::string> apis,
			std::list<SettingsDescription*> settings)
	: Module (moduleName, configName, instanceType, headerName, incDir),
	  Prepended (prependModules),
	  RequiresApi(apis),
	  Configurable (settings)
{
	myType = PLACE_TYPE_MODULE;
	myExecutableName = "";
}

//=============================
// ~Place
//=============================
Place::~Place ( )
{
	/*Nothing to do*/
}

//=============================
// isModule
//=============================
bool Place::isModule ()
{
	if (myType == PLACE_TYPE_MODULE)
		return true;
	return false;
}

//=============================
// setExecutableName
//=============================
void Place::setExecutableName (std::string name)
{
	myExecutableName = name;
}

//=============================
// getExecutableName
//=============================
std::string Place::getExecutableName (void)
{
//	if (myType == PLACE_TYPE_EXECUTABLE)
		return myExecutableName;
//	return "";
}

//=============================
// setType
//=============================
void Place::setType (PlaceType type)
{
	myType = type;
}

//=============================
// print
//=============================
std::ostream& Place::print (std::ostream& out) const
{
	if (myType == PLACE_TYPE_MODULE)
	{
		out << "type=module, ";
		Module::print (out);
		out << ", ";
		Prepended::print(out);
		out << ", ";
		RequiresApi::print (out);
		out << ", ";
		Configurable::print (out);
	}
	else if (myType == PLACE_TYPE_EXECUTABLE)
	{
		out
			<< "type=executable, "
			<< "executableName=" << myExecutableName;
		Prepended::print(out);
		out << ", ";
		RequiresApi::print (out);
		out << ", ";
		Configurable::print (out);
	}
	else
	{
		out << "type=app, ";
		Prepended::print(out);
		out << ", ";
		RequiresApi::print (out);
		out << ", ";
		Configurable::print (out);
	}

	return out;
}

//=============================
// getName
//=============================
std::string Place::getName (void)
{
	if (isModule())
		return getModuleName ();

	return getExecutableName();
}

/*EOF*/
