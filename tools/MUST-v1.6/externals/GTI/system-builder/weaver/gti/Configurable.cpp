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
 * @file Configurable.cpp
 * 		@see gti::weaver::Configurable
 *
 * @author Tobias Hilbrich
 * @date 14.01.2010
 */

#include <assert.h>

#include "Configurable.h"

using namespace gti::weaver::modules;

//=============================
// Configurable
//=============================
Configurable::Configurable ( )
	: mySettings ()
{

}

//=============================
// Configurable
//=============================
Configurable::Configurable (std::list<SettingsDescription*> settings)
	: mySettings()
{
	std::list<SettingsDescription*>::iterator i;
	for (i = settings.begin(); i != settings.end(); i++)
	{
		mySettings.insert (std::make_pair((*i)->getName(), (SettingsDescription*)*i));
	}
}

//=============================
// Configurable
//=============================
Configurable::~Configurable ( )
{
	std::map<std::string, SettingsDescription*>::iterator i;
	for (i = mySettings.begin(); i != mySettings.end(); i++)
	{
		SettingsDescription *p = i->second;

		if (p)
			delete (p);
	}

	mySettings.clear();
}

//=============================
// addSetting
//=============================
void Configurable::addSetting ( SettingsDescription * add_object )
{
	mySettings.insert (std::make_pair(add_object->getName(),add_object));
}

//=============================
// removeSetting
//=============================
void Configurable::removeSetting ( SettingsDescription * remove_object )
{
	std::map<std::string, SettingsDescription*>::iterator i;
	i = mySettings.find (remove_object->getName());

	if (i != mySettings.end())
		mySettings.erase (i);
}

//=============================
// findSetting
//=============================
SettingsDescription* Configurable::findSetting (std::string name)
{
	std::map<std::string, SettingsDescription*>::iterator i;
	i = mySettings.find (name);

	if (i != mySettings.end())
		return i->second;

	return NULL;
}

//=============================
// print
//=============================
std::ostream& Configurable::print (std::ostream& out) const
{
	std::map <std::string, SettingsDescription*>::const_iterator i;

	out << "settings={";
	for (i = mySettings.begin(); i != mySettings.end(); i++)
	{
		if (i != mySettings.begin())
			out << ", ";

		out << "setting={";
		out << *(i->second);
		out << "}";
	}
	out << "}";

	return out;
}

//=============================
// isValidSetting
//=============================
bool Configurable::isValidSetting (std::string settingName, std::string settingValue)
{
	SettingsDescription* desc = findSetting (settingName);

	if (!desc)
	{
		std::cerr
			<< "Error: configurable object has no setting with name \""
			<< settingName
			<< "\""
			<< std::endl;
		return false;
	}

	bool ret = desc->isValidValue (settingValue);

	if (!ret)
	{
		std::cerr
			<< "Error: setting with value \""
			<< settingValue
			<< "\" is not a valid value for "
			<< *desc
			<< std::endl;
	}

	return ret;
}

/*EOF*/
