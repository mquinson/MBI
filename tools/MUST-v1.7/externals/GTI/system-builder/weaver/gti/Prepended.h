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
 * @file Prepended.h
 * 		@see gti::weaver::Prepended
 *
 * @author Tobias Hilbrich
 * @date 14.01.2010
 */

#ifndef PREPENDED_H
#define PREPENDED_H

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
			 * A class that stores P^nMPI modules that need to be put
			 * onto the stack before a certain module.
			 */
			class Prepended : virtual public Printable
			{
			public:
				/**
				 * Constructor
				 */
				Prepended ();

				/**
				 * Constructor with arguments.
				 * @param prependModules list of modules to pre load before loading this module.
				 */
				Prepended (std::list<Module*> prependModules);

				/**
				 * Destructor
				 */
				virtual ~Prepended ( );

				/**
				 * Adds a module to the list of prepend modules.
				 */
				void addPrependModule ( Module* add_object );

				/**
				 * Removes the given module from the list of prepend modules.
				 */
				void removePrependModule ( Module* remove_object );

				/**
				 * Returns the list of modules to prepend.
				 * @return list of modules.
				 */
				std::list<Module*> getPrependedModules (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
				std::list<Module*> myPrependModules;

			}; /*Prepended*/
		} /*namespace modules*/
	} /*namespace weaver*/
} /*namepsace gti*/

#endif // PREPENDED_H
