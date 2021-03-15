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
 * @file Configurable.h
 * 		@see gti::weaver::Configurable
 *
 * @author Tobias Hilbrich
 * @date 14.01.2010
 */

#ifndef CONFIGUREABLEMODULE_H
#define CONFIGUREABLEMODULE_H

#include <string>
#include <map>
#include <list>

#include "Module.h"
#include "SettingsDescription.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Base class for a module that has settings.
			 */
			class Configurable : virtual public Printable
			{
			public:
				/**
				 * Constructor
				 */
				Configurable ();

				/**
				 * Constructor with arguments.
				 * @param settings list of SettingsDescriptions that contains the options for this module.
				 */
				Configurable (std::list<SettingsDescription*> settings);

				/**
				 * Destructor
				 */
				virtual ~Configurable ( );

				/**
				 * Add a SettingsDescription to the list of settings for
				 * this strategy.
				 * @param add_object the setting to add
				 */
				void addSetting (SettingsDescription * add_object );

				/**
				 * Remove a SettingsDescription from the list of
				 * settings for this strategy.
				 * @param remove_object the setting to remove
				 */
				void removeSetting (SettingsDescription * remove_object);

				/**
				 * Searches for a setting with the given name.
				 * @param name of the setting to look for.
				 * @return description for the named setting.
				 */
				SettingsDescription* findSetting (std::string name);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Tests whether the given setting is valid
				 * with the settings description for this
				 * configurable object.
				 * @param settingName name of setting.
				 * @param settingValue desired value for setting.
				 * @return true if the setting is valid, false otherwise.
				 */
				bool isValidSetting (std::string settingName, std::string settingValue);

			protected:
				std::map <std::string, SettingsDescription*> mySettings; /**< Maps the name of each setting to its description. */

			}; /*Configurable*/
		}
	} /*namespace weaver*/
} /*namepsace gti*/

#endif // CONFIGUREABLEMODULE_H
