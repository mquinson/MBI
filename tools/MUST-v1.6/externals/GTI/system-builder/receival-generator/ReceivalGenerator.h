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
 * @file ReceivalGenerator.h
 *		@see gti::codegen::ReceivalGenerator
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#include <fstream>
#include <map>
#include <sstream>

#include "WrapperGenerator.h"

#ifndef RECEIVAL_GENERATOR_H
#define RECEIVAL_GENERATOR_H

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
		 * Receival generator class which processes a
		 * recival generator input XML in its constructor
		 * and creates the source code for the described
		 * receival module.
		 */
		class ReceivalGenerator : public GeneratorBase
		{
		public:
			/**
			 * Constructor.
			 * @param inputFile input XML to process and generate for.
			 * @param retVal pointer to storage for return value to use.
			 *        (is set to 0 is generation successful and to 1
			 *         otherwise)
			 */
			ReceivalGenerator (std::string inputFile, int* retVal);

			/**
			 * Destructor.
			 */
			~ReceivalGenerator (void);

		protected:

			int myCIdFromLevel; /**< Channel id: index of level from which records arrive.*/
			int myCIdNumLevels; /**< Channel id: total number of levels.*/
			int myCIdNum64s; /**< Channel id: number of 64 bit values in channel id.*/
			int myCIdBitsPerChannel; /**< Channel id: Bits per sub-id of the channel id.*/
			std::string myCIdArgumentBaseName; /**< Channel id: Base name of the argument used to store the 64bit values in the incoming record.*/
			int myCIdStartIndexPre; /**< Channel id: First index to append to base name for records of pre events.*/
			int myCIdStartIndexPost; /**< Channel id: Firts index to append to base name for records of post events.*/

			std::stringstream chanFunc; /**< Stream that holds the function which returns the channel id from a record. */
			std::stringstream forwardFunc; /**< Stream that holds the function which executes the forwarding for a record. */
			std::stringstream intraFunc; /**< Stream that holds the function which executes the forwarding for a record that is communicated within the layer. */
			std::stringstream downFunc; /**< Stream that holds the function which executes the forwarding for a record that is broadcasted downwards. */

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
			 * Reads and processes a "receival" node of the
			 * input specification.
			 * @param node the receival node.
			 * @return true iff successful.
			 */
			bool readReceival (SpecificationNode node);

			/**
			 * Reads the channel-id node.
			 * @param node the channel-id node.
			 * @return true iff successful.
			 */
			bool readChannelId (SpecificationNode node);

			/**
			 * Reads an "exec-analysis" node and prints the
			 * code necessary to execute the analysis for the
			 * given record.
			 * @param out stream to use for printing.
			 * @param node the exec-analysis node.
			 * @param isForForwardWithReduction if true an if is added before executing the analysis that only executes it if avoidReducibleForwards is false.
			 * @param uid uid of the record to print for.
			 * @param pRecord record to print for.
			 * @param args the elements in the record.
			 * @param arrayArgs the array elements in the record.
			 * @param isForWrapAcross true if this is an analysis being executed for a wrap across function.
			 * @param gotAReduction true if some reduction applies to this record
			 * @return true iff successful.
			 */
			bool printExecAnalysis (
					std::ostream &out,
					bool isForForwardWithReduction,
					SpecificationNode node,
					int uid,
					gti::I_RecordType *pRecord,
					std::list<std::string> args,
					std::list<std::string> arrayArgs,
					bool isForWrapAcross = false,
					bool gotAReduction = false);

			/**
			 * Reads an exec-analysis node and returns the information for
			 * the analysis to execute.
			 * @param node to read.
			 * @param outInfo pointer to storage for returned information on the analysis to execute.
			 * @param pOutSupportsReduction pointer to storage for bool. Is set to true if the exec-analysis
			 *               node specifies the supports-reduction attribute with a value of "yes". I.e.
			 *               there is a reduction mapped to this actions node and it is supported by this
			 *               analysis.
			 * @return true iff successful.
			 */
			bool preReadExecAnalysis (
					SpecificationNode node,
					AnalysisFunction *outInfo,
					bool *pOutSupportsReduction);
		};
	}/*namespace codegen*/
}/*namespace gti*/

#endif /*RECEIVAL_GENERATOR_H*/
