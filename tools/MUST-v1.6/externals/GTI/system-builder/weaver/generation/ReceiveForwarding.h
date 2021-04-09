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
 * @file ReceiveForwarding.h
 * 		@see gti::weaver::generation::ReceiveForwarding
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#ifndef RECEIVEFORWARDING_H
#define RECEIVEFORWARDING_H

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
			 * Generates the receival and forwarding
			 * generator input by
			 * processing the properties of a level and
			 * creating the respective XML.
			 *
			 * Usage: a completely processed level with
			 *        all call properties computed may
			 *        create an instance of this class
			 *        in order generate the receival and
			 *        forwarding input.
			 */
			class ReceiveForwarding : public GenerationBase
			{
			public:
				/**
				 * Creates a ReceiveForwarding object and immediately
				 * starts with the generation of the receive and
				 * forwarding generator input.
				 * @param level to inherit input for
				 *        generation from.
				 * @param fileName file name to use for writing the
				 *        receive and forwarding generator input.
				 * @param sourceFileName name of the source
				 *        file that should be created by
				 *        the receive and forwarding generator.
				 * @param headerFileName name of the header
				 *        file that should be created by the
				 *        receive and forwarding generator.
				 * @param logFileName name of the log file that
				 *        should be created by the forwarding and
				 *        receival generator.
				 */
				ReceiveForwarding (
						Level *level,
						std::string fileName,
						std::string sourceFileName,
						std::string headerFileName,
						std::string logFileName);
				~ReceiveForwarding (void);

			protected:

				/**
				 * Prints the receival XML node for the given
				 * call.
				 * @param true if successful.
				 */
				bool printReceival (Call *call, CalculationOrder order);
			};

		}
	}
}

#endif /*RECEIVEFORWARDING_H */
