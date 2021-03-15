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
 * @file SettingsDescription.h
 * 		@see gti::weaver::SettingsDescription
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef SETTINGSDESCRIPTION_H
#define SETTINGSDESCRIPTION_H

#include <string>

#include "EnumList.h"
#include "Printable.h"
#include "enums.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Stores the specification of a module setting.
			 */
			class SettingsDescription : virtual public Printable
			{
			public:
				/**
				 * Empty Constructor, for invalid object.
				 */
				SettingsDescription ( );

				/**
				 * Constructor for the following values:
				 *  - bool
				 *  - float (without range)
				 *  - integer (without range)
				 *  - path
				 *  - string
				 * Will fail for other value types !
				 */
				SettingsDescription (
						std::string name,
						std::string description,
						std::string defaultValue,
						SettingType myValueType);

				/**
				 * Constructor for the following values:
				 *  - enum
				 *  - enumselection (without at least one selection required)
				 * Will fail for other value types !
				 */
				SettingsDescription (
						std::string name,
						std::string description,
						std::string defaultValue,
						EnumList    *enumEntries,
						SettingType myValueType);

				/**
				 * Constructor for the following values:
				 *  - enumselection (with at least one selection required)
				 * Will fail for other value types !
				 */
				SettingsDescription (
						std::string name,
						std::string description,
						std::string defaultValue,
						EnumList    *enumEntries,
						bool        selectionRequired,
						SettingType myValueType);

				/**
				 * Constructor for the following values:
				 *  - filepath
				 * Will fail for other value types !
				 */
				SettingsDescription (
						std::string name,
						std::string description,
						std::string defaultValue,
						FilePathSettingIntention myFilePathIntention,
						SettingType myValueType);

				/**
				 * Constructor for the following values:
				 *  - float (with range)
				 *  - integer (with range)
				 * Will fail for other value types !
				 */
				SettingsDescription (
						std::string name,
						std::string description,
						std::string defaultValue,
						std::string rangeMin,
						std::string rangeMax,
						SettingType myValueType);

				/**
				 * Destructor.
				 */
				virtual ~SettingsDescription ( );

				/**
				 * Returns the name of this setting.
				 * @return name
				 */
				std::string getName (void);

				/**
				 * Sets the name of this setting.
				 * @param name name to set
				 */
				void setName (std::string name);

				/**
				 * Returns the default value used for this setting.
				 * @return default value
				 */
				std::string getDefaultValue (void);

				/**
				 * Sets the default value for this setting.
				 * @param defaultValue the default value to set.
				 */
				void setDefaultValue (std::string defaultValue);

				/**
				 * Returns the textual description for this setting.
				 * @return textual description
				 */
				std::string getDescription (void);

				/**
				 * Sets the textual description for this setting.
				 * @param description textual description to set
				 */
				void setDescription (std::string description);

				/**
				 * Returns the type of this setting.
				 * @return type.
				 */
				SettingType getType (void);

				/**
				 * Only valid for settings of the file path type.
				 * Returns the usage intention for the file.
				 * @return usage intention.
				 */
				FilePathSettingIntention getFilePathIntention (void);

				/**
				 * Returns whether this setting has a range limit.
				 * @return true if the setting has a range limit,
				 *         false otherwise.
				 */
				bool hasRange (void);

				/**
				 * Returns the minimum for a range limit.
				 * Only valid if the setting has a range limit
				 * and is of float or integer type.
				 * @return minimum limit.
				 */
				std::string getRangeMin (void);

				/**
				 * Returns the maximum for a range limit.
				 * Only valid if the setting has a range limit
				 * and is of float or integer type.
				 * @return maximum limit.
				 */
				std::string getRangeMax (void);

				/**
				 * Returns whether for this enum selection at
				 * least one element needs to be selected.
				 * Only valid for settings of the enum selection
				 * type.
				 * @return true if at least one entry needs
				 *         to be selected for this enum selection.
				 */
				bool isEnumSelectionOneRequired (void);

				/**
				 * Returns a pointer to the enumeration associated
				 * with this setting.
				 * Only valid for enum and enumselection types.
				 */
				EnumList* getEnumeration (void);

				/**
				 * Returns a string that represents the value type.
				 * @return string.
				 */
				std::string getTypeAsString (void) const;

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Checks whether the given value is a valid
				 * for this description.
				 * @param value to check.
				 * @return true if it is a valid value, false otherwise.
				 */
				bool isValidValue (std::string value);

			protected:
				std::string myName; /**< Name of the setting. */
				std::string myDescription; /**< Textual description for the setting. */
				std::string myDefault; /**< Default value for the setting. */
				SettingType myValueType; /**< Type of the setting. */
				FilePathSettingIntention myFilePathIntention; /**< Usage intention for a file. */
				bool myHasRange; /**< True if this setting has a range limit. */
				std::string myRangeMin; /**< Minium for range limit, if used. */
				std::string myRangeMax; /**< Maximum for range limit, if used. */
				bool myEnumSelectionOneRequired; /**< At least one entry of the enumeration needs to be selected if true, only applies to enum selection setting type. */
				EnumList *myEnum; /**< Link to enum entries, only valid for enum and enum selection settings. */
			};
		} /*namespace modules*/
	}
}
#endif // SETTINGSDESCRIPTION_H
