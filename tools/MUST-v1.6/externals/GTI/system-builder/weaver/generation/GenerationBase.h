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
 * @file GenerationBase.h
 * 		@see gti::weaver::generation::GenerationBase
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#ifndef GENERATIONBASE_H
#define GENERATIONBASE_H

#include <string>
#include <vector>
#include <fstream>

#include "Level.h"

using namespace gti::weaver::layout;

namespace gti
{
	namespace weaver
	{
		namespace generation
		{
        		template< typename T >
	        	class pointer_comparator : public std::binary_function< T, T, bool >
	        	{
	        	        public :
	        	        bool operator()( T x, T y ) const { return *x < *y; }
                        };
			/**
			 * Base class for output generators that process
			 * the properties of levels to generate their output.
			 * Inherits all properties from a level and provides
			 * a constructor that copies the properties of a given
			 * level to this class.
			 * Use this class by inheriting from it and do all
			 * generation directly in the constructor of the
			 * inheriting class.
			 *
			 * Provides several XML generation functions for
			 * generating input XML for code generators.
			 */
			class GenerationBase : public Level
			{
			public:
				/**
				 * Initializes the generation base object and copies the
				 * properties of the given level to this object.
				 * Also opens the output file for the given name.
				 * @param level to inherit input for
				 *        generation from.
				 * @param fileName file name to use for writing the
				 *        generated output to.
				 */
				GenerationBase (
						Level *level,
						std::string fileName,
						std::ios_base::openmode mode = std::ios_base::out);

				/**
				 * Destructor.
				 */
				~GenerationBase (void);

			protected:
				std::ofstream out;
				std::map<std::string, bool> myHeaders;
				std::map <Adjacency*, int> myComms;
				std::map <Analysis*, int > myAnalysisMap;

				/**
				 * Writes an XML node of the settings type to the output.
				 * @param sourceFileName source file to use in settings.
				 * @param headerFileName header file to use in settings.
				 * @param logFileName header file to use in settings.
				 */
				bool writeSettings (
						std::string sourceFileName,
						std::string headerFileName,
						std::string logFileName);

				/**
				 * Calculated the headers that are required for this
				 * levels wrapper or receival/forward. Stores them in the
				 * headers attribute.
				 * @param forWrapper true if headers are for wrapper, otherwise
				 *        they are considered to be for receival and forwarding.
				 * @return true if successful.
				 */
				bool calculateHeaders (bool forWrapper);

				/**
				 * Prints all headers in the myHeaders attribute to the
				 * output XML.
				 * @return true if successful.
				 */
				bool printHeaders (void);

				/**
				 * Calculates a map that maps each outgoing communication
				 * of this level to an id. Stored in attribute myComms.
				 * @return true if successful.
				 */
				bool calculateComms (void);

				/**
				 * Prints myComms as XML nodes.
				 * @param includeIntraComm true if intra communication means should be included.
				 * @return true if successful.
				 */
				bool printComms (bool includeIntraComm);

				/**
				 * Calculates a mapping of each analysis on this level
				 * to an id. Stores it in myAnalysisMap.
				 * @return true if successful.
				 */
				bool calculateAnalyses(void);

				/**
				 * Prints the myAnalysisMap attribute as an XML node of the
				 * "analyses" type.
				 * @return true if successful.
				 */
				bool printAnalyses(void);

				/**
				 * Loops over all analyses of the level and prints an exec-analysis
				 * node for each mapping of the analyses to the given call with the
				 * given properties.
				 * @param call call for which analyses must have mapping.
				 * @param integrity true if analysis must be an integrity.
				 * @param order required order of the mapping.
				 * @param isForWrapper set to true if this is an exec analysis for
				 *              wrapper generation input.
				 */
				void printExecAnalyses (
						Call *call,
						bool integrity,
						CalculationOrder order,
						bool isForWrapper);

				/**
				 * Prints the exec-analysis node for a mapping.
				 * @param m mapping to print.
				 * @param analysisId id of the analysis to refer to.
				 * @param supportsReduction true if an reduction is run for the event to which this exec-analysis
				 *              applies, and this analysis supports the reduction. False otherwise.
				 */
				void printMappingExec (Mapping *m, int analysisId, bool supportsReduction);

				/**
				 * Prints a record for the given list of inputs and the given uid.
				 * @param call from which the record comes.
				 * @param uid unique id for the record.
				 * @param inputs inputs to add to the record.
				 */
				void printRecord (Call *call, int uid, std::list<Input*> inputs);

				/**
				 * Prints the contents of a forwarding node.
				 * Prints the content
				 * for the given call with the given order.
				 * @param call for which to print the forwarding.
				 * @param order of the forwarding node.
				 * @param excludeIncomingRecord checks whether for this call and order
				 *        an eqaul record as a record forwarded is used, if so the
				 *        record is not printed again.
				 * @param allowIntraCommunication set to true if this forwarding should
				 *               include intra communication (true for wrapper gen, false for
				 *               receival gen)
				 * @param disableInterCommunication set to true if only intra communication
				 *               shall be generated, use case is for the wrappers of wrap-across calls
				 *               their inter-forwardings must only be executed on the remote (receival)
				 *               side, not in the wrapper.
				 */
				void printForwardingForCall (
				        Call *call,
				        CalculationOrder order,
				        bool excludeIncomingRecord,
				        bool allowIntraCommunication,
				        bool disableInterCommunication);
			};

		}
	}
}

#endif /*GENERATIONBASE */
