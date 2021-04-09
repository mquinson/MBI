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
 * @file BuildGen.h
 *		@see gti::codegen::BuildGen
 *
 * @author Tobias Hilbrich
 * @date 19.08.2010
 */

#include <fstream>
#include <map>

#include "SpecificationNode.h"

#ifndef BUILD_GEN_H
#define BUILD_GEN_H

using namespace gti::weaver;

/**
 * Prints usage information for the generator.
 * @param execName name of the generator
 *        executable (argv[0]).
 * @param out output stream to use.
 */
void printUsage (std::string execName, std::ostream &out);

namespace gti
{
	namespace codegen
	{
		/**
		 * Generator for build files. Reads an input XML that
		 * lists the sources and properties for a list of modules
		 * the generator processes them and creates a CMake build
		 * file for all of the modules. The output is a
		 * CMakeLists.txt in the current directory.
		 */
		class BuildGen
		{
		public:
			/**
			 * Constructor.
			 * @param inputFile input XML to process and generate for.
			 * @param retVal pointer to storage for return value to use.
			 *        (is set to 0 is generation successful and to 1
			 *         otherwise)
			 */
			BuildGen (std::string inputFile, int* retVal);

			/**
			 * Destructor.
			 */
			~BuildGen (void);

		protected:

			std::string myModPath;
			std::string myCMakeModPath;
			std::ofstream myBuildOut;

			/**
			 * Reads the settings node of the input XML.
			 * @param node the node to read.
			 * @return true iff successful.
			 */
			bool readSettings (SpecificationNode node);

			/**
			 * Reads a module node of the input XML.
			 * @param node to read.
			 * @return true iff successful.
			 */
			bool readModule (SpecificationNode node);
		};
	}
}

#endif /*BUILD_GEN_H*/
