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
 * @file I_RecordGenerator.h
 *       Interface for generation of source code that deals with trace records.
 *
 * Interface defining functions that are used to create source code that is
 * needed to use a certain type of trace record. Actions are the creation of
 * a new record, assignment of values to the record, reading values from the
 * record, serialization and de-serialization and so on.
 *
 * The individual functions may return source code that is directly used in
 * generated source files that deal with trace records. Further it may generate
 * additional source files that are used to implement functions in the returned
 * code.
 *
 * @ref RecordGenerator - Learn more about the Record Generator
 *
 * @author Tobias Hilbrich
 * @date 06.12.2009
 */

/**
 * @page RecordGenerator Trace Record Generation Interface
 *
 * This page introduces the design of the Trace Record
 * Generation Interface and is split into the following parts:
 * - @ref RecordGeneratorOverview -- General introduction to the interface
 * - @ref RecordGeneratorGti -- The interface in the scope of MUST
 * - @ref RecordGeneratorObtainingInterface -- How to get an instance of an implementation of the interface
 * - @ref RecordGeneratorUsage -- How to use the interface
 * - @ref RecordGeneratorExample -- A usage example
 *
 * @section RecordGeneratorOverview Overview
 *
 * This interface is used while generating source code in a
 * trace record independent manner. This idea comes from MUST,
 * which is designed to be independent from the trace record that
 * is used to store and transfer event data. Obviously some parts
 * of the code have to utilize the trace record and need to know how
 * to use it. One solution would be the generation of
 * trace record specific code directly within each code
 * generator. This comes at the disadvantage of needing a record generation
 * implementation for each generator.
 * As a result, the generation of trace record usage code would be tightly
 * coupled to the individual code generators. Ultimately, it will be hard
 * to replace the trace record with such a design, which has disadvantages
 * for tool interoperability.
 *
 \dot
   digraph GeneratorInterfaceOverview {
    subgraph cluster0 {
         Inter [label="Trace Record\nGeneration Interface", shape=box, fillcolor=lightgrey, style=filled];
         color=black;
         label="Code Generator\n(Executable)";
         }

   Impl [label="Interface\nImplementation\n(Shared Library)", shape=record, fillcolor=lightgrey, style=filled];
   Inter->Impl;
   }
 \enddot
 *
 * To overcome these disadvantages we propose the usage of a
 * "Trace Record Generation Interface" that is used by code
 * generators that need to create trace record aware code. This
 * interface is used to create trace record specific code snippets,
 * whereas the generator itself creates the remaining parts of
 * the code. E.g. the generator creates a wrapper function for
 * MPI_Send, whereas the interface is used to create a record
 * for the MPI_Send call and to store "comm" and "tag" of the
 * MPI_Send call in the record. Different implementations of the
 * interface may provide different types of records. The graph
 * above shows the relationship of a code generator, the
 * interface, and its implementation. Each type
 * of record (each interface implementation) can have its own
 * advantages and disadvantages.
 * Common properties are record creation overhead, size of the
 * representation that is used to communicate the record over
 * the wire (serialized record), and so on.
 *
 * @section RecordGeneratorGti Record Generation Interface in the Scope of MUST
 * For MUST there exist 3 types of activities that touch trace records:
 * - Wrapper -- Intercepts calls and creates the initial versions of the trace records
 * - Receival -- Receives trace records (from trace communication network), de-serializes them, and passes the trace data to the analyses
 * - Forward -- Sends trace records to different places (threads/processes)
 *
 * With that design, the code generation in MUST uses the following structure:
 * \dot
   digraph GeneratorInterfaceUsage {
     Config [label="MUST Configuration", shape=box, fillcolor=grey, style=filled];
     SysBuilder [label="System Builder", shape=box, fillcolor=orange, style=filled];
     RGenInter [label="Record Gen. Interface", shape=box, fillcolor=green, style=filled];
     RGenImpl [label="Record Gen. Implementation", shape=box, fillcolor=orange, style=filled];
     Wrapper [label="{Wrapper | <c0>Trace Usage}", shape=record, fillcolor=blue, style=filled];
     Receival [label="{Receival | <c1>Trace Usage}", shape=record, fillcolor=blue, style=filled];
     Forward [label="{Forward | <c2>Trace Usage}", shape=record, fillcolor=blue, style=filled];
     Config -> SysBuilder [label="", weight=1];
     SysBuilder -> Wrapper [label="gen", weight=1];
     SysBuilder -> Receival [label="gen", weight=1];
     SysBuilder -> Forward [label="gen", weight=1];
     SysBuilder -> RGenInter [label="uses", weight=1];
     RGenInter -> RGenImpl [label="implemented by", weight=1];
     RGenImpl -> Wrapper:c0 [label="gen", weight=1,style=dashed];
     RGenImpl -> Receival:c1 [label="gen", weight=1,style=dashed];
     RGenImpl -> Forward:c2 [label="gen", weight=1,style=dashed];
  }
  \enddot
 * The System Builder component of GTI will generate the above mentioned 4 types of activities that
 * use trace records. However, all trace record specific parts of the generated code are returned by
 * an implementation of the record generation interface.
 *
 *
 * @section RecordGeneratorObtainingInterface Obtaining the Top-level Interface
 * This section will describe how a code generator obtains an implementation of the top level
 * record generator interface(see below). Different options for that use case exist:
 * - Linking the code generator to a static library
 * - Usage of a shared library
 * - Usage of a P^nMPI module
 *
 * The last option is the preferred one, but currently not simple enough, due to P^nMPI limitations.
 * This will likely change with upcoming versions of P^nMPI. At that point this section will
 * be extended.
 *
 * Update 2012: we had a PnMPI hack here before, we moved this to a pure shared library
 *                       based version and now use calls to dlopen and dlsym. The base principle
 *                       here is that the record generation implementation is a dynamic loadable
 *                       module (library) that provides a function of the following signature:
 *                       extern "C" I_RecordGenerator* getImplementation (void)
 *                       We use this function to provide the inmplementation of the record
 *                       generation interface to the library user.
 *
 * @todo needs to be updated according to future P^nMPI development.
 *
 * @section RecordGeneratorUsage Usage
 * The Record Generation Interface provides three different interfaces:
 * - gti::I_RecordGenerator -- the top level interface
 * - gti::I_RecordDescription -- description of a trace record
 * - gti::I_RecordType -- provides code generation for an actual trace record
 *
 * Usage of these interfaces works in two phases:
 * - Initialization phase
 * - Usage phase
 *
 * @subsection RecordGeneratorUsageInitPhase Initialization Phase
 * The initialization phase is used to query the implementation of the generator
 * for libraries, header files, and source files that are needed when using the
 * generated record code.
 * It consists of the following series of calls:
 * @code
gti::I_RecordGenerator *pImpl;
//TODO: Get implementation of Record Generator Interface (depends on final solution above)
pImpl->initBegin (
	std::string targetDirectory,
    std::string prefix);
pImpl->initGetIntermediateSourceFileNames (
    std::list<std::string> *pOutSourceNames);
pImpl->initGetStaticSourceFileNames (std::list<std::string> *pOutSourceNames);
pImpl->initGetLibNames (std::list<std::string> *pOutLibNames);
pImpl->initGetHeaderFileNames (std::list<std::string> *pOutHeaderNames);
pImpl->initEnd ();
 @endcode
 * The "Get" methods must only be called in between the "Begin" and the "End" call.
 * All of them need to be called, while the order of the calls is of no importance.
 * The return values of gti::I_RecordGenerator::initGetIntermediateSourceFileNames and
 * gti::I_RecordGenerator::initGetStaticSourceFileNames have to be used as extra source
 * files during compilation of the generated code.
 * The values returned from gti::I_RecordGenerator::initGetLibNames need to be used
 * as extra library dependencies
 * during linking of the
 * generated code. Finally the headers returned with
 * gti::I_RecordGenerator::initGetHeaderFileNames need
 * to be included into the generated code.
 *
 * Rational: These libs, includes, and source file allow the implementation
 *           of the trace record to use generated code itself.
 *
 * @subsection RecordGeneratorUsageUsePhase Usage Phase
 * After the initialization phase, the top level interface is used to create
 * descriptions for trace records (gti::I_RecordDescription). These
 * descriptions are used to specify
 * the layout of an individual record. Once the description is complete,
 * this interface is used to get an interface for the actual record
 * (gti::I_RecordType). This interface provides various calls to create
 * code for the usage of the trace record.
 *
 * @subsubsection RecordGeneratorUsageRecordInstances Instances of a Record
 * The interface for a record (gti::I_RecordType) provides multiple
 * calls to create code that captures an instance of the record.
 * One can either create an instance of a record or a pointer to an
 * instance of a record. It is not allowed to copy an instance of a
 * record! If you need to store a record or pass it to functions you
 * must pass a pointer to the record not the record itself. Use
 * the gti::I_RecordType::getPointerType function to query the
 * datatype name that has to be used for pointers (e.g. to create
 * a function declaration/definition).
 * The chart below shows in what order the code snippets returned by
 * the record interface may be used, and what the respective state
 * of the trace record will be:
 \dot
   digraph RecordUsageFlow {
     Start1 [label="", shape=circle, fillcolor=grey, style=filled];
     Start2 [label="", shape=circle, fillcolor=grey, style=filled];
     PToInstance [label="{Pointer to Instance}", shape=record, fillcolor=grey, style=filled];
     AllocInstance [label="{Allocated Instance}", shape=record, fillcolor=grey, style=filled];

     Start1->PToInstance [label=" createInstancePointer"];
     Start2->AllocInstance [label=" createInstance"];
     PToInstance->AllocInstance [label="allocInstance"];

     InitedInstance [label="{Inizialized Instance|Can be used with:\nwriteArgument()\n...\nreturnArgument()\n...\nserialize()}", shape=record, fillcolor=grey, style=filled];

     AllocInstance->InitedInstance [label=" initInstance"];
     AllocInstance->InitedInstance [label=" deserialize"];

     FreedInstance [label="{Freed up Instance}", shape=record, fillcolor=grey, style=filled];

     InitedInstance->FreedInstance [label=" freeInstance"];

	 FreedInstance->PToInstance [label="deallocInstance\n(for Pointer\n to instance only)"];

	 { rank = same; "Start1"; "Start2"}
  }
  \enddot
 * (The arc labels name the functions that generate the code that is used to get from one
 * state to another, the labels in the boxes name the different states of a record.)
 *
 * @subsubsection RecordGeneratorUsageArrays Arrays
 * Array arguments can be used to store entries with varying size in
 * a record. Each array argument has one so called "lengthArgument"
 * which is another argument in the record that specifies the size
 * of the array. Rules for array argument creation and usage are:
 * - Before defining an array entry its lengthArgument has to be defined
 * - Before writing to an array argument its lengthArgument has to be written
 * - When writing the lengthArgument of an array argument the content of the
 *   array argument will be lost
 * - Before reading from an array it has to be validly written once or
 *   deserialized from a record were it was once validly written
 *
 * @section RecordGeneratorExample Example
 * The following code uses the Trace Record Generation Interface to build
 * a wrapper function for the call "myCall (int myArg1, float *myArg2)".
 * It outputs a source file for the wrapper and also provides a CMake file
 * to build a P^nMPI module from this source file:
 *
 @code
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "I_RecordGenerator.h"

int main (int argc, char** argv)
{
    gti::I_RecordGenerator *pImpl;
    std::list<std::string> sources1,
        sources2, libs, headers;
    std::list<std::string>::iterator iter;

    //Get implementation of Record Generator Interface
    //TODO needs to be refined!

    //Initialize Record Generator
    pImpl->initBegin ("/tmp", "gti-intermediate-code");
    pImpl->initGetIntermediateSourceFileNames (&sources1);
    pImpl->initGetStaticSourceFileNames (&sources2);
    pImpl->initGetLibNames (&libs);
    pImpl->initGetHeaderFileNames (&headers);
    pImpl->initEnd ();

    //Create a record
    gti::I_RecordDescription *pDesc;
    gti::I_RecordType *pRecord;
    pImpl->createRecordDescription (&pDesc);
    pDesc->addArgument ("arg1", "int", "myCall", "arg1");
    pDesc->addArrayArgument (
		"arg2", "float", "myCall", "arg2", "arg1");
    pDesc->createRecord (1234, &pRecord);

    //Get source code for some basic record usage
    std::string
        definitionCode,
        definitionCode2,
        freeCode,
        freeCode2,
        initCode,
        setArg1Code,
        setArg2Code,
        serializeCode,
        deserializeCode,
        returnArg1Code,
        returnArg2Code,
        getUidCode;
    pRecord->createInstance ("myRecord", &definitionCode);
    pRecord->initInstance ("myRecord", &initCode);
    pRecord->freeInstance ("myRecord", &freeCode);
    pRecord->writeArgument ("myRecord", "arg1", "myArg1", &setArg1Code);
    pRecord->writeArrayArgument ("myRecord", "arg2", "myArg2", &setArg2Code);
    pRecord->serialize ("myRecord", "serBuf", "serLen", &serializeCode);
    pRecord->createInstance ("myRecord2", &definitionCode2);
    pRecord->freeInstance ("myRecord", &freeCode2);
    pRecord->deserialize ("serBuf", "serLen", "myRecord2", &deserializeCode);
    pRecord->returnArgument ("myRecord2", "arg1", &returnArg1Code);
    pRecord->returnArrayArgument ("myRecord2", "arg2", &returnArg2Code);
    pImpl->returnUidFromSerialized ("serBuf", &getUidCode); //Reads Uid (1234) from serialized record.

    //Create a tiny source file that uses the record
    std::ofstream out ("test.cc");

    out << "#include <stdio.h>" << std::endl;
    out << "#include <iostream>" << std::endl;

    for (iter = headers.begin(); iter != headers.end (); iter++)
        out << "#include <" << *iter << ">" << std::endl;

    out << std::endl
        << "extern \"C\" int myCall (int myArg1, float *myArg2)" << std::endl
        << "{" << std::endl
        << "    void *serBuf = NULL;" << std::endl
        << "    uint64_t serLen;" << std::endl
        << "    " << definitionCode << ";" << std::endl
        << "    " << definitionCode2 << ";" << std::endl
        << std::endl
        << "    " << initCode << std::endl
        << "    " << setArg1Code << std::endl
        << "    " << setArg2Code << std::endl
        << "    " << serializeCode << std::endl
        << std::endl
        << "    if (" << getUidCode << " == 1234)" << std::endl
        << "    {" << std::endl
        << "        " << deserializeCode << std::endl
        << std::endl
        << "        std::cout << \"Wrapped myCall (\" << myArg1 << \", myArg2={\";" << std::endl
        << "        for (int i = 0; i < myArg1; i++)" << std::endl
        << "            std::cout << myArg2[i] << \"; \";" << std::endl
        << "        std::cout << \"})\" << std::endl;" << std::endl
        << "        std::cout << \"myRecord2 has: myArg1=\" <<"
            << returnArg1Code
            << "<< \" myArg2={\";" << std::endl
        << "        for (int i = 0; i < myArg1; i++)" << std::endl
        << "            std::cout << " << returnArg2Code << "[i] << \"; \";" << std::endl
        << "        std::cout << \"}\" << std::endl;" << std::endl
        << "    }" << std::endl
        << std::endl
        << "    free (serBuf);" << std::endl
        << "    " << freeCode << std::endl
        << "    " << freeCode2 << std::endl
        << "    return 0;" << std::endl
        << "}" << std::endl;

    //Clean up
    pRecord->deleteObject ();
    pDesc->deleteObject ();
    //TODO free instance of main interface (needs to be refined)

    //Write a CMake file for the generated source file
    //TODO: Missing is the include for the GTI Macro file and the import
    //      of the flags, libraries, ... used to compile GTI
    std::ofstream make ("CMakeLists.txt");
    make
        << "CMAKE_MINIMUM_REQUIRED(VERSION 2.6)" << std::endl
        << "PROJECT (MyCallWrapper C CXX)" << std::endl
        << "SET (SOURCES test.cc";

    for (iter = sources1.begin(); iter != sources1.end (); iter++)
        make << " " << *iter;
    for (iter = sources2.begin(); iter != sources2.end (); iter++)
        make << " " << *iter;

    make
        << ")" << std::endl
        << "ADD_LIBRARY (myCallWrapper MODULE ${SOURCES})" << std::endl
        << "SET_TARGET_PROPERTIES (myCallWrapper PROPERTIES" << std::endl
        << "    VERSION 1.0.0 SOVERSION 1" << std::endl
        << "    )" << std::endl
        << "TARGET_LINK_LIBRARIES (myCallWrapper ";

    for (iter = libs.begin(); iter != libs.end (); iter++)
        make << " " << *iter;

    make
        << ")" << std::endl
        << "INSTALL(TARGETS myCallWrapper LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/lib)" << std::endl
        << "#Optional#GTI_MAC_GET_TARGET_MODULE_NAME (name myCallWrapper 1.0.0)" << std::endl
        << "#Optional#GTI_MAC_ADD_PNMPI_PATCH (${name} ${PNMPI_PATCHER})" << std::endl;

    //close files
    out.close ();
    make.close ();

    return 0;
}
 @endcode
 *
 * Given an artificial and very simple trace record, the output of this application
 * could look like:
 @code
#include <stdio.h>
#include <iostream>
#include </tmp/gti-intermediate-codeGenRecord.h>

extern "C" int myCall (int myArg1, float *myArg2)
{
    void *serBuf = NULL;
    uint64_t serLen;
    GtiRecord_1234 myRecord;
    GtiRecord_1234 myRecord2;

    myRecord.arg1 = 0;
myRecord.arg2 = NULL;

    myRecord.arg1 = myArg1;
if (myRecord.arg2)
    free (myRecord.arg2);
if (myRecord.arg1)
    myRecord.arg2 = (float*) malloc (sizeof(float)*myRecord.arg1);
else
    myRecord.arg2 = NULL;

    for (uint64_t i = 0; i < myRecord.arg1; i++)
    myRecord.arg2[i] = myArg2[i];

    GtiRecordSerialize_1234(&myRecord, &serBuf, &serLen);


    if (((uint64_t*)serBuf)[0] == 1234)
    {
        GtiRecordDeserialize_1234(&myRecord2, serBuf, serLen);


        std::cout << "Wrapped myCall (" << myArg1 << ", myArg2={";
        for (int i = 0; i < myArg1; i++)
            std::cout << myArg2[i] << "; ";
        std::cout << "})" << std::endl;
        std::cout << "myRecord2 has: myArg1=" <<myRecord2.arg1<< " myArg2={";
        for (int i = 0; i < myArg1; i++)
            std::cout << myRecord2.arg2[i] << "; ";
        std::cout << "}" << std::endl;
    }

    free (serBuf);
    if (myRecord.arg2)
    free (myRecord.arg2);
myRecord.arg2= NULL;

    if (myRecord.arg2)
    free (myRecord.arg2);
myRecord.arg2= NULL;

    return 0;
}
 @endcode
 *
 * A simple test program to test this wrapper is given below:
 @code
#include <stdio.h>

extern int myCall (int myArg1, float *myArg2);

int main (int argc, char ** argv)
{
    float f1[] = {3.0};
    float f2[] = {1.0, 2.0, 4.0};

    myCall (1, f1);
    myCall (3, f2);

    return 0;
}
 @endcode
 *
 * When runing this test with the generated wrapper, the following output
 * should result:
 @code
$ ./test.exe
Wrapped myCall (1, myArg2={3; })
myRecord2 has: myArg1=1 myArg2={3; }
Wrapped myCall (3, myArg2={1; 2; 4; })
myRecord2 has: myArg1=3 myArg2={1; 2; 4; }
 @endcode
 *
 */

#include <list>

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#ifndef I_RECORD_GENERATOR_H
#define I_RECORD_GENERATOR_H

namespace gti
{
	/*Forward definition to allow declaration in order of importance.*/
	class I_RecordDescription;
	class I_RecordType;

    /**
     * Top level interface for record usage generation.
     * First, the generator needs to be initialized by calling all of
     * the init functions. Afterwards it is used to create descriptions
     * for trace records.
     *
     * @ref RecordGenerator - Learn more about the Record Generator
     *
     * @see gti::I_RecordDescription
     */
    class I_RecordGenerator
    {
    public:

    	/**
    	 * Virtual destructor.
    	 */
    	virtual ~I_RecordGenerator () {}

        /**
         * Starts the init phase of the record generator.
         * All the other init functions have to be called before
         * finalizing the init phase with a call to initEnd.
         *
         * @param targetDirectory specifies the directory in which to create intermediate source files.
         * @param prefix string that is prepended to all source file names that are returned to avoid name collisions, prefix is already part of each returned name.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN initBegin (
        		std::string targetDirectory,
                std::string prefix) = 0;

        /**
         * Returns names for generated source files of the record generator.
         *
         * @param pOutSourceNames pointer to an empty list, used to store the output names.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Rational: Use case is during building of the code which uses the record generator.
         */
        virtual GTI_RETURN initGetIntermediateSourceFileNames (
                std::list<std::string> *pOutSourceNames
                ) = 0;

        /**
         * Returns names for static (non-generated) source files of the record generator.
         * Must return full and absolute paths.
         *
         * @param pOutSourceNames pointer to an empty list, used to store the output names.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Rational: Use case is during building of the code which uses the record generator.
         */
        virtual GTI_RETURN initGetStaticSourceFileNames (std::list<std::string> *pOutSourceNames) = 0;

        /**
         * Returns names of libraries that are used by/for the trace record.
         * Must return absolute paths to libraries, if they are not in any
         * default paths. E.g.: /usr/local/lib/otf/libotf.so.
         *
         * @param pOutLibNames pointer to an empty list, used to store the output names.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Rational: Use case is during building of the code which uses the record generator.
         */
        virtual GTI_RETURN initGetLibNames (std::list<std::string> *pOutLibNames) = 0;

        /**
         * Returns names of header files that need to be included in order to use the record.
         * Must return absolute paths if headers are not in default search paths for headers.
         *
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN initGetHeaderFileNames (std::list<std::string> *pOutHeaderNames) = 0;

        /**
         * Ends the init phase.
         * Must only be called after initBegin was called, all information about libraries
         * headers, source files and so on has to be querried before calling this function.
         *
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN initEnd () = 0;

        /**
         * Returns the uid (unique ID) that is associated to the record which created
         * this serialized string.
         * Returned value from this code must be an "uint64_t" value.
         * The code must be suited to be put into a function as argument or to
         * set a variable with it. E.g.: uint64_t x = [*pOutCode];.
         * Important: For implementors this requires all serializations of all
         *            records to store the Uid at a fixed position in the
         *            serialized string
         *
         * @param bufName name of the variable that holds the serialized record.
         * @param pOutCode the code that is used to return the value.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN returnUidFromSerialized (std::string bufName, std::string *pOutCode) = 0;

        /**
         * Creates an empty description for a record.
         *
         * @param pOutDesc out: interface for the record description.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         * @see gti::I_RecordDescription
         * @see gti::I_RecordType
         */
        virtual GTI_RETURN createRecordDescription (I_RecordDescription **pOutDesc) = 0;
    };

    /**
     * Describes the arguments of a record.
     * The record starts as an empty record and may
     * afterwards be used to create a new record type.
     *
     * @see I_RecordType
     */
    class I_RecordDescription
    {
    public:

    	/**
    	 * Virtual destructor.
    	 */
    	virtual ~I_RecordDescription () {}

        /**
         * Adds an argument to the record.
         *
         * @param name argument name in record, required to be unique for within the record.
         * @param type type of the argument, must be used to write the value and to return
         *             the value, internal storage may use a different type.
         * @param fromCall function name of the function that generates this record.
         * @param asArg argument name in the function that generates this record.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Rational: fromCall and asArg may be used to hint the implementation of the
         *           trace Record. E.g. a MPI specific trace record may use a different
         *           representation of the record if it knows that it comes from a
         *           specific MPI call.
         */
        virtual GTI_RETURN addArgument (
                std::string name,
                std::string type,
                std::string fromCall,
                std::string asArg) = 0;

        /**
         * Adds an argument to the record that is an array of the specified type.
         *
         * @param name argument name in record, required to be unique within the record.
         * @param type type of each array entry, must be used to write the array
         * 	           and to return the arrays or elements of it, internal storage
         *             may use a different type.
         * @param fromCall function name of the function that generates this record.
         * @param asArg argument name in the function that generates this record.
         * @param lengthArgument an already specified argument in the record that
         *                       specifies the length of the new argument, must be of
         *                       type "uint64_t".
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * @see gti::I_RecordDescription::addArgument
         */
        virtual GTI_RETURN addArrayArgument (
        		std::string name,
        		std::string type,
        		std::string fromCall,
        		std::string asArg,
        		std::string lengthArgument) = 0;

        /**
         * Creates a new record from this description.
         *
         * @param uid a (system globally) unique id for this type of record.
         * @param pOutRecord interface for the new record.
         * Rational: The uid is used to identify a record after it was serialized and
         *           passed to another process/thread which needs to identify the
         *           types of the records that it receives.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN createRecord (uint64_t uid, I_RecordType **pOutRecord) = 0;

        /**
         * Frees this description.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN deleteObject (void) = 0;
    };

    /**
     * A record of a certain type (which was specified during
     * its creation). This interface provides all the
     * functionality that is needed to use the record and
     * create the source code that is needed for its usage.
     */
    class I_RecordType
    {
    public:

    	/**
    	 * Virtual destructor.
    	 */
    	virtual ~I_RecordType () {}

        /**
         * Creates a definition for a variable that captures an instance of Record.
         * Is not ended by a semicolon, can be used in function declarations too.
         *
         * @param varName variable name of the new instance of record.
         * @param pOutCode the code used to create the definition for [varName].
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN createInstance (std::string varName, std::string *pOutCode) = 0;

        /**
         * Creates a definition for a pointer to an instance of the Record.
         * Is not ended by a semicolon, can be used in function declarations too.
         * Use a call to gti::I_RecordType::allocInstance to allocate memory
         * for an instance. Afterwards the instance still needs to be initialized!
         * For all calls except gti::I_RecordType::allocInstance and
         * gti::I_RecordType::deallocInstance you need to dereference the pointer
         * created in varName, e.g.: use "(*[varName])" for these calls.
         *
         * @param varName variable name of the pointer to an instance of record.
         * @param pOutCode the code used to create the definition for [varName].
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN createInstancePointer (std::string varName, std::string *pOutCode) = 0;

        /**
         * Get pointer type.
         * Gets the type of a record that has to be used for pointers to an instance.
         * Use case is to declare functions and containers that store pointers to
         * record instances records.
         *
         * @param pOutCode the code for the pointer type.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN getPointerType (std::string *pOutCode) = 0;

        /**
         * Allocates memory for an instance of [record], [varName] has to be created with
         * a call to createInstancePointer beforehand.
         *
         * @param varName variable name of the pointer to an instance of the record.
         * @param pOutCode the source code that initialized the instance.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN allocInstance (std::string varName, std::string *pOutCode) = 0;

        /**
         * Frees memory for an instance of [record], [varName] has to be created with
         * a call to createInstancePointer beforehand.
         *
         * @param varName variable name of the pointer to an instance of the record.
         * @param pOutCode the source code that initialized the instance.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN deallocInstance (std::string varName, std::string *pOutCode) = 0;

        /**
         * Initializes an instance of [record], has to be created with CreateDefintion
         * beforehand.
         *
         * @param varName variable name of the instance.
         * @param pOutCode the source code that initialized the instance.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN initInstance (std::string varName, std::string *pOutCode) = 0;

        /**
         * Deletes an instance of [record], has to be created and initialized
         * beforehand.
         *
         * @param varName variable name of the instance.
         * @param pOutCode the source code that initialized the instance.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN freeInstance (std::string varName, std::string *pOutCode) = 0;

        /**
         * Creates code that sets the value of the argument [argument] to [value].
         *
         * @param varName variable name of the instance.
         * @param argument the argument that will be set, has to be in the description.
         * @param value the source code text that provides the value to be set.
         * @param pOutCode the source code that writes  [argument] with [value].
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Info:
         * - [value] has to be of a form in which it can be passed to a function.
         * - [value] has to be of the type that was associated with [argument] in
         *   the description.
         * - if [argument] is an [lengthArgument] for some other argument, this
         *   other argument will be resized to the length specified with this
         *   write operation, all old content of that argument will be lost !
         */
        virtual GTI_RETURN writeArgument (
                std::string varName,
                std::string argument,
                std::string value,
                std::string *pOutCode) = 0;

        /**
         * Creates code that copies the values of the array [valueArray] to the array
         * argument [argument].
         *
         * @param varName variable name of the instance.
         * @param argument the array argument that will be set, has to be in the description.
         * @param valueArray the source code text that holds the value array to be set.
         * @param pOutCode the source code for this action.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Info:
         * - [valueArray] has to be of a form in which it can be passed to a function.
         * - [valueArray] has to be a pointer to the type that was associated with
         *   [argument] in the description.
         * - The [lengthArgument] associated with [argument] must be set before
         *   setting [argument].
         */
        virtual GTI_RETURN writeArrayArgument (
        		std::string varName,
        		std::string argument,
        		std::string valueArray,
        		std::string *pOutCode) = 0;

        /**
         * Creates code that writes a single entry of the array argument [argument].
         *
         * @param varName variable name of the instance.
         * @param argument the array argument of which a single element will be set, has to be in the description.
         * @param value the source code text that holds the value array to be set.
         * @param index the source code text that holds the index in the array.
         * @param pOutCode the source code for this action.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Info:
         * - [value] and [index] has to be of a form in which it can be passed to a function.
         * - [value] has to be of the type that was associated with
         *   [argument] in the description.
         * - [index] has to be of type uint64_t
         * - The [lengthArgument] associated with [argument] must be set before
         *   setting [argument].
         */
        virtual GTI_RETURN writeArrayArgumentIndexed (
        		std::string varName,
        		std::string argument,
        		std::string value,
        		std::string index,
        		std::string *pOutCode) = 0;

        /**
         * Creates code that returns the value of the argument [argument].
         * The code must be suited to be put into a function as argument or to
         * set a variable with it. E.g.: [TypeFromDescription] x = [*pOutCode];.
         *
         * @param varName variable name of the instance.
         * @param argument the argument (from the description) to read.
         * @param pOutCode the source code that returns the value of [argument].
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN returnArgument (
                std::string varName,
                std::string argument,
                std::string *pOutCode) = 0;

        /**
         * Creates code that returns a pointer to the type that was associated with
         * the array argument [argument]. The length of the array is specified
         * by the [lengthArgument] that is associated with [argument].
         * The code must be suited to be put into a function as argument or to
         * set a variable with it. E.g.: [TypeFromDescription] *x = [*pOutCode];.
         *
         * @param varName variable name of the instance.
         * @param argument the argument (from the description) to return.
         * @param pOutCode the source code that returns the pointer to the array.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Writing to the returned array is not allowed, the outcome of such an action
         * is undefined.
         */
        virtual GTI_RETURN returnArrayArgument (
        		std::string varName,
        		std::string argument,
        		std::string *pOutCode) = 0;

        /**
         * Creates code that returns an entry from the array argument
         * [argument].
         * The code must be suited to be put into a function as argument or to
         * set a variable with it. E.g.: [TypeFromDescription] x = [*pOutCode];.
         *
         * @param varName variable name of the instance.
         * @param argument the argument from which to return an entry (from the description).
         * @param index the text that specifies the index to the array.
         * @param pOutCode the source code that returns the pointer to the array.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         *
         * Info:
         * - [index] has to be of type uint64_t
         */
        virtual GTI_RETURN returnArrayArgumentIndexed (
        		std::string varName,
        		std::string argument,
        		std::string index,
        		std::string *pOutCode) = 0;

        /** Creates code that serializes [varName].
         * The serialized record is returned in the NULL initialized content
         * of serBufOutName, which has to be the name of a variable of type
         * "void*". The length of the serialized buffer is written to the
         * value stored in numBytesOutName, which has to be of type
         * "uint64_t". May be be implemented by calling a function.
         * pOutSerBuf has to be freed with a call to "free" by the user !
         *
         * @param varName variable name of the instance.
         * @param serBufOutName name of the variable that is going to hold
         *        the serialized buffer.
         * @param numBytesOutName name of the variable that is going go hold
         *        the length of the serialized buffer.
         * @param pOutCallCode the code that performs the serialization.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN serialize (
                std::string varName,
                std::string serBufOutName,
                std::string numBytesOutName,
                std::string *pOutCallCode) = 0;

        /** Creates code that de-serializes [serBufName].
         * The new record is stored with the name [varNameOutRecord], which
         * has to be a defined but not yet initialized record instance.
         * [serBufName] still has to be freed with deleteObject by the user.
         * [varNameOutRecord] needs to be freed by the user.
         *
         * @param serBufName string that contains the name of the variable for the
         *                   serialized buffer, of type "void*".
         * @param serBufLength string that contains the name of the variable that
         *                     specifies the length of the serialized record, of
         *                     type uint64_t.
         * @param varNameOutRecord the record instance whose content will be set
         *                         during de-serialization.
         * @param pOutCallCode the code that performs the de-serialization.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN deserialize (
                std::string serBufName,
                std::string serBufLength,
                std::string varNameOutRecord,
                std::string *pOutCallCode) = 0;

        /**
         * Frees this record.
         * @return GTI_SUCCESS if successful, otherwise another entry from GTI_RETURN.
         */
        virtual GTI_RETURN deleteObject (void) = 0;
    };
}

#endif /*I_RECORD_GENERATOR_H*/
