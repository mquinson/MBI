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
 * @file SettingsDescription.cpp
 * 		@see gti::weaver::SettingsDescription
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "SettingsDescription.h"

using namespace gti::weaver::modules;

//=============================
// SettingsDescription
//=============================
SettingsDescription::SettingsDescription (void)
 : myName (""),
   myDescription (""),
   myDefault (""),
   myValueType (UNKNOWN_SETTING),
   myFilePathIntention (UNKNOWN_INTENTION),
   myHasRange (false),
   myRangeMin (""),
   myRangeMax (""),
   myEnumSelectionOneRequired (false),
   myEnum (NULL)
{
	/*Nothing to do*/
}

//=============================
// SettingsDescription
//=============================
			/**
			 * Constructor for the following values:
			 *  - bool
			 *  - float (without range)
			 *  - integer (without range)
			 *  - path
			 *  - string
			 * Will fail for other value types !
			 */
SettingsDescription::SettingsDescription (
					std::string name,
					std::string description,
					std::string defaultValue,
					SettingType myValueType)
 : myName (name),
   myDescription (description),
   myDefault (defaultValue),
   myValueType (myValueType),
   myFilePathIntention (UNKNOWN_INTENTION),
   myHasRange (false),
   myRangeMin (""),
   myRangeMax (""),
   myEnumSelectionOneRequired (false),
   myEnum (NULL)
{
	assert (myValueType == BOOL_SETTING ||
			myValueType == FLOAT_SETTING ||
			myValueType == INTEGER_SETTING ||
			myValueType == PATH_SETTING ||
			myValueType == STRING_SETTING);
}

//=============================
// SettingsDescription
//=============================
			/**
			 * Constructor for the following values:
			 *  - enum
			 *  - enumselection (without at least one selection required)
			 * Will fail for other value types !
			 */
SettingsDescription::SettingsDescription (
					std::string name,
					std::string description,
					std::string defaultValue,
					EnumList    *enumEntries,
					SettingType myValueType)
: myName (name),
  myDescription (description),
  myDefault (defaultValue),
  myValueType (myValueType),
  myFilePathIntention (UNKNOWN_INTENTION),
  myHasRange (false),
  myRangeMin (""),
  myRangeMax (""),
  myEnumSelectionOneRequired (false),
  myEnum (enumEntries)
{
	assert (myValueType == ENUM_SETTING ||
			myValueType == ENUM_SELECTION_SETTING);
	assert (myEnum);
}

//=============================
// SettingsDescription
//=============================
SettingsDescription::SettingsDescription (
					std::string name,
					std::string description,
					std::string defaultValue,
					EnumList    *enumEntries,
					bool        selectionRequired,
					SettingType myValueType)
: myName (name),
  myDescription (description),
  myDefault (defaultValue),
  myValueType (myValueType),
  myFilePathIntention (UNKNOWN_INTENTION),
  myHasRange (false),
  myRangeMin (""),
  myRangeMax (""),
  myEnumSelectionOneRequired (selectionRequired),
  myEnum (enumEntries)
{
	assert (myValueType == ENUM_SELECTION_SETTING);
	assert (myEnum);
}

//=============================
// SettingsDescription
//=============================
SettingsDescription::SettingsDescription (
					std::string name,
					std::string description,
					std::string defaultValue,
					FilePathSettingIntention myFilePathIntention,
					SettingType myValueType)
 : myName (name),
   myDescription (description),
   myDefault (defaultValue),
   myValueType (myValueType),
   myFilePathIntention (myFilePathIntention),
   myHasRange (false),
   myRangeMin (""),
   myRangeMax (""),
   myEnumSelectionOneRequired (false),
   myEnum (NULL)
{
	assert (myValueType == FILE_PATH_SETTING);
}

//=============================
// SettingsDescription
//=============================
			/**
			 * Constructor for the following values:
			 *  - float (with range)
			 *  - integer (with range)
			 * Will fail for other value types !
			 */
SettingsDescription::SettingsDescription (
					std::string name,
					std::string description,
					std::string defaultValue,
					std::string rangeMin,
					std::string rangeMax,
					SettingType myValueType)
 : myName (name),
   myDescription (description),
   myDefault (defaultValue),
   myValueType (myValueType),
   myFilePathIntention (UNKNOWN_INTENTION),
   myHasRange (true),
   myRangeMin (rangeMin),
   myRangeMax (rangeMax),
   myEnumSelectionOneRequired (false),
   myEnum (NULL)
{
	assert (myValueType == FLOAT_SETTING  ||
			myValueType == INTEGER_SETTING);
}

//=============================
// ~SettingsDescription
//=============================
SettingsDescription::~SettingsDescription (void)
{
	/*Nothing to do*/;
	myEnum = NULL;
	myValueType = UNKNOWN_SETTING;
}

//=============================
// getName
//=============================
std::string SettingsDescription::getName (void)
{
	return myName;
}

//=============================
// setName
//=============================
void SettingsDescription::setName (std::string name)
{
	myName = name;
}

//=============================
// getDefaultValue
//=============================
std::string SettingsDescription::getDefaultValue (void)
{
	return myDefault;
}

//=============================
// setDefaultValue
//=============================
void SettingsDescription::setDefaultValue (std::string defaultValue)
{
	myDefault = defaultValue;
}

//=============================
// getDescription
//=============================
std::string SettingsDescription::getDescription (void)
{
	return myDescription;
}

//=============================
// setDescription
//=============================
void SettingsDescription::setDescription (std::string description)
{
	myDescription = description;
}

//=============================
// getType
//=============================
SettingType SettingsDescription::getType (void)
{
	return myValueType;
}

//=============================
// getFilePathIntention
//=============================
FilePathSettingIntention SettingsDescription::getFilePathIntention (void)
{
	return myFilePathIntention;
}

//=============================
// hasRange
//=============================
bool SettingsDescription::hasRange (void)
{
	return myHasRange;
}

//=============================
// getRangeMin
//=============================
std::string SettingsDescription::getRangeMin (void)
{
	return myRangeMin;
}

//=============================
// getRangeMax
//=============================
std::string SettingsDescription::getRangeMax (void)
{
	return myRangeMax;
}

//=============================
// isEnumSelectionOneRequired
//=============================
bool SettingsDescription::isEnumSelectionOneRequired (void)
{
	return myEnumSelectionOneRequired;
}

//=============================
// getEnumeration
//=============================
EnumList* SettingsDescription::getEnumeration (void)
{
	return myEnum;
}

//=============================
// getTypeAsString
//=============================
std::string SettingsDescription::getTypeAsString (void) const
{
	std::string ret = "";

	switch (myValueType)
	{
	case BOOL_SETTING:
		ret = "bool";
		break;
	case FLOAT_SETTING:
		ret = "float";
		break;
	case INTEGER_SETTING:
		ret = "integer";
		break;
	case ENUM_SETTING:
		ret = "enum";
		break;
	case ENUM_SELECTION_SETTING:
		ret = "enum-selection";
		break;
	case FILE_PATH_SETTING:
		ret = "file-path";
		break;
	case PATH_SETTING:
		ret = "path";
		break;
	case STRING_SETTING:
		ret = "string";
		break;
	case UNKNOWN_SETTING:
		ret = "unknown";
		break;
	}

	return ret;
}

//=============================
// print
//=============================
std::ostream& SettingsDescription::print (std::ostream& out) const
{
	//Basic information
	out
		<< "type="
		<< getTypeAsString ()
		<< ", name="
		<< myName
		<< ", default="
		<< myDefault;

	//Range
	if (myHasRange)
	{
		assert (myRangeMin != "*" ||
				myRangeMax != "*");

		out << ", range=[";

		if (myRangeMin == "*")
		{
			out << "<=" << myRangeMax;
		}
		else if (myRangeMax == "*")
		{
			out << ">=" << myRangeMin;
		}
		else
		{
			out << myRangeMin << "-" << myRangeMax;
		}

		out << "]";

	}

	//Enums
	if (myValueType == ENUM_SETTING ||
		myValueType == ENUM_SELECTION_SETTING)
	{
		assert (myEnum);
		out << ", enumId=" << myEnum->getId();
	}

	if (myValueType == ENUM_SELECTION_SETTING)
	{
		out << ", atLeastOneSelectionRequired=";
		if (myEnumSelectionOneRequired)
			out << "true";
		else
			out << "false";
	}

	//file path intention
	if (myValueType == FILE_PATH_SETTING)
	{
		out << ", fileUsageIntention=";
		switch (myFilePathIntention)
		{
		case IN_INTENTION:
			out << "input";
			break;
		case OUT_INTENTION:
			out << "output";
			break;
		case IN_OUT_INTENTION:
			out << "input+output";
			break;
		case UNKNOWN_INTENTION:
			out << "unknown";
			break;
		}
	}

	//possibly lengthy description
	out
		<< ", description=\""
		<< myDescription
		<< "\"";

	return out;
}

//=============================
// isValidValue
//=============================
bool SettingsDescription::isValidValue (std::string value)
{
	switch (myValueType)
	{
	case BOOL_SETTING:
		if (value != "0" && value != "1")
			return false;
		break;
	case FLOAT_SETTING:
	{
		float f;
		if (sscanf (value.c_str(), "%f", &f) != 1)
			return false;

		if (myHasRange)
		{
			float fmin;
			float fmax;
			bool hasMin = false;
			bool hasMax = false;

			if (myRangeMin != "*")
			{
				sscanf (myRangeMin.c_str(), "%f", &fmin);
				hasMin = true;
			}

			if (myRangeMax != "*")
			{
				sscanf (myRangeMax.c_str(), "%f", &fmax);
				hasMax = true;
			}

			if (hasMin && f < fmin)
				return false;

			if (hasMax && f > fmax)
				return false;
		}

		break;
	}
	case INTEGER_SETTING:
	{
		int i;
		if (sscanf (value.c_str(), "%d", &i) != 1)
			return false;

		if (myHasRange)
		{
			int imin;
			int imax;
			bool hasMin = false;
			bool hasMax = false;

			if (myRangeMin != "*")
			{
				sscanf (myRangeMin.c_str(), "%d", &imin);
				hasMin = true;
			}

			if (myRangeMax != "*")
			{
				sscanf (myRangeMax.c_str(), "%d", &imax);
				hasMax = true;
			}

			if (hasMin && i < imin)
				return false;

			if (hasMax && i > imax)
				return false;
		}

		break;
	}
	case ENUM_SETTING:
		return myEnum->isValidEntry(value);
	case ENUM_SELECTION_SETTING:
	{
		//break value into parts seperated by ";"
		char *temp = new char[value.length()+1];
		sprintf (temp, "%s", value.c_str());
		char* pch = strtok (temp,";");
		while (pch != NULL)
		{
			if (!myEnum->isValidEntry(pch))
				return false;
			//next
			pch = strtok (NULL, ";");
		}

		break;
	}
	case FILE_PATH_SETTING:
	case PATH_SETTING:
		/*not going to test anything here, files may only be reachable on backend, ...*/
		break;
	case STRING_SETTING:
		/*well probably ok ;)*/
		break;
	case UNKNOWN_SETTING:
		return false;
	}

	return true;
}

/*EOF*/
