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
 * @file CommProtocol.h
 * 		@see gti::weaver::CommProtocol
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef COMMPROTOCOL_H
#define COMMPROTOCOL_H

#include <string>
#include <vector>
#include <list>

#include "Configurable.h"
#include "Module.h"
#include "Prepended.h"
#include "RequiresApi.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Holds the description of a communication protocol.
			 */
			class CommProtocol : public Module, public Configurable, public Prepended, public RequiresApi, virtual public Printable
			{
			public:
				/**
				 * Empty Constructor
				 */
				CommProtocol ( );

				/**
				 * Constructor with initialization.
				 * @param moduleName the module name of this protocol.
				 * @param configName the name of this module used for P^nMPI configurations
				 * @param settings the list of setting descriptions for this module.
				 * @param prependModules list of modules that need to be put at the root
				 *        of the module stack when using this module.
				 * @param apis list of APIs that are required to use this protocol.	 *
				 */
				CommProtocol (
						std::string moduleName,
						std::string configName,
						std::list<SettingsDescription*> settings,
						std::list<Module*> prependModules,
						std::list<std::string> apis,
						bool supportsIntraComm = false);

				/**
				 * Constructor with initialization.
				 * @param moduleName the module name of this protocol.
				 * @param configName the name of this module used for P^nMPI configurations
				 * @param instanceType the type name used for an instance of this module
				 * @param headerName the header that declares the type for this module
				 * @param incDir path to the include directory in which the module header is stored
				 * @param settings the list of setting descriptions for this module.
				 * @param prependModules list of modules that need to be put at the root
				 *        of the module stack when using this module.
				 * @param apis list of APIs that are required to use this protocol.	 *
				 */
				CommProtocol (
						std::string moduleName,
						std::string configName,
						std::string instanceType,
						std::string headerName,
						std::string incDir,
						std::list<SettingsDescription*> settings,
						std::list<Module*> prependModules,
						std::list<std::string> apis,
						bool supportsIntraComm = false);

				/**
				 * Empty Destructor
				 */
				virtual ~CommProtocol ( );

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Returns true if this protocol supports intra layer communication.
				 * @return true or false.
				 */
				bool supportsIntraComm (void);

			protected:
				bool mySupportsIntraComm; /**< True if this protocol supports intra-layer communication.*/
			}; /*CommProtocol*/
		} /*namespace modules*/
	} /*namespace weaver*/
} /*namespace gti*/

#endif // COMMPROTOCOL_H
