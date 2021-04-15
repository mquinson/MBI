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
 * @file GeneratorBase.h
 *		@see gti::codegen::GeneratorBase
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#include <fstream>
#include <map>
#include <string>
#include <list>

#include "SpecificationNode.h"

#include "I_RecordGenerator.h"

#ifndef GENERATOR_BASE_H
#define GENERATOR_BASE_H

using namespace gti::weaver;

namespace gti
{
	namespace codegen
	{

		/**
		 * Information on a module.
		 */
		struct ModuleInfo
		{
			std::string name, /**< Registered name of module. */
						datatype; /**< Function name of the module. */
			bool isReduction; /**< True if this analysis module is a reduction .*/
			int index; /**< Index of this module in the analysis array used to keep analysis modules. */
			bool listensToTimeouts; /**< True if this is a module that is not a reduction but is also interested in timeouts.*/
			bool continuous; /**< True if this module needs to be triggered continuously.*/
		};

		/**
		 * Information on an analysis function.
		 */
		struct AnalysisFunction
		{
			std::string function; /**< Name of the analysis function. */
			bool needsChannelId; /**< True if this function needs an extra channel id argument.*/
			ModuleInfo *info; /**< Pointer to info for thie module. */
		};

		/**
		 * Base functionalities for code generation from a wrapper
		 * or receival/forward specification.
		 *
		 * An actual generator should inherit from this class and
		 * perform the generation directly in its constructor.
		 */
		class GeneratorBase
		{
		public:
			/**
			 * Constructor.
			 */
			GeneratorBase (void);

			/**
			 * Destructor.
			 */
			~GeneratorBase (void);

		protected:

			bool myProfiling;

			std::string myOutDir,
						mySourceName,
						myHeaderName,
						myLogName,
						myClassName;
			std::list<std::string> myRecordHeaders;

			std::ofstream 	mySourceOut,
							myHeaderOut,
							myLogOut;

			std::map <int, std::string> myCommMods; //maps id of comm module to its name
			std::map <int, std::string> myIntraCommMods; //maps id of intra comm module to its name, should only be one intra communication module
			std::map <int, std::string> myDownCommMods; //maps id of intra comm module to its name, should only be one downwards communication module
			std::map <std::string, AnalysisFunction> myAnalyses; //maps textual id of analyses to information on their analysis function and module
			std::list<ModuleInfo*> myAnalysisMods;

			gti::I_RecordGenerator *myImpl;
			xmlDocPtr myDocument;

			/**
			 * Returns the name of the implementing generator.
			 * @return generator name.
			 */
			virtual std::string myGetGeneratorName (void) = 0;

			/**
			 * Returns the name of the root node for the
			 * type of XML specification used by the
			 * generator.
			 * @return root node name.
			 */
			virtual std::string myGetRootNodeName (void) = 0;


			/**
			 * Returns the name to use as record name when accessing
			 * an argument of a record.
			 * @return record name.
			 */
			virtual std::string myGetRecordName (void) = 0;

			/**
			 * Opens the input XML file with the given name
			 * and returns the root node. If an error occurs
			 * NULL is returned instead.
			 * @param inputFile name of the input XML.
			 * @return root node or NULL if failure.
			 */
			SpecificationNode openInput (std::string inputFile);

			/**
			 * Returns an instance of a record generator.
			 * @param ppImpl out storage for pointer to implementation.
			 * @return true if successful, false otherwise.
			 */
			bool getRecordGenerationImplementation(gti::I_RecordGenerator **ppImpl);

			/**
			 * Frees the record generation implementation.
			 * @return true if successful, false otherwise.
			 */
			bool freeRecordGenerationImplementation(void);

			/**
			 * Reads the contents of the settings node in the
			 * XML input and initializes the output file names
			 * and streams. Must be called before reading any
			 * of the other parts of the XML file.
			 *
			 * @param node the settings node.
			 * @return true if successful, false otherwise.
			 */
			bool readSettings (SpecificationNode node);

			/**
			 * Reads the "headers" node of the wrapper generator
			 * input XML. Immediately adds the respective includes
			 * to the source/header file.
			 * @param node to read.
			 * @return true if successful, false otherwise.
			 */
			bool readAndPrintHeaders (SpecificationNode node);

			/**
			 * Reads the "communications" node in the wrapper
			 * generator input XML.
			 * @param node to read
			 * @return true if successful, false otherwise.
			 */
			bool readCommunications (SpecificationNode node);

			/**
			 * Reads the "analyses" node in the wrapper
			 * generator input XML.
			 * @param node to read.
			 * @return true if successful, false otherwise.
			 */
			bool readAnalyses (SpecificationNode node);

			/**
			 * Reads a "record" node and creates its
			 * corresponding I_RecordType implementation.
			 * @param node the record node.
			 * @param pOutUid pointer to storage for the uid
			 *                of the read record.
			 * @param pOutRecordp pointer to storage for
			 *        pointer to record implementation, used
			 *        to store the created record, needs to
			 *        be delete by the caller.
			 * @param args pointer to list of strings, used
			 *        to store the non array element names
			 *        of the record.
			 * @param arrayArgs pointer to list of strings,
			 *        used to store the array element names
			 *        of the record.
			 * @return true iff successful.
			 */
			bool readRecord (
					SpecificationNode node,
					int *pOutUid,
					gti::I_RecordType **pOutRecord,
					std::list<std::string> *args,
					std::list<std::string> *arrayArgs);

			/**
			 * Reads a record node from the XML input for
			 * the wrapper generator. Prints the information
			 * on the record to the source file using the
			 * record generation interface.
			 * @param node the node to process.
			 * @param pOutFreeCode output: pointer to string, used to store the
			 *        code that frees the record.
			 * @param pOutUid output: pointer to storage of type int, used to store uid
			 *        of the record in.
			 * @param pExistingRecord optional: record to initialize new record from.
			 * @param existingArgs list of arguments in the optional record.
			 * @param existingArgs list of array arguments in the optional record.
			 * @return true if successful, false otherwise.
			 */
			bool printRecord (
					SpecificationNode node,
					int *pOutUid,
					std::string *pOutFreeCode,
					std::ostream& out,
					gti::I_RecordType *pExistingRecord = NULL,
					std::list<std::string> existingArgs = std::list<std::string> (),
					std::list<std::string> existingArrayArgs = std::list<std::string> ()
					);

			/**
			 * Processes and prints a forwarding node to
			 * the source.
			 * @param node the forwarding node.
			 * @param moduleAccessCode piece of code to add to access used communication modules.
			 * @param avoidUid set to true if you want to specify a uid for which no record and forwarding should be generated (e.g. because you already have a serialized record for it).
			 * @param uidToAvoid uid to avoid, only considered if avoidUid is true.
			 * @param pOutAvoidedCommIds  list of commIds to which the avoided uid would have been forwarded, only used if avoidUid is true.
			 * @param pRecord optional: record to initialize new records from.
			 * @param args list of arguments in the optional record.
			 * @param arrayArgs list of array arguments in the optional record.
			 * @param avoidReducibleVar name of a variable that holds a bool value in the generated code. It is used to produce code were all records and forwards that are
			 *              reducible can not be created/executed if this variable is true. If set to "" no such ifs will be created.
			 * @return true if successful, false otherwise.
			 */
			bool printForwarding (
					SpecificationNode node,
					std::string moduleAccessCode,
					std::ostream& out,
					bool avoidUid = false,
					int uidToAvoid = 0,
					std::list<int> *pOutAvoidedCommIds = NULL,
					gti::I_RecordType *pRecord = NULL,
					std::list<std::string> args = std::list<std::string> (),
					std::list<std::string> arrayArgs = std::list<std::string> (),
					std::string avoidReducibleVar = ""
					);

			/**
			 * Returns the code used to return a named argument
			 * from a given record.
			 * @param argName name of the argument to return.
			 * @param pRecord the record in which the argument is.
			 * @param args list of non-array elements in the record.
			 * @param arrayArgs list of array elements in the record.
			 * @param pOutCode pointer to storage for the code that
			 *                 is used to return the argument.
			 * @return true iff successful.
			 */
			bool getArgumentAccessCode (
					std::string argName,
					gti::I_RecordType *pRecord,
					std::list<std::string> args,
					std::list<std::string> arrayArgs,
					std::string *pOutCode);

			/**
			 * Returns the name of the profiling variable used for the
			 * given analysis function. The variable is an uint64_t and
			 * counts microseconds spend for this analysis function.
			 */
			std::string getProfilingVariableName (AnalysisFunction* fn);

			/**
			 * Returns the name of the profiling variable used for the
			 * given analysis function. The variable holds the invocation
			 * count and is of type uint64_t.
			 */
			std::string getProfilingCountVariableName (AnalysisFunction* fn);
		};
	}
}

#endif /*GENERATOR_BASE_H*/
