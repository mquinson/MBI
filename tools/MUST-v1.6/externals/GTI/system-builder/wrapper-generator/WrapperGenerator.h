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
 * @file WrapperGenerator.h
 *		@see gti::codegen::WrapperGenerator
 *
 * @author Tobias Hilbrich
 * @date 11.08.2010
 */

#include <fstream>
#include <map>

#include "GeneratorBase.h"

#ifndef WRAPPER_GENERATOR_H
#define WRAPPER_GENERATOR_H

using namespace gti::weaver;

/**
 * Prints usage information for the wrapper generator.
 * @param execName name of the wrapper generator
 *        executable (argv[0]).
 * @param out output stream to use.
 */
void printUsage (std::string execName, std::ostream &out);

namespace gti
{
	namespace codegen
	{
		/**
		 * WrapperGenerator main class that processes the
		 * generator input.
		 */
		class WrapperGenerator : public GeneratorBase
		{
		protected:


		public:
			/**
			 * Constructor.
			 * @param inputFile input XML to process and generate for.
			 * @param retVal pointer to storage for return value to use.
			 *        (is set to 0 is generation successful and to 1
			 *         otherwise)
			 */
			WrapperGenerator (std::string inputFile, int* retVal);

			/**
			 * Destructor.
			 */
			~WrapperGenerator (void);

		protected:

			int myOrder;

			/**
			 * Returns the name of the implementing generator.
			 * @return generator name.
			 */
			virtual std::string myGetGeneratorName (void);

			/**
			 * Returns the name of the root node for the
			 * type of XML specification used by the
			 * generator.
			 * @return root node name.
			 */
			virtual std::string myGetRootNodeName (void);

			/**
			 * Returns the name to use as record name when accessing
			 * an argument of a record.
			 * @return record name.
			 */
			virtual std::string myGetRecordName (void);

			/**
			 * Writs the base definition of the wrapper class
			 * to the output header and the basic initialization
			 * functions to the output source.
			 */
			bool writeSourceHeaderBasics (void);

			/**
			 * Reads the "calls" node in the wrapper generator
			 * input XML.
			 * @param node to read.
			 * @return true if successful, false otherwise.
			 */
			bool readCalls (SpecificationNode node);

			/**
			 * Reads a "call" node in the wrapper generator
			 * input XML.
			 * @param node to read.
			 * @param pOutCallName name of the call that was read.
			 * @param pIsWrapAcross pinter to storage for a bool value, is set to true if
			 *               this was a wrap-across call, to false otherwise.
			 * @return true if successful, false otherwise.
			 */
			bool readCall (SpecificationNode node, std::string *pOutCallName, bool *pIsWrapAcross, bool *pIsWrapDown);

			/**
			 * Reads all "exec-analysis" and "source-piece"
			 * child nodes of this node (usually a pre or post
			 * node) and prints these to the source.
			 * @param node to process.
			 * @return true if successful, false otherwise.
			 */
			bool printAnalysesPieces (SpecificationNode node);

			/**
			 * Reads a source-piece node and prints its code
			 * into the source output.
			 * @param node of the source-piece.
			 * @return true if successful, false otherwise.
			 */
			bool printSourcePiece (SpecificationNode node);

			/**
			 * Reads an exec-analysis node and prints the
			 * resulting code into the source output.
			 * @param node of the exec-analysis.
			 * @return true if successful, false otherwise.
			 */
			bool printExecAnalysis (SpecificationNode node);

		};
	}/*namespace codegen*/
}/*namespace gti*/

#endif /*WRAPPER_GENERATOR_H*/
