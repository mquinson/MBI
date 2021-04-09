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
 * @file Wrapper.h
 * 		@see gti::weaver::Wrapper
 *
 * @author Tobias Hilbrich
 * @date 11.08.2010
 */

#ifndef WRAPPER_H
#define WRAPPER_H

#include <string>
#include <vector>

#include "GenerationBase.h"

using namespace gti::weaver::layout;

namespace gti
{
	namespace weaver
	{
		namespace generation
		{
			/**
			 * Generates the wrapper generator input by
			 * processing the properties of a level and
			 * creating the respective XML.
			 *
			 * Usage: a completely processed level with
			 *        all call properties computed may
			 *        create an instance of this class
			 *        in order generate the wrapper input.
			 */
			class Wrapper : public GenerationBase
			{
			public:
				/**
				 * Creates a Wrapper object and immediately
				 * starts with the generation of the wrapper
				 * generator input.
				 * @param level to inherit input for
				 *        generation from.
				 * @param fileName file name for the wrapper
				 *        generator input.
				 * @param sourceFileName name of the source
				 *        file that should be created by
				 *        the wrapper generator.
				 * @param headerFileName name of the header
				 *        file that should be created by the
				 *        wrapper generator.
				 * @param logFileName name of the log file that
				 *        should be created by the wrapper
				 *        generator.
				 */
				Wrapper (
						Level *level,
						std::string fileName,
						std::string sourceFileName,
						std::string headerFileName,
						std::string logFileName);
				~Wrapper (void);

			protected:
				/**
				 * Prints the wrapper generation input for the given
				 * call. The call needs to be a call which requires
				 * a wrapper on this level.
				 * @param call the call to generate for.
				 */
				void printWrappGenCall (Call* call);

				/**
				 * Prints source-piece nodes
				 * for all operation mapped to the given call with
				 * the given properties.
				 * @param call for which operations should be printed.
				 * @param printCleanup if false prints the actual source
				 *                     of the operation, if true function
				 *                     prints the cleanup source.
				 * @param pExecutedOps pointer to storage for a list of
				 *              operations, any operations that are already in
				 *              this list will not be executed, while newly executed
				 *              ones will be added to the list, can be NULL.
				 * @param order only print source for operation mappings
				 *              of the given order.
				 */
				void printSourcePieces (
						Call *call,
						bool printCleanup,
						CalculationOrder order,
						std::list<std::pair<Operation*, int> > *pExecutedOps);

				/**
				 * Prints source-piece nodes of all operations that are
				 * used as input for an integrity that is mapped to the
				 * given call an order.
				 * Stores the executed ops in the given list.
				 * @param call for which operations should be printed.
				 * @param order only print source for operation mappings
				 *              of the given order.
				 * @param pExecutedOps pointer to storage for a list of
				 *              operations, any operations that are already in
				 *              this list will not be executed, while newly executed
				 *              ones will be added to the list, must not be NULL.
				 */
				void printIntegritySourcePieces (
						Call *call,
						CalculationOrder order,
						std::list<std::pair<Operation*, int> > *pExecutedOps);
			};
		}
	}
}

#endif /*WRAPPER_H */
