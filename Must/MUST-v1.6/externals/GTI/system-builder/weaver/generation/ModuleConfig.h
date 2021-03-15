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
 * @file ModuleConfig.h
 * 		@see gti::weaver::generation::ModuleConfig
 *
 * @author Tobias Hilbrich
 * @date 1.11.2010
 */

#ifndef MODULECONFIG_H
#define MODULECONFIG_H

#include <string>
#include <vector>

#include "GenerationBase.h"

using namespace gti::weaver::layout;
using namespace gti::weaver::modules;

namespace gti
{
	namespace weaver
	{
		namespace generation
		{
			/**
			 * Generates an XML that describes the modules
			 * used on a level along with their relationship.
			 * This input should be used to create an
			 * actual module configuration, e.g. for PnMPI.
			 *
			 * Usage: a completely processed level with
			 *        all call properties computed may
			 *        create an instance of this class
			 *        in order generate a module
			 *        configuration XML.
			 */
			class ModuleConfig : public GenerationBase
			{
			public:
				/**
				 * Creates a ModuleConfig object and immediately
				 * starts with the generation of the used module
				 * and module relationship XML.
				 * @param level to inherit input for
				 *        generation from.
				 * @param fileName output file name which will
				 *        be used to write the module configuration
				 *        information of this level to.
				 *
				 * Important: all levels should write their
				 *            output into one file to create
				 *            a document of type
				 *            module-config.dtd, so this file
				 *            will be opened and closed multiple
				 *            times. A header and footer needs to
				 *            be written to it before/after all
				 *            the levels wrote their respective
				 *            configuration nodes.
				 */
				ModuleConfig (
						Level *level,
						std::string fileName);
				~ModuleConfig (void);

			protected:
				std::list<Setting*> myExtraSettings;
				int myOwnIndex; /**< Number or predecessor layers. */

				/**
				 * Prints information on the a module to the
				 * output XML.
				 *
				 * @param module pointer to the module to print.
				 * @param type module type, any one out of "analysis", "wrapper", "receival", "place", "protocol-up", "protocol-down", "strategy".
				 * @param name identifier to use for this module.
				 * @param settings list of settings for this module.
				 * @param needsToLevel true if this module is a protocol that connects to another level.
				 * @param toLevel if needsToLevel is true, id of the level to which the protocol connects.
				 *
				 * @return true if successful, false otherwise.
				 */
				bool printModule (
						Module *module,
						std::string type,
						std::string name,
						std::vector<Setting*> settings = std::vector<Setting*> (),
						bool needsToLevel = false,
						int toLevel = 0);

				/**
				 * Extension of the other version of printModule. Used
				 * for modules that need prepended modules.
				 *
				 * @param prepModule pointer to Prepended to use for
				 *        accessing information on all prepended modules.
				 *
				 * @return true if successful, false otherwise.
				 */
				bool printModule (
						Module *module,
						Prepended *prepModule,
						std::string type,
						std::string name,
						std::vector<Setting*> settings = std::vector<Setting*> (),
						bool needsToLevel = false,
						int toLevel = 0);

				/**
				 * Prints the modules for an adjacency (strategy and protocol module)
				 * with printModule.
				 * @param a the adjacency to print modules for.
				 * @param moduleNamePrefix string to be appended in front of the protocol
				 *        and strategy names, the names will be "<moduleNamePrefix>strategy"
				 *        and "<moduleNamePrefix>protocol".
				 * @param protocolType the module type to use for the protocol, either
				 *        "protocol-up" or "protocol-down".
				 */
				bool printAdjacency (
						Adjacency *a,
						std::string moduleNamePrefix,
						std::string protocolType,
						bool isDown);

				/**
				 * Adds the toAdd list of settings to an existing vector of settings (settings).
				 * @param settings existing list of settings
				 * @param toAdd list of settings to add
				 * @return augmented settings list.
				 */
				std::vector<Setting*> augmentSettings (std::vector<Setting*> settings, std::list<Setting*> toAdd);

			}; /*class ModuleConfig*/

		}
	}
}

#endif /*RECEIVEFORWARDING_H */
