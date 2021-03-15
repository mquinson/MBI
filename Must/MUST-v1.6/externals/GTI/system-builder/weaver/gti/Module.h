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
 * @file Module.h
 * 		@see gti::weaver::Module
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef MODULE_H
#define MODULE_H

#include <string>

#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Base class for all P^nMPI Modules.
			 */
			class Module : virtual public Printable
			{
			public:
				/**
				 * Empty Constructor
				 */
				Module ( );

				/**
				 * Constructor with initialization
				 */
				Module (
						std::string moduleName,
						std::string configName
						);

				/**
				 * Constructor with initialization
				 */
				Module (
						std::string moduleName,
						std::string configName,
						std::string instanceType,
						std::string headerName,
						std::string incDir
						);

				/**
				 * Empty Destructor
				 */
				virtual ~Module (void);

				/**
				 * Sets the name used to register this module with P^nMPI.
				 * @param new_var new name
				 */
				void setModuleName ( std::string new_var );

				/**
				 * Returns the name used to register this module with P^nMPI.
				 * @return name
				 */
				std::string getModuleName (void);

				/**
				 * Sets the name that is used to load this module in a P^nMPI
				 * configuration file.
				 * @param new_var new config file name
				 */
				void setConfigName ( std::string new_var );

				/**
				 * Returns the name that is used to load this module in a P^nMPI
				 * configuration file.
				 * @return config file name
				 */
				std::string getConfigName (void);

				/**
				 * Sets the type name for an instance of the module (for code).
				 * @param instanceType type name
				 */
				void setInstanceType (std::string instanceType);

				/**
				 * Returns the type name for an instance of the module (for code).
				 * @return type name
				 */
				std::string getInstanceType (void);

				/**
				 * Sets the name of the header that declares the type name.
				 * @param headerName header name
				 */
				void setHeaderName (std::string headerName);

				/**
				 * Returns the name of the header that declares the type name.
				 * @return header name
				 */
				std::string getHeaderName (void);

				/**
				 * Sets the include directory used for the header that declares
				 * the type name.
				 * @param incDir include directory to use
				 */
				void setIncDir (std::string incDir);

				/**
				 * Returns the include directory path used for the header that
				 * declares the type name.
				 * @return the include directory path
				 */
				std::string getIncDir (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
				std::string myModuleName; /**< Required: name which is used to register the module with P^nMPI.*/
				std::string myConfigName; /**< Required: name which is used to load this module in a P^nMPI configuration file.*/
				std::string myInstanceType; /**< Optional: Name of the data type to use for an instance of this module.*/
				std::string myHeaderName; /**< Optional: Name of the header the provides the declaration of the instance type.*/
				std::string myIncDir; /**< Directory in which the module header resides.*/

			}; /*Module*/
		} /*namespace modules*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // MODULE_H
