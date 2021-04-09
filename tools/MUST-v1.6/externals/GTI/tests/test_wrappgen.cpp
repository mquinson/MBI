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
 * @file test_wrappgen.cpp
 * Simple test case for trace record generation interface implementations.
 * Creates a simple source file and a CMake module to build a shared
 * library that wraps a call of the signature "mycall (int a, float* b)".
 *
 * @author Tobias Hilbrich
 */

#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "I_RecordGenerator.h"

using namespace gti;

int main (int argc, char** argv)
{
    gti::I_RecordGenerator *pImpl;
    std::list<std::string> sources1,
        sources2, libs, headers;
    std::list<std::string>::iterator iter;

    //Get implementation of Record Generator Interface
    /*
     * @todo we need a final solution here, right now we use dlopen/dlsym calls.
     *             In the long term I would prefer to use pnmpi instead, but that always
     *             requires MPI at the moment, thus we would need to call the generators
     *             with "mpirun -np 1 generator.exe" which is not so lucky.
     */

    if (getenv("GTI_RECORD_GEN_IMPL") == NULL)
    {
        std::cerr << "ERROR: you need to specify the environmental GTI_RECORD_GEN_IMPL which should contain a filepath for the record generation library to use." << std::endl;
        return false;
    }

    void* libHandle = 0;
    I_RecordGenerator* (*getImplP) (void);
    libHandle = dlopen (getenv("GTI_RECORD_GEN_IMPL"), RTLD_LAZY);

    if (!libHandle)
    {
        std::cerr << "ERROR: could not find the shared library specified with GTI_RECORD_GEN_IMPL (\"" << getenv("GTI_RECORD_GEN_IMPL") << "\"), check this specification, it must be a filepath to the record generation implementation." << std::endl;
        return false;
    }

    getImplP = (I_RecordGenerator* (*) (void)) dlsym (libHandle, "getImplementation");

    if (!getImplP)
    {
        std::cerr << "ERROR: the library specified with GTI_RECORD_GEN_IMPL (\"" << getenv("GTI_RECORD_GEN_IMPL") << "\"), has no \"getImplementation\" function, check the library implementation." << std::endl;
        return false;
    }

    pImpl = getImplP ();

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
    delete pImpl;

    //Write a CMake file for the generated source file
    //TODO: Missing is the include for the gti Macro file and the import
    //      of the flags, libraries, ... used to compile gti
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
