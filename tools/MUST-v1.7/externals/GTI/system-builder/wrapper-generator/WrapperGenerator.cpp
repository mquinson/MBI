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
 * @file WrapperGenerator.cpp
 *		@see gti::codegen::WrapperGenerator
 *
 * @author Tobias Hilbrich
 * @date 11.08.2010
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <assert.h>

#include "gtiConfig.h"
#include "Verbose.h"

#include "WrapperGenerator.h"

using namespace gti::codegen;

/**
 * WrapperGenerator main function.
 */
int main (int argc, char** argv)
{
	//=========== 0 read input or print help ===========
	//print help if requested
	if ((argc == 2) &&
		(
				(strncmp(argv[1], "--help", strlen("--help")) == 0 ) ||
				(strncmp(argv[1], "-help", strlen("--help")) == 0 ) ||
				(strncmp(argv[1], "-h", strlen("--help")) == 0 )
		)
	)
	{
		printUsage (argv[0], std::cout);
		return 0;
	}

	//enough arguments ?
	if (argc < 2)
	{
		std::cerr << "Error: Not enough arguments!" << std::endl << std::endl;
		printUsage (argv[0], std::cerr);
		return 1;
	}

	std::string fileName = argv[1];
	int retVal = 0;
	WrapperGenerator generator (fileName, &retVal);

	return retVal;
}

//=============================
// printUsage
//=============================
void printUsage (std::string execName, std::ostream &out)
{
	out
		<< "Usage: "
		<< execName
		<< " <WrapperGeneratorInputXML>" << std::endl
		<< std::endl
		<< "E.g.: " << execName << " wrappGenIn.xml"
		<< std::endl;
}

//=============================
// myGetGeneratorName
//=============================
std::string WrapperGenerator::myGetGeneratorName (void)
{
	return "wrapper generator";
}

//=============================
// myGetRootNodeName
//=============================
std::string WrapperGenerator::myGetRootNodeName (void)
{
	return "wrapper-specification";
}

//=============================
// WrapperGenerator
//=============================
WrapperGenerator::WrapperGenerator (std::string inputFile, int* retVal)
	: 	GeneratorBase (),
	  	myOrder (0)
{
	VERBOSE(1) << "Processing input file " << inputFile << " ..." << std::endl;

	if (retVal)
		*retVal = 1;

	//Open the input XML
	//=================================
	SpecificationNode currentPointer;
	SpecificationNode child;

	currentPointer = openInput (inputFile);
	if (!currentPointer) return;
	std::string orderStr;
	if (!currentPointer.getAttributeOrErr("order", "|  |--> Error: root node has no \"order\" attribute.", &orderStr))
		return;
	myOrder = atoi(orderStr.c_str());

	//Read: settings
	//=================================
	VERBOSE(1) << "|- Reading settings ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"settings",
			"|  |--> Error: root node has no \"settings\" child.");
	if (!readSettings (child))
		return;

	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

	//Get record generation implementation
	//=================================
	if (!getRecordGenerationImplementation(&myImpl))
		return;

	//Init record generation
	//=================================
	std::list<std::string>	sources1,
							sources2,
							libs,
							headers;

	myImpl->initBegin(myOutDir,mySourceName);
	myImpl->initGetIntermediateSourceFileNames (&sources1);
	myImpl->initGetStaticSourceFileNames (&sources2);
	myImpl->initGetLibNames (&libs);
	myImpl->initGetHeaderFileNames (&myRecordHeaders);
	myImpl->initEnd ();

	//Read: headers
	//=================================
	VERBOSE(1) << "|- Reading headers ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"headers",
			"|  |--> Error: root node has no \"headers\" child.");
	if (!readAndPrintHeaders (child))
		return;

	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

	//Read: communications
	//=================================
	VERBOSE(1) << "|- Reading communications ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"communications",
			"|  |--> Error: root node has no \"communications\" child.");
	if (!readCommunications (child))
		return;

	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

	//Read: analyses
	//=================================
	VERBOSE(1) << "|- Reading analyses ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"analyses",
			"|  |--> Error: root node has no \"analyses\" child.");
	if (!readAnalyses (child))
		return;

	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

	//Write basic parts of source
	//=================================
	if (!writeSourceHeaderBasics())
		return;

	//Read: calls
	//=================================
	VERBOSE(1) << "|- Reading calls ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"calls",
			"|  |--> Error: root node has no \"calls\" child.");
	if (!readCalls (child))
		return;

	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

	//Write bye bye to the sources
	mySourceOut << std::endl << "/*EOF*/" << std::endl << std::endl;

	//Clean up
	//=================================
	VERBOSE(1) << "--> SUCCESS" << std::endl;
	if (retVal)
		*retVal = 0;

	//Write output XML
	//=================================
	std::list<std::string>::iterator lIter;

	myLogOut
		<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		<< "<wrappgen-output>" << std::endl
		<< "\t<extra-sources>" << std::endl;

	for (lIter = sources1.begin(); lIter != sources1.end(); lIter++)
	{
		myLogOut << "<source>" << *lIter << "</source>" << std::endl;
	}

	for (lIter = sources2.begin(); lIter != sources2.end(); lIter++)
	{
		myLogOut << "<source>" << *lIter << "</source>" << std::endl;
	}

	myLogOut
		<< "\t</extra-sources>" << std::endl
		<< "\t<extra-libs>" << std::endl;

	for (lIter = libs.begin(); lIter != libs.end(); lIter++)
	{
		myLogOut << "<lib>" << *lIter << "</lib>" << std::endl;
	}
	myLogOut << "<lib>libGtiTLS.so</lib>" << std::endl;

	myLogOut
		<< "\t</extra-libs>" << std::endl
		<< "</wrappgen-output>";
}

//=============================
// ~WrapperGenerator
//=============================
WrapperGenerator::~WrapperGenerator (void)
{

}

//=============================
// writeSourceHeaderBasics
//=============================
bool WrapperGenerator::writeSourceHeaderBasics (void)
{
	mySourceOut
		<< "using namespace gti;"
		<< "" << std::endl;
        
        mySourceOut
		<< "extern \"C\" int getFunctionGenWrapper_" << myClassName << " (const char* functionName, GTI_Fct_t* pOutFunctionAddr);" << std::endl
		<< "extern \"C\" int getNextEventStridedFunctionGenWrapper_" << myClassName << " (GTI_Fct_t* pOutFunctionAddr);" << std::endl
		<< "extern \"C\" int getAcrossFunctionGenWrapper_" << myClassName << " (const char* functionName, GTI_Fct_t* pOutFunctionAddr);" << std::endl
		<< "extern \"C\" int getBroadcastFunctionGenWrapper_" << myClassName << " (const char* functionName, GTI_Fct_t* pOutFunctionAddr);" << std::endl
		<< "extern \"C\" int getPlaceGenWrapper_" << myClassName << " (I_Place** outPlace);" << std::endl
		<< "" << std::endl
		<< "mGET_INSTANCE_FUNCTION (GenWrapper_" << myClassName << ")" << std::endl
		<< "mFREE_INSTANCE_FUNCTION_FORCED (GenWrapper_" << myClassName << ")" << std::endl
		<< "mPNMPI_REGISTRATIONPOINT_FUNCTION_WRAPPER (GenWrapper_" << myClassName << ")" << std::endl
        << std::endl
		<< "GenWrapper_" << myClassName << "::GenWrapper_" << myClassName << " (const char* instanceName)" << std::endl
		<< "    : ModuleBase<GenWrapper_" << myClassName << ", I_Module> (instanceName), myProfiler (NULL)" << std::endl
		<< "{" << std::endl
		<< "    //add the id to all sub modules" << std::endl
		<< "    char temp[64];" << std::endl
		<< "    sprintf (temp,\"%lld\",buildLayer());" << std::endl
		<< "    addDataToSubmodules (\"id\", temp);" << std::endl
		<< "" << std::endl
		<< "    //create sub modules" << std::endl
		<< "    std::vector<I_Module*> subModInstances;" << std::endl
		<< "    subModInstances = createSubModuleInstances ();" << std::endl
		<< "" << std::endl
		<< "    //save sub modules" << std::endl;

	//Add module saving
	//0) check enough modules given
	int extraMods = 0;
	if (myProfiling) extraMods++; //If so we get one extra module for the profiler
	mySourceOut
		<< "    assert (subModInstances.size() == " << 1 /*place*/ + myCommMods.size() + myAnalysisMods.size() + myIntraCommMods.size() + myDownCommMods.size() + extraMods<< ");" << std::endl;

	//1) comm modules
	std::map <int, std::string>::iterator commIter;
	int i = 0;
	for (commIter = myCommMods.begin(); commIter != myCommMods.end(); commIter++, i++)
	{
		int id = commIter->first;
		std::string name = commIter->second;

		mySourceOut	<< "    myCStrats[" << i << "] = (I_CommStrategyUp*) subModInstances[" << i << "];" << std::endl;
	}

	//1b) intra comm module
	int intraIndex = 0;
	for (commIter = myIntraCommMods.begin(); commIter != myIntraCommMods.end(); commIter++, i++, intraIndex++)
	{
	    int id = commIter->first;
	    std::string name = commIter->second;

	    mySourceOut << "    myIntraCStrats[" << intraIndex << "] = (I_CommStrategyIntra*) subModInstances[" << i << "];" << std::endl;
	}

	//1c) down comm module
	intraIndex = 0;
	for (commIter = myDownCommMods.begin(); commIter != myDownCommMods.end(); commIter++, i++, intraIndex++)
	{
	    int id = commIter->first;
	    std::string name = commIter->second;

	    mySourceOut << "    myDownCStrats[" << intraIndex << "] = (I_CommStrategyDown*) subModInstances[" << i << "];" << std::endl;
	}

	//2) analyses
	std::list <ModuleInfo*>::iterator anIter;
	for (anIter = myAnalysisMods.begin(); anIter != myAnalysisMods.end(); anIter++, i++)
	{
		ModuleInfo* info = *anIter;

		mySourceOut << "    analysis" << info->index << " = (" << info->datatype << "*) subModInstances[" << i << "];" << std::endl;
	}
        
	//2b) profiler
	if (myProfiling){
	    mySourceOut << "    myProfiler = (I_Profiler*) subModInstances[" << i << "];" << std::endl;
	    i++;
        }
        
        //2c) place
		mySourceOut	<< "    myPlace = (I_Place*) subModInstances["<<i<<"];" << std::endl;
	//3) profiling variables for profiling (if enabled)
	if (myProfiling)
	{
	    std::map <std::string, AnalysisFunction>::iterator anFnIter;
	    for (anFnIter = myAnalyses.begin(); anFnIter != myAnalyses.end();anFnIter++)
	    {
	        mySourceOut << "    " << getProfilingVariableName(&(anFnIter->second)) << " = 0;" << std::endl;
	        mySourceOut << "    " << getProfilingCountVariableName(&(anFnIter->second)) << " = 0;" << std::endl;
	    }
	    mySourceOut << "    usec_communicating = 0;" << std::endl;
	    mySourceOut << "    count_communicating = 0;" << std::endl;
	    mySourceOut << "    usec_communicatingIntra = 0;" << std::endl;
	    mySourceOut << "    count_communicatingIntra = 0;" << std::endl;
	    mySourceOut << "    myIdleTime = 0;" << std::endl;
	    mySourceOut << "    myLastWrapperExitTime = 0;" << std::endl;
	    mySourceOut << "    myNextEventOffset = 0;" << std::endl;
	    mySourceOut << "    myNextEventStride = 1;" << std::endl;
	}

	mySourceOut
		<< "" << std::endl
		<< "    myInWrapper=false;" << std::endl
		<< "" << std::endl
		<< "    //trigger trace init" << std::endl;

        mySourceOut
		<< "}" << std::endl
		<< std::endl
		<< "GenWrapper_" << myClassName << "::~GenWrapper_" << myClassName << " (void)"  << std::endl
		<< "{" << std::endl;

	//1) comm modules
	i = 0;
	for (commIter = myCommMods.begin(); commIter != myCommMods.end(); commIter++, i++)
	{
		int id = commIter->first;
		std::string name = commIter->second;

		mySourceOut
			<< "    myCStrats[" << i << "]->shutdown(GTI_FLUSH, GTI_SYNC);" << std::endl
			<< "    destroySubModuleInstance ((I_Module*) myCStrats[" << i << "]);" << std::endl;

	}

	i=0;
	for (commIter = myIntraCommMods.begin(); commIter != myIntraCommMods.end(); commIter++, i++)
	{
	    int id = commIter->first;
	    std::string name = commIter->second;

	    mySourceOut
	        //No shutdown call needed, the comm strategy is managed by the placement driver, we just use it
	        << "    destroySubModuleInstance ((I_Module*) myIntraCStrats[" << i << "]);" << std::endl;

	}

	i=0;
	for (commIter = myDownCommMods.begin(); commIter != myDownCommMods.end(); commIter++, i++)
	{
	    int id = commIter->first;
	    std::string name = commIter->second;

	    mySourceOut
	    //No shutdown call needed, the comm strategy is managed by the placement driver, we just use it
	    << "    destroySubModuleInstance ((I_Module*) myDownCStrats[" << i << "]);" << std::endl;

	}

	//2) analyses
	for (anIter = myAnalysisMods.begin(); anIter != myAnalysisMods.end(); anIter++, i++)
	{
		ModuleInfo *info = *anIter;

		mySourceOut
			<< "    destroySubModuleInstance ((I_Module*) analysis" << info->index << ");" << std::endl;
	}

	//3) profiling: print the results (if enabled)
	if (myProfiling)
	{
	    std::map <std::string, AnalysisFunction>::iterator anFnIter;
	    for (anFnIter = myAnalyses.begin(); anFnIter != myAnalyses.end();anFnIter++)
	    {
	        mySourceOut << "   myProfiler->reportWrapperAnalysisTime (\"" << anFnIter->second.info->name << "\", \"" << anFnIter->second.function << "\", " << getProfilingVariableName(&(anFnIter->second)) << ", " << getProfilingCountVariableName(&(anFnIter->second)) << ");" << std::endl;
	    }

	    mySourceOut << "   myProfiler->reportUpCommTime (usec_communicating, count_communicating);" << std::endl;
	    mySourceOut << "   myProfiler->reportIntraCommTime (usec_communicatingIntra, count_communicatingIntra);" << std::endl;
	    mySourceOut << "   myProfiler->reportIdleTime (myIdleTime);" << std::endl;

	    mySourceOut << "    destroySubModuleInstance ((I_Module*) myProfiler);" << std::endl;
	}
        mySourceOut << "    destroySubModuleInstance ((I_Module*) myPlace);" << std::endl;

	mySourceOut
		<< "}" << std::endl
		<< std::endl
		<< "uint64_t GenWrapper_" << myClassName << "::getUsecTime (void)" <<  std::endl
		<< "{" << std::endl
		<< "    struct timeval t;" << std::endl
		<< "    gettimeofday(&t, NULL);" << std::endl
		<< "    return t.tv_sec * 1000000 + t.tv_usec;" << std::endl
		<< "}" << std::endl
		<< "" << std::endl
		<< "" << std::endl
		<< "GTI_RETURN free_serialized_buf (void* free_data, uint64_t num_bytes, void* buf)" << std::endl
		<< "{" << std::endl
		<< "    if (!free_data)" << std::endl
		<< "    {" << std::endl
		<< "        delete[] ((char*)buf);" << std::endl
		<< "        return GTI_SUCCESS;" << std::endl
		<< "    }" << std::endl
		<< std::endl
		<< "    int *refCount = (int*) free_data;" << std::endl
		<< "    *refCount = *refCount - 1;" << std::endl
		<< "    if (*refCount == 0)" << std::endl
		<< "    {" << std::endl
		<< "        free ((char*)buf);" << std::endl
///*C stuff in record gen uses malloc ...*/		<< "        delete[] ((char*)buf);" << std::endl
		<< "        delete refCount;" << std::endl
		<< "    }" << std::endl
		<< std::endl
		<< "    return GTI_SUCCESS;" << std::endl
		<< "}" << std::endl << std::endl;     
        
	//Add a function that we use to set stride/offset for strided event injection
	mySourceOut
	    << "extern \"C\" int gtiInternalSetNextEventStrided (uint32_t offset, uint32_t stride)" << std::endl
	    << "{" << std::endl
	    << "  GenWrapper_" << myClassName << "* place;" << std::endl
	    << "  place = GenWrapper_" << myClassName << "::getInstance(\"\");" << std::endl
	    << "  if (place)" << std::endl
	    << "  {" << std::endl
	    << "    place->myNextEventOffset = offset;" << std::endl
	    << "    place->myNextEventStride = stride;" << std::endl
	    << "  }" << std::endl
	    << "  return 0;" << std::endl
	    << "}" << std::endl << std::endl;

	//Stuff for header
	myHeaderOut
		<< "#include \"GtiDefines.h\"" << std::endl
		<< "#include \"GtiEnums.h\"" << std::endl
		<< "#include \"GtiMacros.h\"" << std::endl
		<< "#include \"GtiTypes.h\"" << std::endl
		<< "#include \"I_Module.h\"" << std::endl
        << "#include \"GtiHelper.h\"" << std::endl
		<< "#include \"ModuleBase.h\"" << std::endl
		<< "#include \"I_Profiler.h\"" << std::endl
		<< "#include <iostream>" << std::endl
		<< "#include <stdio.h>" << std::endl
		<< "#include <stdlib.h>" << std::endl
		<< "#include <sstream>" << std::endl
		<< "#include <fstream>" << std::endl
        << "#include <sys/time.h>" << std::endl;

	std::list<std::string>::iterator headerIter;
	for (headerIter = myRecordHeaders.begin(); headerIter != myRecordHeaders.end(); headerIter++)
	{
		myHeaderOut << "#include \"" << *headerIter <<  "\"" << std::endl;
	}

	myHeaderOut
		<< "" << std::endl
		<< "namespace gti" << std::endl
		<< "{" << std::endl;
        
        myHeaderOut
		<< "    class GenWrapper_" << myClassName << " : public ModuleBase<GenWrapper_" << myClassName << ", I_Module>, public GtiHelper" << std::endl
		<< "    {" << std::endl
		<< "    public:" << std::endl
		<< "        bool myInWrapper;" << std::endl;

        //0) place module
	myHeaderOut	<< "        I_Place* myPlace;" << std::endl;

	//1) comm modules
	myHeaderOut	<< "        I_CommStrategyUp* myCStrats[" << myCommMods.size() << "];" << std::endl;

	//1b) intra comm modules
	if (!myIntraCommMods.empty())
	    myHeaderOut << "        I_CommStrategyIntra* myIntraCStrats[" << myIntraCommMods.size() << "];" << std::endl;

	//1c) down comm modules
	if (!myDownCommMods.empty())
	    myHeaderOut << "        I_CommStrategyDown* myDownCStrats[" << myDownCommMods.size() << "];" << std::endl;

	//2) analyses
	for (anIter = myAnalysisMods.begin(); anIter != myAnalysisMods.end(); anIter++)
	{
		ModuleInfo *info = *anIter;

		myHeaderOut << "        " << info->datatype << "* analysis" << info->index << ";" << std::endl;
	}

	//2b) Profiler
	myHeaderOut << "        I_Profiler* myProfiler;" << std::endl;

	//3) profiling variables for profiling (if enabled)
	if (myProfiling)
	{
	    std::map <std::string, AnalysisFunction>::iterator anFnIter;
	    for (anFnIter = myAnalyses.begin(); anFnIter != myAnalyses.end();anFnIter++)
	    {
	        myHeaderOut << "        uint64_t " <<getProfilingVariableName(&(anFnIter->second)) << ";" << std::endl;
	        myHeaderOut << "        uint64_t " <<getProfilingCountVariableName(&(anFnIter->second)) << ";" << std::endl;
	    }
	    myHeaderOut << "        uint64_t usec_communicating;" << std::endl;
	    myHeaderOut << "        uint64_t count_communicating;" << std::endl;
	    myHeaderOut << "        uint64_t usec_communicatingIntra;" << std::endl;
	    myHeaderOut << "        uint64_t count_communicatingIntra;" << std::endl;
	    myHeaderOut << "        uint64_t myIdleTime;" << std::endl;
	    myHeaderOut << "        uint64_t myLastWrapperExitTime;" << std::endl;
	}

	    myHeaderOut << "        uint32_t myNextEventOffset;" << std::endl;
	    myHeaderOut << "        uint32_t myNextEventStride;" << std::endl;
           
	myHeaderOut
		<< "" << std::endl
		<< "    public:" << std::endl
		<< "        /**" << std::endl
		<< "         * Constructor." << std::endl
		<< "         * Sets up the this tool place with all its modules." << std::endl
		<< "         * @param instanceName name of this module instance." << std::endl
		<< "         */" << std::endl
		<< "        GenWrapper_" << myClassName << " (const char* moduleConf);" << std::endl
		<< "" << std::endl
		<< "        /**" << std::endl
		<< "         * Destructor." << std::endl
		<< "         */" << std::endl
		<< "        ~GenWrapper_" << myClassName << " (void);" << std::endl
		<< "" << std::endl
		<< "        /*" <<  std::endl
		<< "         * Helper Method for usec time stamp" <<  std::endl
		<< "         */" <<  std::endl
		<< "        uint64_t getUsecTime (void);" <<  std::endl
		<< "    };" << std::endl
		<< "}" << std::endl;
        
	return true;
}

//=============================
// readCalls
//=============================
bool WrapperGenerator::readCalls (SpecificationNode node)
{
	SpecificationNode child;
	std::list <std::pair<std::string, int> > wrappedCalls; //used to generate code that returns pointers to wrapped calls; the int is:
	                                                                                  // 0-> regular wrapp-everywhere call, 1-> wrapp-across call, 2-> wrapp-down call!
	std::list <std::pair<std::string, int> >::iterator iter;
	bool isWrapAcross, isWrapDown;

	//==Childs: call
	child = node.findChildNodeNamed("call");

	while (child)
	{
		std::string callName;

		isWrapAcross = isWrapDown = false;
		if (!readCall (child, &callName, &isWrapAcross, &isWrapDown))
			return false;

		int type = 0;
		if (isWrapAcross)
		    type = 1;
		else if (isWrapDown)
		    type = 2;
		wrappedCalls.push_back (std::make_pair(callName, type));

		//next
		child = child.findSiblingNamed("call");
	}

	//==Create the "getFunction" function that returns addresses
	//  to wrapper functions
	mySourceOut
		<< std::endl
		<< "extern \"C\" int getFunctionGenWrapper_" << myClassName << " (const char* functionName, GTI_Fct_t* pOutFunctionAddr)" << std::endl
		<< "{" << std::endl;

	bool hadEntry = false;
	for (iter = wrappedCalls.begin(); iter != wrappedCalls.end(); iter++)
	{
	    if (iter->second != 0)
	        continue;

		if (hadEntry)
			mySourceOut << "    else" << std::endl;

		hadEntry = true;

		mySourceOut
			<< "    if ((std::string)functionName == (std::string)\"" << iter->first << "\")" << std::endl
			<< "    {" << std::endl
			<< "        *pOutFunctionAddr = (GTI_Fct_t) &" << iter->first << ";" << std::endl
			<< "        return PNMPI_SUCCESS;" << std::endl
			<< "    }" << std::endl;
	}

	mySourceOut
		<< "    " << std::endl
		<< "    return PNMPI_NOSERVICE;" << std::endl
		<< "}" << std::endl
		<< std::endl;

	//==Create the "getNextEventStridedFunction" function that provides modules the options to tell GTI that an aggregated event uses a strided subset of its rank-range
	mySourceOut
        << std::endl
        << "extern \"C\" int getNextEventStridedFunctionGenWrapper_" << myClassName << " (GTI_Fct_t* pOutFunctionAddr)" << std::endl
        << "{" << std::endl
        << "        *pOutFunctionAddr = (GTI_Fct_t) &gtiInternalSetNextEventStrided;" << std::endl
        << "        return PNMPI_SUCCESS;" << std::endl
        << "}" << std::endl
        << std::endl;

	//==Create the "getAcrossFunction" function that returns addresses
	//  to wrapper functions that are wrap-across functions
	mySourceOut
        << std::endl
        << "extern \"C\" int getAcrossFunctionGenWrapper_" << myClassName << " (const char* functionName, GTI_Fct_t* pOutFunctionAddr)" << std::endl
        << "{" << std::endl;

	hadEntry = false;
	for (iter = wrappedCalls.begin(); iter != wrappedCalls.end(); iter++)
	{
	    if (iter->second != 1)
	        continue;

	    if (hadEntry)
	        mySourceOut << "    else" << std::endl;

	    hadEntry = true;

	    mySourceOut
	    << "    if ((std::string)functionName == (std::string)\"" << iter->first << "\")" << std::endl
	    << "    {" << std::endl
	    << "        *pOutFunctionAddr = (GTI_Fct_t) &" << iter->first << ";" << std::endl
	    << "        return PNMPI_SUCCESS;" << std::endl
	    << "    }" << std::endl;
	}

	mySourceOut
	<< "    " << std::endl
	<< "    return PNMPI_NOSERVICE;" << std::endl
	<< "}" << std::endl
	<< std::endl;

	//==Create the "getBroadcastFunction" function that returns addresses
	//  to wrapper functions that are wrap-across functions
	mySourceOut
	<< std::endl
	<< "extern \"C\" int getBroadcastFunctionGenWrapper_" << myClassName << " (const char* functionName, GTI_Fct_t* pOutFunctionAddr)" << std::endl
	<< "{" << std::endl;

	hadEntry = false;
	for (iter = wrappedCalls.begin(); iter != wrappedCalls.end(); iter++)
	{
	    if (iter->second != 2)
	        continue;

	    if (hadEntry)
	        mySourceOut << "    else" << std::endl;

	    hadEntry = true;

	    mySourceOut
	    << "    if ((std::string)functionName == (std::string)\"" << iter->first << "\")" << std::endl
	    << "    {" << std::endl
	    << "        *pOutFunctionAddr = (GTI_Fct_t) &" << iter->first << ";" << std::endl
	    << "        return PNMPI_SUCCESS;" << std::endl
	    << "    }" << std::endl;
	}

	mySourceOut
	<< "    " << std::endl
	<< "    return PNMPI_NOSERVICE;" << std::endl
	<< "}" << std::endl
	<< std::endl;

	//==Create the "getPlace" function that returns the 
	//  the address of the Placement driver
	mySourceOut
	<< std::endl
	<< "extern \"C\" int getPlaceGenWrapper_" << myClassName << " (I_Place** outPlace)" << std::endl
	<< "{" << std::endl
	<< "    if (outPlace)" << std::endl
	<< "    {" << std::endl
	<< "      GenWrapper_" << myClassName << "* place;" << std::endl
	<< "      place = GenWrapper_" << myClassName << "::getInstance(\"\");" << std::endl
	<< "      if (place)" << std::endl
	<< "      {" << std::endl
	<< "         *outPlace = place->myPlace;" << std::endl
	<< "        return PNMPI_SUCCESS;" << std::endl
	<< "      }" << std::endl
	<< "    }" << std::endl
	<< "    return PNMPI_NOSERVICE;" << std::endl
	<< "}" << std::endl
	<< std::endl;


	return true;
}

//=============================
// readCall
//=============================
bool WrapperGenerator::readCall (
        SpecificationNode node,
        std::string *pOutCallName,
        bool *pIsWrapAcross,
        bool *pIsWrapDown)
{
	SpecificationNode child, subchild, subsubchild;
	std::string retType;
	std::string name;
	bool isFinalizer = false;
	bool isLocalFinalizer = false;
	std::string isFinalizerString;
	bool isWrapAcross = false, isWrapDown = false;
        bool isCallback = false;
	bool isHook = false;
	std::string isWrapAcrossString, isWrapDownString, isCallbackString, isHookString;
	std::list< std::pair<std::string, std::string> > argsnTypes; // List:(type,arg)
	std::list< std::pair<std::string, std::string> >::iterator iter;
	std::list<std::string> typesAfterArg;
	std::list<std::string>::iterator afterIter;

	//==Optional attribute: is-finalizer
	if (node.getAttribute("is-finalizer", &isFinalizerString))
	{
		if (isFinalizerString == "yes")
			isFinalizer = true;

		if (isFinalizerString != "yes" && isFinalizerString != "no")
		{
			std::cerr << "|  |-->Error: a call uses the \"is-finalizer\" attribute with an unknown value \"" << isFinalizerString << "\", valid values are \"yes\" and \"no\"." << std::endl;
			return false;
		}
	}

	//==Optional attribute: is-local-finalizer
	if (node.getAttribute("is-local-finalizer", &isFinalizerString))
	{
	    if (isFinalizerString == "yes")
	        isLocalFinalizer = true;

	    if (isFinalizerString != "yes" && isFinalizerString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"is-local-finalizer\" attribute with an unknown value \"" << isFinalizerString << "\", valid values are \"yes\" and \"no\"." << std::endl;
	        return false;
	    }
	}

	//==Optional attribute: is-wrap-across
	if (node.getAttribute("is-wrap-across", &isWrapAcrossString))
	{
	    if (isWrapAcrossString == "yes")
	        isWrapAcross = true;

	    if (isWrapAcrossString != "yes" && isWrapAcrossString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"is-wrap-across\" attribute with an unknown value \"" << isWrapAcrossString << "\", valid values are \"yes\" and \"no\"." << std::endl;
	        return false;
	    }
	}

	if (pIsWrapAcross)
	    *pIsWrapAcross = isWrapAcross;

	//==Optional attribute: is-wrap-down
	if (node.getAttribute("is-wrap-down", &isWrapDownString))
	{
	    if (isWrapDownString == "yes")
	        isWrapDown = true;

	    if (isWrapDownString != "yes" && isWrapDownString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"is-wrap-down\" attribute with an unknown value \"" << isWrapDownString << "\", valid values are \"yes\" and \"no\"." << std::endl;
	        return false;
	    }
	}

	if (pIsWrapDown)
	    *pIsWrapDown = isWrapDown;
        
        //==Optional attribute: callback
	if (node.getAttribute("callback", &isCallbackString))
	{
	    if (isCallbackString == "yes")
	        isCallback = true;

	    if (isCallbackString != "yes" && isCallbackString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"callback\" attribute with an unknown value \"" << isCallbackString << "\", valid values are \"yes\" and \"no\"." << std::endl;
	        return false;
	    }
	}

	//==Optional attribute: hook
	if (node.getAttribute("hook", &isHookString))
	{
	    if (isHookString == "yes")
	        isHook = true;

	    if (isHookString != "yes" && isHookString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"hook\" attribute with an unknown value \"" << isHookString << "\", valid values are \"yes\" and \"no\"." << std::endl;
	        return false;
	    }
	}

	//==Child: return-type
	child = node.findChildNodeNamedOrErr(
			"return-type",
			"|  |-->Error: a call node has no \"return-type\" child.");
	if (!child) return false;
	retType = child.getNodeContent();
	bool isVoid=false;
        if (retType.compare("void")==0)
            isVoid=true;
	
	//==Child: call-name
	child = node.findChildNodeNamedOrErr(
			"call-name",
			"|  |-->Error: a call node has no \"call-name\" child.");
	if (!child) return false;
	name = child.getNodeContent();

	if (pOutCallName)
		*pOutCallName = name;

	//==Child: arguments and argument*
	child = node.findChildNodeNamedOrErr(
			"arguments",
			"|  |-->Error: a call node has no \"arguments\" child.");
	if (!child) return false;

	subchild = child.findChildNodeNamed("argument");

	while (subchild)
	{
		std::string type,arg, typeAfterArg = "";

		//==Attribute: typeAfterArg
		subchild.getAttribute("typeAfterArg", &typeAfterArg);
		typesAfterArg.push_back (typeAfterArg);

		//==Child: type
		subsubchild = subchild.findChildNodeNamedOrErr(
				"type",
				"|  |-->Error: an argument node has no \"type\" child.");
		if (!subsubchild) return false;
		type = subsubchild.getNodeContent();

		//==Child: arg
		subsubchild = subchild.findChildNodeNamedOrErr(
				"arg",
				"|  |-->Error: an argument node has no \"arg\" child.");
		if (!subsubchild) return false;
		arg = subsubchild.getNodeContent();

		//Add to list of arguments
		argsnTypes.push_back (std::make_pair(type,arg));

		//next
		subchild = subchild.findSiblingNamed("argument");
	}

	//==Open up the call!
	mySourceOut
		<< "extern \"C\" " << retType << " " << name << " (";
	afterIter = typesAfterArg.begin();
	for (iter = argsnTypes.begin(); iter != argsnTypes.end(); iter++,afterIter++)
	{
		if (iter != argsnTypes.begin())
			mySourceOut << ", ";
		mySourceOut << iter->first << " " << iter->second << *afterIter;
	}

	if (isWrapAcross)
	{
	    if (!argsnTypes.empty())
	        mySourceOut << ", ";

	    mySourceOut << "int implicitToPlace";
	}

	mySourceOut
		<< ")" << std::endl
		<< "{" << std::endl;
        if (!isVoid)
            mySourceOut
		<< "    " << retType << " ret" << name << ";" << std::endl;
  mySourceOut
		<< "    bool " << name << "wrapped = false;" << std::endl
		<< "" << std::endl
		<< "    GenWrapper_" << myClassName << "* place;" << std::endl
		<< "    place = GenWrapper_" << myClassName << "::getInstance(\"\");" << std::endl
		<< "" << std::endl
	    << "    /*********** Place alive ? && Recursion Guard ***********/" << std::endl
	    << "    if (place";

	//TODO recursion guards disabled, do we need them ? Will PnMPI handle this for MPI ?
    //Lets have recursion guards for MPI functions at the least!
	if (name[0] == 'M' && name[1] == 'P' && name[2] == 'I' && name[3] == '_')
	    mySourceOut << " && !place->myInWrapper";

	mySourceOut
	    << ")" << std::endl
		<< "    {" << std::endl;

	if (myProfiling)
	{
	    mySourceOut
	    << "   uint64_t entryTStamp = place->getUsecTime();" << std::endl
        << "   place->myProfiler->setWrapperEntryTime (entryTStamp);" << std::endl << std::endl;

	    //On application we recognize time between wrapper invocations as idle time!
	    if (myOrder == 0)
	    {
	        mySourceOut
	        << "   if (place->myLastWrapperExitTime != 0 && !place->myInWrapper)" << std::endl
	        << "      place->myIdleTime += entryTStamp - place->myLastWrapperExitTime;" <<std::endl << std::endl;
	    }
	}

	mySourceOut
	    << "        bool setInWrapper = false;" << std::endl
	    << "        if (!place->myInWrapper) setInWrapper = true;" << std::endl
	    << "        place->myInWrapper = true;" << std::endl
		<< "" << std::endl
		<< "#ifdef GTI_VERBOSE" << std::endl
		<< "        std::cout << \"Entering " << name << "\" << std::endl;" << std::endl
		<< "#endif" << std::endl
	    << "" << std::endl;

	//==Child: pre
	mySourceOut << "        /*********** Pre ***********/" << std::endl;
	child = node.findChildNodeNamedOrErr(
			"pre",
			"|  |-->Error: a call node has no \"pre\" child.");
	if (!child) return false;

	//=Childs: (source-piece|exec-analysis)*
	if (!printAnalysesPieces (child))
		return false;

	//=Sub-Child: forwarding
	subchild = child.findChildNodeNamedOrErr(
			"forwarding",
			"|  |-->Error: a pre node has no \"forwarding\" child.");
	if (!subchild) return false;
	if (!printForwarding(subchild, "place->", mySourceOut)) return false;

	//==If this is a finalizer we shutdown here ! (If we are the application layer, otherwise the placement driver will do this job)
	//// Before the actual call
	if ((isFinalizer || isLocalFinalizer) && (myOrder == 0))
	{
		mySourceOut
			<< "    //This call finalizes, we shutdown now !" << std::endl
			<< "    GenWrapper_" << myClassName << "::freeInstanceForced (place);" << std::endl
			<< "    place = NULL;" << std::endl
			<< std::endl;
	}
// ELP: callbacks do not need to call PXXX functions!
        if (!isCallback)
        {
            //==Do the actual call
            mySourceOut
                    << std::endl << "    /*********** Actual Call ***********/" << std::endl;

            if (!isHook)
            {
                if (!isVoid)
	                mySourceOut
	                    << "    ret" << name << " = ";

                mySourceOut
                    << "P" << name << "(";
                for (iter = argsnTypes.begin(); iter != argsnTypes.end(); iter++)
                {
                    if (iter != argsnTypes.begin())
                            mySourceOut << ", ";
                    mySourceOut << iter->second;
                }
                mySourceOut << ");" << std::endl;
            } else
                mySourceOut
                    << "PNMPI_Service_CallHook(\""<< name << "\");" << std::endl;
        }

	//==If this is a finalizer we must not do anything in Post
	//place is now NULL!
	if ((isFinalizer || isLocalFinalizer) && (myOrder == 0))
	{
            if (!isVoid)
		mySourceOut << "    return ret" << name << ";" << std::endl;
            else
		mySourceOut << "    return;" << std::endl;
	}

	//==Child: post
	mySourceOut << std::endl << "        /*********** Post ***********/" << std::endl;
	child = node.findChildNodeNamedOrErr(
			"post",
			"|  |-->Error: a call node has no \"post\" child.");
	if (!child) return false;

	//=Childs: (source-piece|exec-analysis)*
	if (!printAnalysesPieces (child))
		return false;

	//=Sub-Child: forwarding
	subchild = child.findChildNodeNamedOrErr(
			"forwarding",
			"|  |-->Error: a post node has no \"forwarding\" child.");
	if (!subchild) return false;
	if (!printForwarding(subchild, "place->", mySourceOut)) return false;

	//==Child: cleanup
	child = node.findChildNodeNamedOrErr(
			"cleanup",
			"|  |-->Error: a call node has no \"cleanup\" child.");
	if (!child) return false;

	subchild = child.findChildNodeNamed("source-piece");
	while (subchild)
	{
		if (!printSourcePiece (subchild))
			return false;

		//next
		subchild = subchild.findSiblingNamed("source-piece");
	}

	//==Close the if wrapped part
	mySourceOut
		<< std::endl
		<< "#ifdef GTI_VERBOSE" << std::endl
		<< "        std::cout << \"Leaving " << name << "\" << std::endl;" << std::endl
		<< "#endif" << std::endl
		<< std::endl
		<< "        if (setInWrapper)" << std::endl
		<< "          place->myInWrapper = false;" << std::endl;

	if (myProfiling && myOrder == 0)
	{
	    mySourceOut
	    << "   place->myLastWrapperExitTime = place->getUsecTime();" << std::endl;
	}

	mySourceOut
		<< "    } /*End place alive and recursion guard (if used)*/" << std::endl;


	//==Do the actual call for not wrapped case
	mySourceOut
		<< "    else" << std::endl
		<< "    {" << std::endl;

        if (!isCallback)
        {
            if (!isHook)
            {
                if (!isVoid)
                    mySourceOut
                        << "    ret" << name << " = ";

                mySourceOut
                    << "P" << name << "(";
                for (iter = argsnTypes.begin(); iter != argsnTypes.end(); iter++)
                {
                    if (iter != argsnTypes.begin())
                            mySourceOut << ", ";
                    mySourceOut << iter->second;
                }
                mySourceOut << ");" << std::endl;
            } else
                mySourceOut
                    << "PNMPI_Service_CallHook(\""<< name << "\");" << std::endl;
        }

	mySourceOut << "    }"<< std::endl;

	//==Close the call
	mySourceOut
		<< std::endl;
            if (!isVoid)
		mySourceOut << "    return ret" << name << ";" << std::endl;
            else
		mySourceOut << "    return;" << std::endl;
	mySourceOut
		<< "}" << std::endl << std::endl;

	return true;
}

//=============================
// printAnalysesPieces
//=============================
bool WrapperGenerator::printAnalysesPieces (SpecificationNode node)
{
	SpecificationNode child;

	std::list<std::string> toFind;
	toFind.push_back("source-piece");
	toFind.push_back("exec-analysis");
	std::string got;

	child = node.findChildNodeNamed (toFind, &got);

	while (child)
	{
		if (got == "source-piece")
		{
			if (!printSourcePiece (child))
				return false;
		}
		else
		{
			if (!printExecAnalysis(child))
				return false;
		}

		//next
		child = child.findSiblingNamed(toFind, &got);
	}

	return true;
}

//=============================
// printSourcePiece
//=============================
bool WrapperGenerator::printSourcePiece (SpecificationNode node)
{
	if (node.getNodeContent().length() > 0)
		mySourceOut
			<< "        //source-piece" << std::endl
			<< "        " << node.getNodeContent()
			<< std::endl
			<< std::endl;
	return true;
}

//=============================
// printExecAnalysis
//=============================
bool WrapperGenerator::printExecAnalysis (SpecificationNode node)
{
    static int x = 0;//For profiling

	SpecificationNode child, subchild;
	std::string analysisId;
	AnalysisFunction info;

	//==Child: analysis-id
	child = node.findChildNodeNamedOrErr(
			"analysis-id",
			"|  |-->Error: an exec-analysis node has no \"analysis-id\" child.");
	if (!child) return false;
	analysisId = child.getNodeContent();

	if (myAnalyses.find(analysisId) == myAnalyses.end())
	{
		std::cerr << "|  |-->Error: an exec-analysis node specifies an invalid analysis-id, no analysis with id \"" << analysisId << "\" known." << std::endl;
		return false;
	}

	info = myAnalyses[analysisId];

	//Start timing (if profiling)
	if (myProfiling)
	{
	    mySourceOut
	        << "uint64_t tStart_" << getProfilingVariableName(&info) << x << " = place->getUsecTime();" << std::endl;
	}

	mySourceOut
		<< "        //exec-analysis" << std::endl
		<< "        place->analysis" << info.info->index << "->" << info.function << "(";

	//==Child: inputs and input*
	child = node.findChildNodeNamedOrErr(
			"inputs",
			"|  |-->Error: an exec-analysis node has no \"inputs\" child.");
	if (!child) return false;

	subchild = child.findChildNodeNamed("input");

	bool isFirst = true;
	while (subchild)
	{
		if (!isFirst)
			mySourceOut << ", ";
		if (isFirst)
			isFirst= false;

		mySourceOut << subchild.getNodeContent();

		//next
		subchild = subchild.findSiblingNamed("input");
	}

	if (info.needsChannelId)
	{
		if (!isFirst)
			mySourceOut << ", ";

		mySourceOut << "NULL";
	}

	mySourceOut << ");" << std::endl << std::endl;

	//Stop timing (if profiling)
	if (myProfiling)
	{

	    /* PROFILING:
	     * Rational: we check whether we entered some wrapper since we started the
	     * timer for this analysis, if so we only account till this entry time.
	     * IMPORTANT: if an analysis calls multiple wrappers we still include
	     *                     some amount of infrastructure, wrapper, and comm in the
	     *                     recorded analysis time, as we will use the later wrapper
	     *                     entry time in that case. I hope this is rare enough such that
	     *                     we don't need to discern stacks of wrapper entry times.
	     */
	    mySourceOut
            << "if (place->myProfiler->getLastWrapperEntryTime() > tStart_" << getProfilingVariableName(&info) << x<< ")" << std::endl
            << "   place->" << getProfilingVariableName(&info) << " += place->myProfiler->getLastWrapperEntryTime() - tStart_" << getProfilingVariableName(&info) << x << ";" << std::endl
            << "else" << std::endl
            << "   place->" << getProfilingVariableName(&info) << " += place->getUsecTime() - tStart_" << getProfilingVariableName(&info) << x << ";" << std::endl
            << "place->" << getProfilingCountVariableName(&info) << " += 1;" << std::endl;
	    x++;
	}

	return true;
}

//=============================
// myGetRecordName
//=============================
std::string WrapperGenerator::myGetRecordName (void)
{
	return "recordIn";
}

/*EOF*/
