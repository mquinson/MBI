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
 * @file Place.h
 * 		@see gti::weaver::Place
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef PLACE_H
#define PLACE_H

#include <string>
#include <list>
#include <vector>

#include "Module.h"
#include "Prepended.h"
#include "Configurable.h"
#include "RequiresApi.h"
#include "enums.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Class that captures the specification
			 * for a place of the GTI.
			 */
			class Place : public Module, public Prepended, public RequiresApi, public Configurable, virtual public Printable
			{
			public:
				/**
				 * Empty constructor.
				 */
				Place (void);

				/**
				 * Constructor with initialization as application.
				 * @param executableName the name of the executable.
				 * @param prependModules list of modules that need to be put at the root
				 *        of the module stack when using this module.
				 * @param apis list of APIs that are required to use this protocol.
				 * @param settings list of settings for this module.
				 */
				Place (
						std::string name,
						std::list<std::string> apis,
						std::list<SettingsDescription*> settings);
				/**
				 * Constructor with initialization as executable.
				 * @param executableName the name of the executable.
				 * @param prependModules list of modules that need to be put at the root
				 *        of the module stack when using this module.
				 * @param apis list of APIs that are required to use this protocol.
				 * @param settings list of settings for this module.
				 */
				Place (
						std::string executableName,
						std::list<Module*> prependModules,
						std::list<std::string> apis,
						std::list<SettingsDescription*> settings);

				/**
				 * Constructor with initialization as Module.
				 * @param moduleName the module name of this protocol.
				 * @param configName the name of this module used for P^nMPI configurations
				 * @param prependModules list of modules that need to be put at the root
				 *        of the module stack when using this module.
				 * @param apis list of APIs that are required to use this protocol.
				 * @param settings list of settings for this module.
				 */
				Place (
						std::string moduleName,
						std::string configName,
						std::list<Module*> prependModules,
						std::list<std::string> apis,
						std::list<SettingsDescription*> settings);

				/**
				 * Constructor with initialization as Module.
				 * @param moduleName the module name of this protocol.
				 * @param configName the name of this module used for P^nMPI configurations
				 * @param instanceType the type name used for an instance of this module
				 * @param headerName the header that declares the type for this module
				 * @param incDir path to the include directory in which the module header is stored
				 * @param prependModules list of modules that need to be put at the root
				 *        of the module stack when using this module.
				 * @param apis list of APIs that are required to use this protocol.
				 * @param settings list of settings for this module.
				 */
				Place (
						std::string moduleName,
						std::string configName,
						std::string instanceType,
						std::string headerName,
						std::string incDir,
						std::list<Module*> prependModules,
						std::list<std::string> apis,
						std::list<SettingsDescription*> settings);

				/**
				 * Empty Destructor
				 */
				virtual ~Place ( );

				/**
				 * Returns true if this place is a module,
				 * false otherwise.
				 * @return true if a module
				 */
				bool isModule ();

				/**
				 * Returns the name of the place.
				 * For executables it is the name of the executable,
				 * for modules the module name (not the config file
				 * name).
				 * @return name of place.
				 */
				std::string getName (void);

				/**
				 * Sets the name of the executable for this place,
				 * only valid for non Module places.
				 * @param name the name for this place
				 */
				void setExecutableName (std::string name);

				/**
				 * Returns the name of the executable for this place,
				 * only valid for non Module places.
				 * @return executable name of this place
				 */
				std::string getExecutableName (void);

				/**
				 * Sets the type of this place.
				 * @param type new type
				 */
				void setType (PlaceType type);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
				PlaceType myType;
				std::string myExecutableName;
			};
		} /*namespace modules*/
	}
}
#endif // PLACE_H
