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
 * @file Gti.h
 * 		@see gti::weaver::Gti
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef GTI_H
#define GTI_H

#include <string>
#include <vector>

#include "GtiEnums.h"

#include "CommStrategy.h"
#include "CommStrategyIntra.h"
#include "CommProtocol.h"
#include "Place.h"
#include "SpecificationNode.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Singleton that manages the communication
			 * strategies, protocols and places from a
			 * GTI specification.
			 */
			class Gti
			{
			public:
				/**
				 * Returns the singleton instance.
				 * @return Gti
				 */
				static Gti* getInstance (void);

				/**
				 * @param  gtiSpecificationXml
				 */
				GTI_RETURN load (std::string gtiSpecificationXml );

				/**
				 * Returns the enumeration with the given id.
				 * @param id enumeration id.
				 * @return pointer to the enum if successful, NULL otherwise.
				 */
				EnumList* findEnumForId (int id);

				/**
				 * Searches for a place with the given name in the
				 * list of loaded GTI places.
				 * @param name name of the place to search for.
				 * @return pointer to place if successful, NULL otherwise.
				 */
				Place* findPlace (std::string name);

				/**
				 * Searches for a communication strategy with the
				 * given name.
				 * @param name of the comm strategy.
				 * @return pointer to strategy if found, NULL otherwise.
				 */
				CommStrategy* findStrategy (std::string name);

				/**
				 * Searches for a communication strategy (intra) with the
				 * given name.
				 * @param name of the comm strategy.
				 * @return pointer to strategy if found, NULL otherwise.
				 */
				CommStrategyIntra* findStrategyIntra (std::string name);

				/**
				 * Searches for a communication protocol with the
				 * given name.
				 * @param name of the comm protocol.
				 * @return pointer to protocol if found, NULL otherwise.
				 */
				CommProtocol* findProtocol (std::string name);

			protected:

				static Gti* myInstance;

				std::vector<CommStrategy*> myCommStrategies;
				std::vector<CommStrategyIntra*> myCommStrategiesIntra;
				std::vector<CommProtocol*> myCommProtocols;
				std::vector<Place*> myPlaces;
				std::vector<EnumList*> myEnums;

				std::string myLibDir;
				std::string myIncDir;
				std::string myExecDir;

				/**
				 * Empty Constructor
				 */
				Gti ( );

				/**
				 * Empty Destructor
				 */
				virtual ~Gti ( );

				/**
				 * Add a myCommStrategies object to the myCommStrategies List
				 */
				void addCommStrategy ( CommStrategy * add_object );

				/**
				 * Remove a myCommStrategies object from myCommStrategies List
				 */
				void removeCommStrategy ( CommStrategy * remove_object );

				/**
				 * Adds an intra comm strategy.
				 */
				void addCommStrategyIntra ( CommStrategyIntra * add_object );

				/**
				 * Removes the given intra comm strategy
				 */
				void removeCommStrategyIntra ( CommStrategyIntra * remove_object );

				/**
				 * Add a MyCommProtocols object to the myCommProtocols List
				 */
				void addCommProtocol ( CommProtocol * add_object );

				/**
				 * Remove a MyCommProtocols object from myCommProtocols List
				 */
				void removeCommProtocol ( CommProtocol * remove_object );

				/**
				 * Adds a enum to the list of enums known by the GTI.
				 * @param addEnum the enum to add.
				 */
				void addEnum (EnumList *addEnum);

				/**
				 * Removes an enum from the list of GTI enums.
				 * @param removeEnum the enum to remove.
				 */
				void removeEnum (EnumList *removeEnum);

				/**
				 * Add a Place object to the myPlaces List
				 */
				void addPlace ( Place * add_object );

				/**
				 * Remove a MyPlaces object from myPlaces List
				 */
				void removePlace ( Place * remove_object );

				/**
				 * Reads all communication protocols from the GTI
				 * specification.
				 * @param root the root node of the GTI specification.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readCommProtocols (SpecificationNode root);

				/**
				 * Reads a communication protocol from the GTI
				 * specification.
				 * @param node the node of the GTI comm protocol.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readCommProtocol (SpecificationNode node);

				/**
				 * Reads all communication strategies from the GTI
				 * specification.
				 * @param root the root node of the GTI specification.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readCommStrategies (SpecificationNode root);

				/**
				 * Reads all places from the GTI
				 * specification.
				 * @param root the root node of the GTI specification.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readPlaces (SpecificationNode root);

				/**
				 * Reads all enumerations from the GTI
				 * specification.
				 * @param root the root node of the GTI specification.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readEnums (SpecificationNode root);

				/**
				 * Reads a settings node from the GTI specification XML.
				 * @param node the settings node.
				 * @param settings the list to store the settings in.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readSettings (SpecificationNode node, std::list<SettingsDescription*> *settings);

				/**
				 * Reads a setting node from the GTI specification XML.
				 * @param node the setting node.
				 * @param setting pointer to an unallocated pointer for a setting,
				 *        which is used to allocate space for the setting and to
				 *        store it.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readSetting (SpecificationNode node, SettingsDescription **setting);

				/**
				 * Reads a value node from the GTI.
				 * @param node the value node.
				 * @param pType out: type of the value.
				 * @param pDefaultValue out: default setting for this value.
				 * @param pHasRange out: has this value a range limit.
				 * @param pRangeMin out: lower range bound.
				 * @param pRangeMax out: upper range bound.
				 * @param pIntention out: usage intention for file value.
				 * @param pEnumId out: id of a possibly associated enumeration.
				 * @param pSelectionRequired out: for enum selection value,
				 *        specifies whether at least one selection is required.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readValue (
						SpecificationNode node,
						SettingType *pType,
						std::string *pDefaultValue,
						bool *pHasRange,
						std::string *pRangeMin,
						std::string *pRangeMax,
						FilePathSettingIntention *pIntention,
						int *pEnumId,
						bool *pSelectionRequired
						);

				/**
				 * Reads an instance node from the GTI.
				 * @param node the instance node.
				 * @param pIsModule out: set to true if the instance node specified a module.
				 * @param pIsModule out: set to true if the instance node specified an application node.
				 * @param pModuleName out: set to specified module name for module instances.
				 * @param pExecutableName out: set to specified executable name for executable instances.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readInstance (
						SpecificationNode node,
						bool *pIsModule,
						bool *pIsApp,
						std::string *pModuleName,
						std::string *pExecutableName);

				/**
				 * Reads a prepend-modules node nad returns the read
				 * list of prepend modules.
				 * @param node the prepend-modules node to read in.
				 * @param prependModules a std::list to store the output in.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readPrependModules (SpecificationNode node, std::list<Module*> *prependModules);

				/**
				 * Reads a required-apis node from the specification.
				 * @param node the required-apis node.
				 * @param apis output for the list with the names of the required APIs.
				 * @return GTI_SUCCESS if successful.
				 */
				GTI_RETURN readRequiredApis (SpecificationNode node, std::list<std::string> *apis);
			};
		} /*namespace modules*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // GTI_H

