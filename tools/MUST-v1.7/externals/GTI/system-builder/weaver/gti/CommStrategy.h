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
 * @file CommStrategy.h
 * 		@see gti::weaver::CommStrategy
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef COMMSTRATEGY_H
#define COMMSTRATEGY_H

#include <string>
#include <vector>

#include "Module.h"
#include "Configurable.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Holds information for a communication
			 * strategy of the GTI.
			 * Uses the base class as upwards module
			 * and the member as downwards module.
			 */
			class CommStrategy : public Module, public Configurable, public virtual Printable
			{
			public:
				/**
				 * Empty Constructor
				 */
				CommStrategy ( );

				/**
				 * Constructor with arguments.
				 * @param moduleNameUp Name of the up module.
				 * @param configNameUp P^nMPI config name of the up module.
				 * @param moduleNameDown Name of the down module.
				 * @param configNameDown P^nMPI config name of the down module.
				 * @param settings list of SettingsDescriptions which lists the options for this strategy.
				 */
				CommStrategy (
						std::string moduleNameUp,
						std::string configNameUp,
						std::string moduleNameDown,
						std::string configNameDown,
						std::list<SettingsDescription*> settings);

				/**
				 * Constructor with arguments.
				 * @param moduleNameUp Name of the up module.
				 * @param configNameUp P^nMPI config name of the up module.
				 * @param moduleNameDown Name of the down module.
				 * @param configNameDown P^nMPI config name of the down module.
				 * @param instanceType datatype of a instance of this module.
				 * @param headerName header for this module.
				 * @param incDir include directory for this header.
				 * @param settings list of SettingsDescriptions which lists the options for this strategy.
				 */
				CommStrategy (
						std::string moduleNameUp,
						std::string configNameUp,
						std::string moduleNameDown,
						std::string configNameDown,
						std::string instanceType,
						std::string headerName,
						std::string incDir,
						std::list<SettingsDescription*> settings);

				/**
				 * Empty Destructor
				 */
				virtual ~CommStrategy ( );

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Returns a pointer to the down module of the strategy.
				 * @return pointer to down module.
				 */
				Module* getDownModule (void);

			private:
				Module myDownModule;

			}; /*CommStrategy*/
		} /*namespace modules*/
	} /*namespace weaver*/
} /*namepsace gti*/

#endif // COMMSTRATEGY_H
