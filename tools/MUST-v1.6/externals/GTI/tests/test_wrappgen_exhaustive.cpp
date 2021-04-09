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
 * More exhaustive test case for trace record generation interface
 * implementations.
 * Creates a source file and a CMake module to build a shared
 * library.
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

    //======
    //Get implementation of Record Generator Interface
    //======
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

	//======
    //Initialize Record Generator
	//======
    pImpl->initBegin (getenv ("PWD"), "gti-intermediate-code");
    pImpl->initGetIntermediateSourceFileNames (&sources1);
    pImpl->initGetStaticSourceFileNames (&sources2);
    pImpl->initGetLibNames (&libs);
    pImpl->initGetHeaderFileNames (&headers);
    pImpl->initEnd ();

    //======
    //Create a Records
    //======
    gti::I_RecordDescription *pDescA, *pDescB, *pDescC, *pDescD;
    gti::I_RecordType *pRecordA, *pRecordB, *pRecordC, *pRecordD;
    pImpl->createRecordDescription (&pDescA);
    pImpl->createRecordDescription (&pDescB);
    pImpl->createRecordDescription (&pDescC);
    pImpl->createRecordDescription (&pDescD);

    //Record A
    pDescA->addArgument ("x", "int", "callA", "x");
    pDescA->addArgument ("y", "float", "callA", "y");
    pDescA->addArgument ("w_len", "int", "callA", "strlen(w)");
    pDescA->addArrayArgument (
        "z", "int", "callA", "z", "x");
    pDescA->addArrayArgument (
        "w", "char", "callA", "w", "w_len");
    pDescA->createRecord (1, &pRecordA);

    //Record B
    pDescB->addArgument ("len", "int", "callB", "len");
    pDescB->addArrayArgument (
        "x", "int", "callB", "x", "len");
    pDescB->addArrayArgument (
        "y", "int", "callB", "y", "len");
    pDescB->createRecord (2, &pRecordB);

    //Record C
    pDescC->addArgument ("x", "int", "callC", "x");
    pDescC->createRecord (3, &pRecordC);

    //Record D
    pDescD->addArgument ("x", "int", "callD", "x");
    pDescD->createRecord (4, &pRecordD);

    //======
    //Get source code for record usage
    //======
    std::string getUidCode;
    pImpl->returnUidFromSerialized ("serBuf", &getUidCode); //Reads Uid (1234) from serialized record.

    //record A
    std::string
		pointerTypeA,
        definitionCodeA,
        definitionPointerA,
        freeCodeA,
        initCodeA,
        setXCodeA,
        setYCodeA,
        setZCodeA,
        setWLenCodeA,
        setWCodeA,
        serializeCodeA,
        deserializeCodeA,
        returnXCodeA,
        returnYCodeA,
        returnZCodeA,
        returnWCodeA;
    pRecordA->getPointerType(&pointerTypeA);
    pRecordA->createInstance ("myRecordA", &definitionCodeA);
    pRecordA->createInstancePointer ("pRecordA", &definitionPointerA);
    pRecordA->initInstance ("myRecordA", &initCodeA);
    pRecordA->freeInstance ("myRecordA", &freeCodeA);
    pRecordA->writeArgument ("myRecordA", "x", "my_x", &setXCodeA);
    pRecordA->writeArgument ("myRecordA", "y", "my_y", &setYCodeA);
    pRecordA->writeArgument ("myRecordA", "w_len", "strlen(my_w)+1", &setWLenCodeA);
    pRecordA->writeArrayArgument ("myRecordA", "z", "my_z", &setZCodeA);
    pRecordA->writeArrayArgument ("myRecordA", "w", "my_w", &setWCodeA);
    pRecordA->serialize ("myRecordA", "serBuf", "serLen", &serializeCodeA);
    pRecordA->deserialize ("serBuf", "serLen", "myRecordA", &deserializeCodeA);
    pRecordA->returnArgument ("(*pRecordA)", "x", &returnXCodeA);
    pRecordA->returnArgument ("(*pRecordA)", "y", &returnYCodeA);
    pRecordA->returnArrayArgument ("(*pRecordA)", "z", &returnZCodeA);
    pRecordA->returnArrayArgument ("(*pRecordA)", "w", &returnWCodeA);

    //record B
    std::string
		pointerTypeB,
		definitionCodeB,
		definitionPointerB,
		freeCodeB,
		allocPointerCodeB,
		deallocPointerCodeB,
		freePointerCodeB,
		initCodeB,
		setXCodeB,
		setYCodeB,
		setLenCodeB,
		serializeCodeB,
		deserializeCodeB,
		returnXCodeB,
		returnYCodeB,
		returnLenCodeB;
    pRecordB->getPointerType(&pointerTypeB);
    pRecordB->createInstance ("myRecordB", &definitionCodeB);
    pRecordB->createInstancePointer ("pRecordB", &definitionPointerB);
    pRecordB->initInstance ("myRecordB", &initCodeB);
    pRecordB->freeInstance ("myRecordB", &freeCodeB);
    pRecordB->allocInstance("pRecordB", &allocPointerCodeB);
    pRecordB->deallocInstance("pRecordB", &deallocPointerCodeB);
    pRecordB->freeInstance ("(*pRecordB)", &freePointerCodeB);
    pRecordB->writeArrayArgument ("myRecordB", "x", "my_x", &setXCodeB);
    pRecordB->writeArrayArgument ("myRecordB", "y", "my_y", &setYCodeB);
    pRecordB->writeArgument ("myRecordB", "len", "my_len", &setLenCodeB);
    pRecordB->serialize ("myRecordB", "serBuf", "serLen", &serializeCodeB);
    pRecordB->deserialize ("serBuf", "serLen", "(*pRecordB)", &deserializeCodeB);
    pRecordB->returnArrayArgument ("(*pRecordB)", "x", &returnXCodeB);
    pRecordB->returnArrayArgument ("(*pRecordB)", "y", &returnYCodeB);
    pRecordB->returnArgument ("(*pRecordB)", "len", &returnLenCodeB);

    //record C
    std::string
		pointerTypeC,
        definitionCodeC,
        definitionPointerC,
        freeCodeC,
        initCodeC,
        setXCodeC,
        returnXCodeC;
    pRecordC->getPointerType(&pointerTypeC);
    pRecordC->createInstance ("myRecordC", &definitionCodeC);
    pRecordC->createInstancePointer ("pRecordC", &definitionPointerC);
    pRecordC->initInstance ("myRecordC", &initCodeC);
    pRecordC->freeInstance ("myRecordC", &freeCodeC);
    pRecordC->writeArgument ("myRecordC", "x", "my_x", &setXCodeC);
    pRecordC->returnArgument ("(*pRecordC)", "x", &returnXCodeC);

    //record D
    std::string
		pointerTypeD,
		definitionCodeD,
		definitionPointerD,
		freeCodeD,
		initCodeD,
		setXCodeD,
		returnXCodeD;
    pRecordD->getPointerType(&pointerTypeD);
    pRecordD->createInstance ("myRecordD", &definitionCodeD);
    pRecordD->createInstancePointer ("pRecordD", &definitionPointerD);
    pRecordD->initInstance ("myRecordD", &initCodeD);
    pRecordD->freeInstance ("myRecordD", &freeCodeD);
    pRecordD->writeArgument ("myRecordD", "x", "my_x", &setXCodeD);
    pRecordD->returnArgument ("(*pRecordD)", "x", &returnXCodeD);

    //======
    //Create a source file that uses the records
    //======
    //header
    std::ofstream out ("test_exhaustive.cc");

    out << "#include <stdio.h>" << std::endl;
    out << "#include <iostream>" << std::endl;
    out << "#include <list>" << std::endl;
    out << "#include <string.h>" << std::endl;

    for (iter = headers.begin(); iter != headers.end (); iter++)
        out << "#include <" << *iter << ">" << std::endl;

    out
		<< "std::list <std::pair<void*,uint64_t> > gSerBufs;" << std::endl
        << "std::list <" << pointerTypeB << "> gCallBRecords;" << std::endl << std::endl;

    //print function for A
    out
		<< "void printRecordA (" << definitionPointerA << ")" << std::endl
		<< "{" << std::endl
		<< "    std::cout << \"RecordA={x=\" " << std::endl
		<< "              << " << returnXCodeA << "<< \", y=\"" << std::endl
		<< "              << " << returnYCodeA << "<< \", z={\";" << std::endl
		<< "    for (int i = 0; i < " << returnXCodeA << "; i++)" << std::endl
		<< "        std::cout << " << returnZCodeA << "[i] << \", \";" << std::endl
		<< "    std::cout << \"}, w=\" <<" << returnWCodeA << "<< \"}\" << std::endl;"
		<< "}" << std::endl;

    //print function for B
    out
		<< "void printRecordB (" << definitionPointerB << ")" << std::endl
		<< "{" << std::endl
		<< "    std::cout << \"RecordB={x={\";" << std::endl
		<< "    for (int i = 0; i < " << returnLenCodeB << "; i++)" << std::endl
		<< "        std::cout << " << returnXCodeB << "[i] << \", \";" << std::endl
		<< "    std::cout << \"}, y={\";" << std::endl
		<< "    for (int i = 0; i < " << returnLenCodeB << "; i++)" << std::endl
		<< "        std::cout << " << returnYCodeB << "[i] << \", \";" << std::endl
		<< "    std::cout << \"}, len=\" <<" << returnLenCodeB << "<< \"}\" << std::endl;"
		<< "}" << std::endl;

    //print function for C
    out
		<< "void printRecordC (" << definitionPointerC << ")" << std::endl
		<< "{" << std::endl
		<< "    std::cout << \"RecordC={x=\" << " << returnXCodeC << "<< \"}\" << std::endl;" << std::endl
		<< "}" << std::endl;

    //print function for D
	out
		<< "void printRecordD (" << definitionPointerD << ")" << std::endl
		<< "{" << std::endl
		<< "    std::cout << \"RecordD={x=\" << " << returnXCodeD << "<< \"}\" << std::endl;" << std::endl
		<< "}" << std::endl;

	//wrapper function for A
    out << std::endl
        << "extern \"C\" int callA (int my_x, float my_y, int *my_z, char *my_w)" << std::endl
        << "{" << std::endl
        << "    void *serBuf = NULL;" << std::endl
        << "    uint64_t serLen;" << std::endl
        << "    " << definitionCodeA << ";" << std::endl
        << std::endl
        << "    " << initCodeA << std::endl
        << "    " << setXCodeA << std::endl
        << "    " << setYCodeA << std::endl
        << "    " << setZCodeA << std::endl
        << "    " << setWLenCodeA << std::endl
        << "    " << setWCodeA << std::endl
        << "    " << serializeCodeA << std::endl
        << std::endl
        << "    assert (" << getUidCode << " == 1);" << std::endl
        << std::endl
        << "    gSerBufs.push_back (std::make_pair(serBuf, serLen));"
        << std::endl
        << "    " << freeCodeA << std::endl
        << "    return 0;" << std::endl
        << "}" << std::endl;


    //wrapper function for B
    out << std::endl
		<< "extern \"C\" int callB (int *my_x, int *my_y, int my_len)" << std::endl
		<< "{" << std::endl
		<< "    void *serBuf = NULL;" << std::endl
		<< "    uint64_t serLen;" << std::endl
		<< "    " << definitionCodeB << ";" << std::endl
		<< "    " << definitionPointerB << "= NULL;" << std::endl
		<< std::endl
		<< "    " << initCodeB << std::endl
		<< "    " << setLenCodeB << std::endl
		<< "    " << setXCodeB << std::endl
		<< "    " << setYCodeB << std::endl
		<< "    " << serializeCodeB << std::endl
		<< std::endl
		<< "    assert (" << getUidCode << " == 2);" << std::endl
		<< std::endl
		<< "    " << allocPointerCodeB << std::endl
		<< "    " << deserializeCodeB << std::endl
		<< std::endl
		<< "    gCallBRecords.push_back (pRecordB);"
		<< std::endl
		<< "    " << freeCodeB << std::endl
		<< "    free (serBuf);" << std::endl
		<< "    return 0;" << std::endl
		<< "}" << std::endl;

	//wrapper function for C
	out << std::endl
		<< "extern \"C\" int callC (int my_x)" << std::endl
		<< "{" << std::endl
		<< "    void *serBuf = NULL;" << std::endl
		<< "    uint64_t serLen;" << std::endl
		<< "    " << definitionCodeC << ";" << std::endl
		<< std::endl
		<< "    " << initCodeC << std::endl
		<< "    " << setXCodeC << std::endl
		<< std::endl
		//The list of serialized buffers
		<< "    std::list <std::pair<void*,uint64_t> >::iterator i;" << std::endl
		<< "    for (i = gSerBufs.begin (); i != gSerBufs.end(); i++)" << std::endl
		<< "    {" << std::endl
		<< "        serBuf = i->first;" << std::endl
		<< "        serLen = i->second;" << std::endl
		<< "        " << definitionCodeA << ";" << std::endl
		<< "        " << deserializeCodeA << std::endl
		<< "        printRecordA (&myRecordA);" << std::endl
		<< "        " << freeCodeA << std::endl
		<< "        free (serBuf);" << std::endl
		<< "    }" << std::endl
		<< "    gSerBufs.clear ();" << std::endl
		<< std::endl
		//The list of record pointers
		<< "    std::list <" << pointerTypeB << ">::iterator j;" << std::endl
		<< "    for (j = gCallBRecords.begin (); j != gCallBRecords.end(); j++)" << std::endl
		<< "    {" << std::endl
		<< "        " << definitionPointerB << " = *j;" << std::endl
		<< "        printRecordB (pRecordB);" << std::endl
		<< "        " << freePointerCodeB << std::endl
		<< "        " << deallocPointerCodeB << std::endl
		<< "    }" << std::endl
		<< "    gCallBRecords.clear ();" << std::endl
		<< std::endl
		//print C
		<< "    printRecordC (&myRecordC);" << std::endl
		<< "    " << freeCodeC << std::endl
		<< "    return 0;" << std::endl
		<< "}" << std::endl;

	//wrapper for D
	out << std::endl
		<< "extern \"C\" int callD (int my_x)" << std::endl
		<< "{" << std::endl
		<< "    void *serBuf = NULL;" << std::endl
		<< "    uint64_t serLen;" << std::endl
		<< "    " << definitionCodeD << ";" << std::endl
		<< std::endl
		<< "    " << initCodeD << std::endl
		<< "    " << setXCodeD << std::endl
		<< std::endl
		//print D
		<< "    printRecordD (&myRecordD);" << std::endl
		<< "    " << freeCodeD << std::endl
		<< "    return 0;" << std::endl
		<< "}" << std::endl;

    //======
    //Clean up
    //======
    pRecordA->deleteObject ();
    pRecordB->deleteObject ();
    pRecordC->deleteObject ();
    pRecordD->deleteObject ();
    pDescA->deleteObject ();
    pDescB->deleteObject ();
    pDescC->deleteObject ();
    pDescD->deleteObject ();
    delete pImpl;

    //======
    //Write a CMake file for the generated source file
    //======
    //TODO: Missing is the include for the gti Macro file and the import
    //      of the flags, libraries, ... used to compile gti
    std::ofstream make ("CMakeLists.txt");
    make
        << "CMAKE_MINIMUM_REQUIRED(VERSION 2.6)" << std::endl
        << "PROJECT (MyExhaustiveCallWrapper C CXX)" << std::endl
        << "SET (SOURCES test_exhaustive.cc";

    for (iter = sources1.begin(); iter != sources1.end (); iter++)
        make << " " << *iter;
    for (iter = sources2.begin(); iter != sources2.end (); iter++)
        make << " " << *iter;

    make
        << ")" << std::endl
        << "ADD_LIBRARY (myExhaustiveWrapper MODULE ${SOURCES})" << std::endl
        << "SET_TARGET_PROPERTIES (myExhaustiveWrapper PROPERTIES" << std::endl
        << "    VERSION 1.0.0 SOVERSION 1" << std::endl
        << "    )" << std::endl
        << "TARGET_LINK_LIBRARIES (myExhaustiveWrapper ";

    for (iter = libs.begin(); iter != libs.end (); iter++)
        make << " " << *iter;

    make
        << ")" << std::endl
        << "INSTALL(TARGETS myExhaustiveWrapper LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/lib)" << std::endl
        << "#Optional#GTI_MAC_GET_TARGET_MODULE_NAME (name myExhaustiveWrapper 1.0.0)" << std::endl
        << "#Optional#GTI_MAC_ADD_PNMPI_PATCH (${name} ${PNMPI_PATCHER})" << std::endl;

    //======
    //close files
    //======
    out.close ();
    make.close ();

    return 0;
}
