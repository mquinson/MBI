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
 * @file ReceivalGenerator.cpp
 *		@see gti::codegen::ReceivalGenerator
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <assert.h>

#include "Verbose.h"

#include "ReceivalGenerator.h"

using namespace gti::codegen;

/**
 * ReceivalGenerator main function.
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
	ReceivalGenerator generator (fileName, &retVal);

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
		<< " <ReceivalGeneratorInputXML>" << std::endl
		<< std::endl
		<< "E.g.: " << execName << " recvGenIn.xml"
		<< std::endl;
}

//=============================
// myGetRootNodeName
//=============================
std::string ReceivalGenerator::myGetRootNodeName (void)
{
	return "receival-specification";
}

//=============================
// myGetGeneratorName
//=============================
std::string ReceivalGenerator::myGetGeneratorName (void)
{
	return "receival generator";
}

//=============================
// ReceivalGenerator
//=============================
ReceivalGenerator::ReceivalGenerator (std::string inputFile, int* retVal)
 : myCIdFromLevel (0),
   myCIdNumLevels (0),
   myCIdNum64s (0),
   myCIdBitsPerChannel (0),
   myCIdArgumentBaseName (""),
   myCIdStartIndexPre (0),
   myCIdStartIndexPost (0),
   chanFunc (),
   forwardFunc (),
   intraFunc (),
   downFunc ()
{
	VERBOSE(1) << "Processing input file " << inputFile << " ..." << std::endl;

	if (retVal)
		*retVal = 1;

	//Open the input XML
	//=================================
	SpecificationNode currentPointer;
	SpecificationNode child, subchild;

	currentPointer = openInput (inputFile);
	if (!currentPointer) return;

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

	//Read: Channel Id Information
	//=================================
	VERBOSE(1) << "|- Reading channel-id ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"channel-id",
			"|  |--> Error: root node has no \"channel-id\" child.");
	if (!readChannelId (child))
		return;

	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

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

	//Read receivals
	//=================================
	VERBOSE(1) << "|- Reading receivals ..." << std::endl;
	child = currentPointer.findChildNodeNamedOrErr(
			"receivals",
			"|  |--> Error: root node has no \"receivals\" child.");

	subchild = child.findChildNodeNamed("receival");

	while (subchild)
	{
		if (!readReceival (subchild))
			return;

		//next
		subchild = subchild.findSiblingNamed("receival");
	}
	VERBOSE(1) << "|  |--> SUCCESS" << std::endl;

	//End update channel id
	//=================================
	chanFunc
		<< std::endl
		<< "    } /*End switch uid*/" << std::endl
		<< std::endl
		<< "    return GTI_SUCCESS;" << std::endl
		<< "} /*End getUpdatedChannelId*/" << std::endl
		<< std::endl;

	//End executeForward function
	//=================================
	forwardFunc
        << std::endl
        << "    } /*End switch uid*/" << std::endl
        << std::endl
        << "    return true;" << std::endl
        << "} /*End executeForeward*/" << std::endl
        << std::endl;

	//End ReceiveIntraRecord function
	//=================================
	intraFunc
	    << std::endl
	    << "    } /*End switch uid*/" << std::endl
	    << std::endl
	    << "    return GTI_SUCCESS;" << std::endl
	    << "} /*End ReceiveIntraRecord*/" << std::endl
	    << std::endl;

	//End executeDownForward function
	//=================================
	downFunc
	<< std::endl
	<< "    } /*End switch uid*/" << std::endl
	<< std::endl
	<< "    return GTI_SUCCESS;" << std::endl
	<< "} /*End ReceiveBroadcastRecord*/" << std::endl
	<< std::endl;

	//End source code
	//=================================
	std::string chanFuncStr = chanFunc.str();
	std::string forwardFuncStr = forwardFunc.str();
	std::string intraRecordStr = intraFunc.str();
	std::string downRecordStr = downFunc.str();

	mySourceOut
		<< std::endl
		<< "    } /*End switch uid*/" << std::endl
		<< std::endl
		<< "    return GTI_SUCCESS;" << std::endl
		<< "} /*End ReceiveRecord*/" << std::endl
		<< std::endl
		<< chanFuncStr  //Push channel id function in here !
		<< std::endl
		<< forwardFuncStr //Push execute forward function here !
		<< std::endl
		<< intraRecordStr //Push processIntraRecord function here !
		<< std::endl
		<< downRecordStr //Push processIntraRecord function here !
		<< std::endl
		<< "/*EOF*/"<< std::endl << std::endl;

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
		<< "<recvgen-output>" << std::endl
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
	<< "</recvgen-output>";
}

//=============================
// ~ReceivalGenerator
//=============================
ReceivalGenerator::~ReceivalGenerator (void)
{

}

//=============================
// writeSourceHeaderBasics
//=============================
bool ReceivalGenerator::writeSourceHeaderBasics (void)
{
	mySourceOut
			<< "using namespace gti;"
			<< "" << std::endl
			<< "mGET_INSTANCE_FUNCTION (GenReceival" << myClassName << ")" << std::endl
			<< "mFREE_INSTANCE_FUNCTION_FORCED (GenReceival" << myClassName << ")" << std::endl
			<< "mPNMPI_REGISTRATIONPOINT_FUNCTION (GenReceival" << myClassName << ")" << std::endl
			<< "" << std::endl
			<< "GenReceival" << myClassName << "::GenReceival" << myClassName << " (const char* instanceName)" << std::endl
			<< "    : ModuleBase<GenReceival" << myClassName << ", I_PlaceReceival> (instanceName), myTempRecords(), myWaitingForwards(), myProfiler (NULL)" << std::endl
			<< "{" << std::endl
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
			<< "    assert (subModInstances.size() == " << myCommMods.size() + myAnalysisMods.size() + myDownCommMods.size() + extraMods<< ");" << std::endl;

	//1) comm modules
	std::map <int, std::string>::iterator commIter;
	int i = 0;
	for (commIter = myCommMods.begin(); commIter != myCommMods.end(); commIter++, i++)
	{
		int id = commIter->first;
		std::string name = commIter->second;

		mySourceOut	<< "    myCStrats[" << i << "] = (I_CommStrategyUp*) subModInstances[" << i << "];" << std::endl;
	}

	int downIndex = 0;
	for (commIter = myDownCommMods.begin(); commIter != myDownCommMods.end(); commIter++, i++, downIndex++)
	{
	    int id = commIter->first;
	    std::string name = commIter->second;

	    mySourceOut << "    myDownCStrats[" << downIndex << "] = (I_CommStrategyDown*) subModInstances[" << i << "];" << std::endl;
	}

	//2) analyses
	std::list <ModuleInfo*>::iterator anIter;
	for (anIter = myAnalysisMods.begin(); anIter != myAnalysisMods.end(); anIter++, i++)
	{
		ModuleInfo *info = *anIter;

		mySourceOut << "    analysis" << info->index << " = (" << info->datatype << "*) subModInstances[" << i << "];" << std::endl;
	}

	//2b) profiler
	if (myProfiling)
	    mySourceOut << "    myProfiler = (I_Profiler*) subModInstances[" << i << "];" << std::endl;

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
	    mySourceOut << "    usec_gti_timeout = 0;" << std::endl;
	    mySourceOut << "    count_gti_timeout = 0;" << std::endl;
	}

	mySourceOut
			<< "}" << std::endl
			<< std::endl
			<< "uint64_t GenReceival" << myClassName << "::getUsecTime (void)" <<  std::endl
			<< "{" << std::endl
			<< "    struct timeval t;" << std::endl
			<< "    gettimeofday(&t, NULL);" << std::endl
			<< "    return t.tv_sec * 1000000 + t.tv_usec;" << std::endl
			<< "}" << std::endl
			<< "" << std::endl
			<< std::endl
			<< "GenReceival" << myClassName << "::~GenReceival" << myClassName << " (void)"  << std::endl
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
	i = 0;
	for (commIter = myDownCommMods.begin(); commIter != myDownCommMods.end(); commIter++, i++)
	{
	    int id = commIter->first;
	    std::string name = commIter->second;

	    mySourceOut
	    << "    myDownCStrats[" << i << "]->shutdown(GTI_FLUSH, GTI_SYNC);" << std::endl
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
	        mySourceOut << "   myProfiler->reportReceivalAnalysisTime (\"" << anFnIter->second.info->name << "\", \"" << anFnIter->second.function << "\", " << getProfilingVariableName(&(anFnIter->second)) << ", " << getProfilingCountVariableName(&(anFnIter->second)) << ");" << std::endl;
	    }

	    mySourceOut << "   myProfiler->reportUpCommTime (usec_communicating, count_communicating);" << std::endl;
	    mySourceOut << "   myProfiler->reportTimeoutTime (usec_gti_timeout, count_gti_timeout);" << std::endl;

	    mySourceOut << "    destroySubModuleInstance ((I_Module*) myProfiler);" << std::endl;
	}

	mySourceOut
			<< "}" << std::endl
			<< std::endl
			<< "GTI_RETURN GenReceival" << myClassName << "::getUpwardsCommunications (std::list<I_CommStrategyUp*> *ups)" << std::endl
			<< "{" << std::endl
	        << "    if (!ups) return GTI_SUCCESS;" << std::endl;

	i = 0;
	for (commIter = myCommMods.begin(); commIter != myCommMods.end(); commIter++, i++)
	{
	    mySourceOut
	        << "    ups->push_back (myCStrats[" << i << "]);" << std::endl;
	}

	mySourceOut
	        << "    return GTI_SUCCESS;" << std::endl
			<< "}" << std::endl
			<< std::endl
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
			<< "}" << std::endl << std::endl
			<< "" << std::endl
			<< "GTI_RETURN free_reused_buf (void* free_data, uint64_t num_bytes, void* buf)" << std::endl
			<< "{" << std::endl
			<< "    if (!free_data)" << std::endl
			<< "    {" << std::endl
			<< "        delete[] ((char*)buf);" << std::endl
			<< "        return GTI_SUCCESS;" << std::endl
			<< "    }" << std::endl
			<< std::endl
			<< "    ReusedBufInfo *bufInfo = (ReusedBufInfo*) free_data;" << std::endl
			<< "    bufInfo->refCount = bufInfo->refCount - 1;" << std::endl
			<< "    if (bufInfo->refCount == 0)" << std::endl
			<< "    {" << std::endl
			<< "        if (bufInfo->buf_free_function)" << std::endl
			<< "            bufInfo->buf_free_function (bufInfo->data, num_bytes, buf);" << std::endl
			<< "        delete bufInfo;" << std::endl
			<< "    }" << std::endl
			<< std::endl
			<< "    return GTI_SUCCESS;" << std::endl
			<< "}" << std::endl << std::endl
			<< "" << std::endl;

	//==Add the timeOutReductions function
	mySourceOut
		<< "GTI_RETURN GenReceival" << myClassName << "::timeOutReductions (void)" << std::endl
		<< "{" << std::endl;

	//Start timing (if profiling)
	if (myProfiling)
	{
	    mySourceOut
	        << "uint64_t tStart_gti_timeout = getUsecTime();" << std::endl;
	}

	//First, execute the forwards the we enqueued as waiting
	mySourceOut
		<< "   std::map<I_ChannelId*, std::pair<uint64_t, void*> >::iterator forwardIter;" << std::endl
		<< "   for (forwardIter = myWaitingForwards.begin(); forwardIter != myWaitingForwards.end(); forwardIter++)" << std::endl
		<< "   {" << std::endl
		<< "      executeForward (forwardIter->first, forwardIter->second.first, forwardIter->second.second, false);" << std::endl
		<< "   }" << std::endl
		<< "   myWaitingForwards.clear ();" << std::endl
		<< std::endl;


	//Second, notify the reductions of the timeout
	for (anIter = myAnalysisMods.begin(); anIter != myAnalysisMods.end(); anIter++)
		{
			ModuleInfo *info = *anIter;

			if (!info->isReduction && !info->listensToTimeouts)
				continue;

			mySourceOut << "   analysis" <<info->index << "->timeout ();" << std::endl;
		}

	//Stop timing (if profiling)
	if (myProfiling)
	{
	    mySourceOut
	        << "usec_gti_timeout += getUsecTime() - tStart_gti_timeout;" << std::endl
	        << "count_gti_timeout += 1;" << std::endl;
	}

	mySourceOut
		<< std::endl
		<< "   return GTI_SUCCESS;" << std::endl
		<< "}" << std::endl
		<< std::endl;

	//==Add the timeOutReductions function
	mySourceOut
		<< "GTI_RETURN GenReceival" << myClassName << "::triggerContinuous (uint64_t usecSinceLastTrigger)" << std::endl
		<< "{" << std::endl;

	//Start timing (if profiling)
	if (myProfiling)
	{
		mySourceOut
			<< "uint64_t tStart_gti_timeout = getUsecTime();" << std::endl;
	}

	//Notify every continuous analysis
	for (anIter = myAnalysisMods.begin(); anIter != myAnalysisMods.end(); anIter++)
	{
		ModuleInfo *info = *anIter;

		if (!info->continuous)
			continue;

		mySourceOut << "   analysis" <<info->index << "->trigger (usecSinceLastTrigger);" << std::endl;
	}

	/**
	 * We account this as timeout time, not ideal, but the closest thing we have immediately.
	 */
	//Stop timing (if profiling)
	if (myProfiling)
	{
		mySourceOut
		<< "usec_gti_timeout += getUsecTime() - tStart_gti_timeout;" << std::endl
		<< "count_gti_timeout += 1;" << std::endl;
	}

	mySourceOut
	<< std::endl
	<< "   return GTI_SUCCESS;" << std::endl
	<< "}" << std::endl
	<< std::endl;

	//Start the Receive Record funcion
	mySourceOut
			<< "GTI_RETURN GenReceival" << myClassName << "::ReceiveRecord (" << std::endl
			<< "            void* buf," << std::endl
			<< "            uint64_t num_bytes," << std::endl
			<< "            void* free_data," << std::endl
			<< "            GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)," << std::endl
			<< "            uint64_t *numRemainingClients," << std::endl
			<< "            I_ChannelId *recordChannelId," << std::endl
            << "            bool *outWasChannelIdSuspended," << std::endl
            << "            std::list<I_ChannelId*> *outChannelIdsToOpen" << std::endl
			<< "            )" << std::endl
			<< "{" << std::endl;

	//Start the big UID switch
	std::string getUidCode;
	myImpl->returnUidFromSerialized ("buf", &getUidCode);

	mySourceOut
			<< "    switch (" << getUidCode << ")" << std::endl
			<< "    {" << std::endl;

	//Start the big switch in getUpdatedChannelId too
	chanFunc
		<< "GTI_RETURN GenReceival" << myClassName << "::getUpdatedChannelId (" << std::endl
		<< "    		void* buf," << std::endl
		<< "    		uint64_t num_bytes," << std::endl
		<< "    		uint64_t channel," << std::endl
		<< "         uint64_t numChannels," << std::endl
		<< "    		I_ChannelId** pOutChannelId," << std::endl
		<< "         bool* pOutIsFinalizer," << std::endl
		<< "         bool* pOutIsOutOfOrder" << std::endl
		<< "    		)" << std::endl
		<< "{"
		<< "    switch (" << getUidCode << ")" << std::endl
		<< "    {" << std::endl;

	//Start the ReceiveIntraRecord function
	intraFunc
	    << "GTI_RETURN GenReceival" << myClassName << "::ReceiveIntraRecord (" << std::endl
	    << "void* buf," << std::endl
	    << "uint64_t num_bytes," << std::endl
	    << "void* free_data," << std::endl
	    << "GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)," << std::endl
	    << "uint64_t channel" << std::endl
	    << ")" << std::endl
	    << "{" << std::endl
	    << "    switch (" << getUidCode << ")" << std::endl
	    << "    {" << std::endl;

	//Start the ReceiveDownRecord function
	downFunc
	    << "GTI_RETURN  GenReceival" << myClassName << "::ReceiveBroadcastRecord (" << std::endl
        << "            void* buf," << std::endl
        << "            uint64_t num_bytes," << std::endl
        << "            void* free_data," << std::endl
        << "            GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)," << std::endl
        << "            bool *pOutWasFinalizeEvent" << std::endl
        << ")" << std::endl
        << "{" << std::endl
        << "    switch (" << getUidCode << ")" << std::endl
        << "    {" << std::endl;

	//Stuff for header
	myHeaderOut
			<< "#include <stdio.h>" << std::endl
			<< "#include <stdlib.h>" << std::endl
			<< "#include \"GtiDefines.h\"" << std::endl
			<< "#include \"GtiMacros.h\"" << std::endl
			<< "#include \"GtiEnums.h\"" << std::endl
			<< "#include \"GtiTypes.h\"" << std::endl
			<< "#include \"I_Module.h\"" << std::endl
			<< "#include \"ModuleBase.h\"" << std::endl
			<< "#include \"I_PlaceReceival.h\"" << std::endl
			<< "#include \"GtiChannelId.h\"" << std::endl
			<< "#include \"I_Profiler.h\"" << std::endl
			<< "#include <iostream>" << std::endl
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
			<< "{" << std::endl
			<< "" << std::endl
			<< "    struct ReusedBufInfo" << std::endl
			<< "    {" << std::endl
			<< "        int refCount;" << std::endl
			<< "        void* data;" << std::endl
			<< "        GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf);" << std::endl
			<< "    };" << std::endl
			<< "" << std::endl
			<< "    class GenReceival" << myClassName << " : public ModuleBase<GenReceival" << myClassName << ", I_PlaceReceival>" << std::endl
			<< "    {" << std::endl
			<< "    public:" << std::endl
			<< "       std::map<void*,void*> myTempRecords;" << std::endl
			<< "       std::map<I_ChannelId*, std::pair<uint64_t, void*>,I_ChannelIdComp > myWaitingForwards;" << std::endl
			<< std::endl;

	//1) comm modules
	if (!myCommMods.empty())
	    myHeaderOut	<< "        I_CommStrategyUp* myCStrats[" << myCommMods.size() << "];" << std::endl;
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
	    myHeaderOut << "        uint64_t usec_gti_timeout;" << std::endl;
	    myHeaderOut << "        uint64_t count_gti_timeout;" << std::endl;
	}

	myHeaderOut
			<< "" << std::endl
			<< "    public:" << std::endl
			<< "        /**" << std::endl
			<< "         * Constructor." << std::endl
			<< "         * Sets up the this module with all its sub-modules." << std::endl
			<< "         * @param instanceName name of this module instance." << std::endl
			<< "         */" << std::endl
			<< "        GenReceival" << myClassName << " (const char* instanceName);" << std::endl
			<< "" << std::endl
			<< "        /**" << std::endl
			<< "         * Destructor." << std::endl
			<< "         */" << std::endl
			<< "        ~GenReceival" << myClassName << " (void);" << std::endl
			<< "" << std::endl
			<< "" << std::endl
			<< "        uint64_t getUsecTime (void);" <<  std::endl
			<< "" << std::endl
			<< "        /**" << std::endl
			<< "        * @see I_PlaceReceival::ReceiveRecord" << std::endl
			<< "        */" << std::endl
			<< "        GTI_RETURN ReceiveRecord (" << std::endl
			<< "            void* buf," << std::endl
			<< "            uint64_t num_bytes," << std::endl
			<< "            void* free_data," << std::endl
			<< "            GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)," << std::endl
			<< "            uint64_t *numRemainingClients," << std::endl
			<< "            I_ChannelId *recordChannelId," << std::endl
			<< "            bool *outWasChannelIdSuspended," << std::endl
			<< "            std::list<I_ChannelId*> *outChannelIdsToOpen" << std::endl
			<< "            );" << std::endl
			<< std::endl
			<< "        GTI_RETURN getUpdatedChannelId (" << std::endl
			<< "        		void* buf," << std::endl
			<< "       		uint64_t num_bytes," << std::endl
			<< "       		uint64_t channel," << std::endl
			<< "             uint64_t numChannels," << std::endl
			<< "        		I_ChannelId** pOutChannelId," << std::endl
			<< "             bool* pOutIsFinalizer," << std::endl
			<< "             bool* pOutIsOutOfOrder" << std::endl
			<< "       		);" << std::endl
			<< std::endl
			<< "        GTI_RETURN timeOutReductions (void);" << std::endl
			<< std::endl
			<< "        GTI_RETURN triggerContinuous (uint64_t usecSinceLastTrigger);" << std::endl
			<< std::endl
			<< "        bool executeForward (I_ChannelId* chanIdCopy, uint64_t uid, void* record, bool avoidReducibleForwards);" << std::endl
			<< std::endl
			<< "        GTI_RETURN ReceiveIntraRecord (" << std::endl
            << "             void* buf," << std::endl
            << "             uint64_t num_bytes," << std::endl
            << "             void* free_data," << std::endl
            << "             GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf)," << std::endl
            << "             uint64_t channel" << std::endl
            << "             );" << std::endl
            << std::endl
            << "        GTI_RETURN getUpwardsCommunications (std::list<I_CommStrategyUp*> *ups);" << std::endl
            << std::endl
            << "        GTI_RETURN ReceiveBroadcastRecord (" << std::endl
            << "            void* buf," << std::endl
            << "            uint64_t num_bytes," << std::endl
            << "            void* free_data," << std::endl
            << "            GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf),"
            << "            bool *pOutWasFinalizeEvent);" << std::endl
			<< "    };" << std::endl
			<< "}" << std::endl;

	//Write beginning of the forward function
	forwardFunc
		<< "bool GenReceival" << myClassName << "::executeForward (I_ChannelId* chanIdCopy, uint64_t uid, void* record, bool avoidReducibleForwards)" << std::endl
		<< "{" << std::endl
		<< "    switch (uid)" << std::endl
		<< "    {" <<std::endl;

	return true;
}

//=============================
// readReceival
//=============================
bool ReceivalGenerator::readReceival (SpecificationNode node)
{
	SpecificationNode child, subchild;
	std::string
		name,
		orderString,
		recordDefinitionCode,
		recordPDefinitionCode,
		recordDeserializeCode,
		recordAllocCode,
		recordFreeCode,
		recordDeallocCode;
	gti::I_RecordType *pRecord;
	int uid;
	std::list<std::string> args, arrayArgs;
	bool isFinalizer = false, isNotifyFinalize=false, isOutOfOrder = false;
	std::string isFinalizerString, isNotifyFinalizeString, isOutOfOrderString;
	bool isWrapAcross = false, isWrapDown = false;
	std::string isWrapAcrossString, isWrapDownString;
	std::string recordPointerType;
	bool hasReduction = false;

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

	//==Optional attribute: notify-finalize
	if (node.getAttribute("notify-finalize", &isNotifyFinalizeString))
	{
	    if (isNotifyFinalizeString == "yes")
	        isNotifyFinalize = true;

	    if (isNotifyFinalizeString != "yes" && isNotifyFinalizeString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"notify-finalize\" attribute with an unknown value \"" << isNotifyFinalizeString << "\", valid values are \"yes\" and \"no\"." << std::endl;
	        return false;
	    }
	}

	//==Optional attribute: out-of-order
	if (node.getAttribute("out-of-order", &isOutOfOrderString))
	{
	    if (isOutOfOrderString == "yes")
	        isOutOfOrder = true;

	    if (isOutOfOrderString != "yes" && isOutOfOrderString != "no")
	    {
	        std::cerr << "|  |-->Error: a call uses the \"out-of-order\" attribute with an unknown value \"" << isOutOfOrderString << "\", valid values are \"yes\" and \"no\"." << std::endl;
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

	//==Child: call-name
	child = node.findChildNodeNamedOrErr("call-name", "|  |-->Error: a receival node has no \"call-name\" child.");
	if (!child) return false;
	name = child.getNodeContent();

	//==Child: order
	child = node.findChildNodeNamedOrErr("order", "|  |-->Error: a receival node has no \"order\" child.");
	if (!child) return false;
	orderString = child.getNodeContent();

	//==Child: record
	child = node.findChildNodeNamedOrErr("record", "|  |-->Error: a receival node has no \"record\" child.");
	if (!child) return false;

	if (!readRecord (child, &uid, &pRecord, &args, &arrayArgs))
		return false;

	pRecord->createInstancePointer("pRecordIn", &recordPDefinitionCode);
	pRecord->allocInstance("pRecordIn", &recordAllocCode);
	pRecord->deserialize ("buf", "num_bytes", "(*pRecordIn)", &recordDeserializeCode);
	pRecord->getPointerType(&recordPointerType);
	pRecord->deallocInstance("pRecordIn", &recordDeallocCode);
	pRecord->freeInstance("(*pRecordIn)", &recordFreeCode);

	//==Print start of case for executeForward function
	forwardFunc
		<< "        /*** " << name << ":" << orderString << " ***/" << std::endl
		<< "        case " << uid << ":" << std::endl
		<< "        {" << std::endl
		<< "            " << recordPDefinitionCode << ";" << std::endl
		<< "            pRecordIn = (" << recordPointerType << ") record;" << std::endl
		<< "            " << std::endl
		<< "" << std::endl;

	//==Print start of case to channel id function
	if (!isWrapAcross && !isWrapDown)
	{
        chanFunc
            << "        /*** " << name << ":" << orderString << " ***/" << std::endl
            << "        case " << uid << ":" << std::endl
            << "        {" << std::endl;

        if (isOutOfOrder)
            chanFunc << "           if (pOutIsOutOfOrder) *pOutIsOutOfOrder = true;" << std::endl;
        else
            chanFunc << "           if (pOutIsOutOfOrder) *pOutIsOutOfOrder = false;" << std::endl;

        chanFunc
            << "            " << recordPDefinitionCode << ";" << std::endl
            << "            " << recordAllocCode << ";" << std::endl
            << "            " << std::endl
            << "            //Deserialize" << std::endl
            << "            " << recordDeserializeCode << std::endl
            << "" << std::endl;

        //==Create the channel id, update it, update it back into the record
        chanFunc
            <<  "        GtiChannelId<" << myCIdNum64s << ", " << myCIdNumLevels << ", " << myCIdBitsPerChannel << "> *channelId = new GtiChannelId<" << myCIdNum64s << ", " << myCIdNumLevels << ", " << myCIdBitsPerChannel << "> (" << myCIdFromLevel + 1 << ");" << std::endl
            <<  "        if (pOutChannelId) *pOutChannelId = channelId;" << std::endl;

        //initialize channel id
        for (int n64 = 0; n64 < myCIdNum64s; n64++)
        {
            std::string accessCode;
            char temp[128];
            char temp2[128];
            if (orderString == "pre")
                sprintf (temp, "%d", myCIdStartIndexPre + n64);
            else
                sprintf (temp, "%d", myCIdStartIndexPost + n64);
            sprintf (temp2, "%d", n64);

            std::string base = myCIdArgumentBaseName;
            if (n64 == myCIdNum64s - 1)
            {
                base.resize(base.length()-1);
                base.append("Strided_");
            }

            pRecord->returnArgument("(*pRecordIn)", base +  temp, &accessCode);

            chanFunc
                << "        channelId->set64 (" << temp2 << ", " << accessCode << ");" << std::endl;
        }

        //update channel id
        chanFunc
            << "        channelId->setSubId (" << myCIdFromLevel << ", channel);" << std::endl
            << "        channelId->setSubIdNumChannels (" << myCIdFromLevel << ", numChannels);" << std::endl;
        if (isFinalizer)
            chanFunc << "if (pOutIsFinalizer) *pOutIsFinalizer = true;" << std::endl;
        else
            chanFunc << "if (pOutIsFinalizer) *pOutIsFinalizer = false;" << std::endl;

        //update new channel id into record
        for (int n64 = 0; n64 < myCIdNum64s; n64++)
        {
            std::string accessCode;
            char temp[128];
            char temp2[128];
            if (orderString == "pre")
                sprintf (temp, "%d", myCIdStartIndexPre + n64);
            else
                sprintf (temp, "%d", myCIdStartIndexPost + n64);
            sprintf (temp2, "%d", n64);

            std::string base = myCIdArgumentBaseName;
            if (n64 == myCIdNum64s - 1)
            {
                base.resize(base.length()-1);
                base.append("Strided_");
            }

            pRecord->writeArgument("(*pRecordIn)", base + temp, (std::string)"channelId->get64(" + temp2 + ")" , &accessCode);

            chanFunc
                << "        " << accessCode << ";" << std::endl;
        }

        //Store record in map
        chanFunc
            << "        myTempRecords.insert (std::make_pair(buf, pRecordIn));" << std::endl
            << "        } /*End case: " << name << ":" << orderString << "*/" << std::endl
            << "        break;" << std::endl;
	}

	//==Print intra record receival function
	if (isWrapAcross)
	{
	    intraFunc
	    << "        /*** " << name << ":" << orderString << " ***/" << std::endl
	    << "        case " << uid << ":" << std::endl
	    << "        {" << std::endl
	    << "            " << recordPDefinitionCode << ";" << std::endl
	    << "            " << recordAllocCode << ";" << std::endl
	    << "            " << std::endl
	    << "            //Deserialize" << std::endl
	    << "            " << recordDeserializeCode << std::endl
	    << "" << std::endl;
	}
	else if (isWrapDown)
	{
	    downFunc
	    << "        /*** " << name << ":" << orderString << " ***/" << std::endl
	    << "        case " << uid << ":" << std::endl
	    << "        {" << std::endl;

	    if (isNotifyFinalize)
	        downFunc << "        if (pOutWasFinalizeEvent) *pOutWasFinalizeEvent = true;" << std::endl;

	    downFunc
            << "            " << recordPDefinitionCode << ";" << std::endl
            << "            " << recordAllocCode << ";" << std::endl
            << "            " << std::endl
            << "            //Deserialize" << std::endl
            << "            " << recordDeserializeCode << std::endl
            << "" << std::endl;
	}
	else
	{
        //==Print start of case (only needed for the regular record receival)
        mySourceOut
            << "        /*** " << name << ":" << orderString << " ***/" << std::endl
            << "        case " << uid << ":" << std::endl
            << "        {" << std::endl
            << "            " << recordPDefinitionCode << ";" << std::endl
            << "            std::map<void*,void*>::iterator recIter = myTempRecords.find (buf);" << std::endl
            << "            assert (recIter != myTempRecords.end()); /*Error, GetUpdatedChannelId was not called beforehand for this channel id!*/" << std::endl
            << "            pRecordIn = (" << recordPointerType << ") recIter->second;" << std::endl
            << "            myTempRecords.erase (recIter);" << std::endl
            << "            " << std::endl
            << "" << std::endl;
	}

	//==Child: actions and subchilds
	child = node.findChildNodeNamedOrErr("actions", "|  |-->Error: a receival node has no \"actions\" child.");
	if (!child) return false;

	//=Subchild: exec-analysis*
	subchild = child.findChildNodeNamed("exec-analysis");
	SpecificationNode temp = child.findChildNodeNamed("exec-analysis");;

	//Determine whether we have a reduction to execute
	hasReduction = false;
	while (temp)
	{
		AnalysisFunction info;
		if (!preReadExecAnalysis (temp, &info, NULL))
			return false;

		if (info.info->isReduction)
			hasReduction = true;

		//next
		temp = temp.findSiblingNamed("exec-analysis");
	}

	//Copy the channel id if we have a reduction
	if (hasReduction && !isWrapAcross && !isWrapDown)
	{
		mySourceOut << "            I_ChannelId* chanIdCopy = recordChannelId->copy();" << std::endl;
	}

    //=If finalizer: Only execute analyses and forwarding if this is the last required shutdown
    // which is really going to shutdown the place
    if (isFinalizer && !isWrapAcross && !isWrapDown)
    {
        mySourceOut
            << "    if (*numRemainingClients == 1)" << std::endl
            << "    {" << std::endl;
    }

	//Print the exec analysis
	while (subchild)
	{
		AnalysisFunction info;
		bool supportsReduction;

		if (!preReadExecAnalysis (subchild, &info, &supportsReduction))
			return false;

		if (info.info->isReduction || !hasReduction || isWrapAcross || isWrapDown)
		{
		    if (isWrapAcross)
		    {
		        //For intra events we do things in a seperate receive record function
		        if (!printExecAnalysis (intraFunc, false, subchild, uid, pRecord, args, arrayArgs, true, hasReduction || info.info->isReduction))
		            return false;
		    }
		    else if (isWrapDown)
		    {
		        //For down events we do things in a seperate receive record function
		        if (!printExecAnalysis (downFunc, false, subchild, uid, pRecord, args, arrayArgs, true, hasReduction || info.info->isReduction))
		            return false;
		    }
		    else
		    {
		        //Usually in the regular receive Record function
		        if (!printExecAnalysis (mySourceOut, false, subchild, uid, pRecord, args, arrayArgs))
		            return false;
		    }
		}
		else
		{
			//We use the value of "supportsReduction" for the if value here,
			//only if the analysis supports the reduction we must avoid calling
			//the analysis in cases where the reduction was successful.
			if (!printExecAnalysis (forwardFunc, supportsReduction, subchild, uid, pRecord, args, arrayArgs, hasReduction || info.info->isReduction))
				return false;
		}

		//next
		subchild = subchild.findSiblingNamed("exec-analysis");
	}

	//=Subchild: forwarding
	subchild = child.findChildNodeNamedOrErr("forwarding", "|  |-->Error: a actions node has no \"forwarding\" child.");
	if (!child) return false;

	std::list<int> avoidedCommIds;
	/*
	 * TODO: currently avoiding the recreation of the incoming record is disables
	 * Reason: we can't update the channel id in th the incoming buffer, so we must
	 * not forward it ! (The false value in the argument list indicates this ...)
	 */
	if (!printForwarding (subchild, "", forwardFunc, false, uid, &avoidedCommIds, pRecord, args, arrayArgs, "avoidReducibleForwards"))
	    return false;

	if (!hasReduction || isWrapAcross || isWrapDown)
	{
		//We have no reduction here -> just do the forward and be done
	    if (isWrapAcross)
	    {
	        intraFunc
	        << std::endl
	        << "            //Execute the forwarding now" << std::endl
	        << "            executeForward (NULL, " << uid << ", pRecordIn, false);" << std::endl
	        << std::endl;
	    }
	    else if (isWrapDown)
	    {
	        downFunc
	        << std::endl
	        << "            //Execute the forwarding now" << std::endl
	        << "            executeForward (NULL, " << uid << ", pRecordIn, false);" << std::endl
	        << std::endl;
	    }
	    else
	    {
	        mySourceOut
	        << std::endl
	        << "            //Execute the forwarding now" << std::endl
	        << "            executeForward (recordChannelId, " << uid << ", pRecordIn, false);" << std::endl
	        << std::endl;
	    }
	}
	else
	{
		//We have a reduction here -> Switch based on its return value, handle returned values of reduction
		mySourceOut
			<< std::endl
			<< "switch (redReturn)" << std::endl
			<< "{" << std::endl
			<< "    case GTI_ANALYSIS_SUCCESS:" << std::endl
			<< "    {" << std::endl
			//call execute Forward for channel ids in opened list AND for this record, use true for avoid reducible !
			<< "        std::list<I_ChannelId*>::iterator openedIter;" << std::endl
			<< "        for (openedIter = outChannelIdsToOpen->begin(); openedIter != outChannelIdsToOpen->end(); openedIter++)" << std::endl
			<< "        {" << std::endl
			//<< "            std::cout << \"Finished\" << (*openedIter)->toString() << std::endl;" << std::endl
			<< "            std::map<I_ChannelId*, std::pair<uint64_t, void*> >::iterator pos = myWaitingForwards.find (*openedIter);" << std::endl
			<< "            assert (pos != myWaitingForwards.end());" << std::endl
			<< "            executeForward (pos->first, pos->second.first, pos->second.second, true);" << std::endl
			<< "            myWaitingForwards.erase (pos);" << std::endl
			<< "        }" << std::endl
			<< "        executeForward (chanIdCopy, " << uid << ", pRecordIn, true);" << std::endl
			//<< "        std::cout << \"Finished\" << chanIdCopy->toString() << std::endl;" << std::endl
			<< "        delete (chanIdCopy);" << std::endl
			<< "    }" << std::endl
			<< "    break;" << std::endl
			<< "    case GTI_ANALYSIS_WAITING:" << std::endl
			<< "    {" << std::endl
			//add the copied channel id and the record to the map of waiting forwards, set outWasChannelIdSuspended to true
			<< "        myWaitingForwards.insert (std::make_pair (chanIdCopy, std::make_pair(" << uid << ", pRecordIn)));" << std::endl
			//<< "        std::cout << \"Enqueued\" << chanIdCopy->toString() << std::endl;" << std::endl
			<< "        *outWasChannelIdSuspended = true;" << std::endl
			<< "    }" << std::endl
			<< "    break;" << std::endl
			<< "    case GTI_ANALYSIS_IRREDUCIBLE:" << std::endl
			<< "    {" << std::endl
			//call execute Forward for channel ids in opened list AND for this record, use false for avoid reducible !
			<< "        std::list<I_ChannelId*>::iterator openedIter;" << std::endl
			<< "        for (openedIter = outChannelIdsToOpen->begin(); openedIter != outChannelIdsToOpen->end(); openedIter++)" << std::endl
			<< "        {" << std::endl
			<< "            std::map<I_ChannelId*, std::pair<uint64_t, void*> >::iterator pos = myWaitingForwards.find (*openedIter);" << std::endl
			<< "            assert (pos != myWaitingForwards.end());" << std::endl
			<< "            executeForward (pos->first, pos->second.first, pos->second.second, false);" << std::endl
			<< "            myWaitingForwards.erase (pos);" << std::endl
			<< "        }" << std::endl
			<< "        executeForward (chanIdCopy, " << uid << ", pRecordIn, false);" << std::endl
			<< "        delete (chanIdCopy);" << std::endl
			<< "    }" << std::endl
			<< "    break;" << std::endl
			<< "    case GTI_ANALYSIS_FAILURE:" << std::endl
			<< "    {" << std::endl
			//print something and assert (?) TODO: what to do ? Also check this for regular analyses in the wrapper as well!
			<< "        std::cerr << \"ERROR: a reduction returned a failure, aborting.\" << std::endl;" << std::endl
			<< "        assert(0);" << std::endl
			<< "    }" << std::endl
			<< "    break;" << std::endl
			<< "}" << std::endl
			<< std::endl;
	}

	//Add free code to forward function, in order to free the incoming record
	forwardFunc
		<< std::endl
		<< "//Delete the incoming record" << std::endl
		<< recordFreeCode << std::endl
		<< recordDeallocCode << std::endl;

	//Handle forwarding of the received record
//	if (avoidedCommIds.size() == 0)
//	{
		//incoming buffer not reused -> free it
		std::stringstream tempStream;
		tempStream
		    << "//Free incoming buffer directly" << std::endl
			<< "(*buf_free_function) (free_data, num_bytes, buf);" << std::endl;

		if (!isWrapAcross && !isWrapDown)
		    mySourceOut << tempStream.str();
		else if (isWrapAcross)
		    intraFunc << tempStream.str();
		else if (isWrapDown)
		    downFunc << tempStream.str();
//	}
//	else
//	{
//		/*Must not happen, See comment above!*/
//		assert (0);
//
//		mySourceOut
//			<< "//Forward incoming buffer" << std::endl
//			<< "ReusedBufInfo *newBufInfo = new ReusedBufInfo;" << std::endl
//			<< "newBufInfo->data = free_data;" << std::endl
//			<< "newBufInfo->refCount = " << avoidedCommIds.size() << ";" << std::endl
//			<< "newBufInfo->buf_free_function = buf_free_function;" << std::endl;
//
//		std::list<int>::iterator commIdIter;
//		for (commIdIter = avoidedCommIds.begin(); commIdIter != avoidedCommIds.end(); commIdIter++)
//		{
//			mySourceOut
//				<< "//To: " << *commIdIter << std::endl
//				<< "myCStrats[" << *commIdIter << "]->send(buf, num_bytes, newBufInfo, free_reused_buf);" << std::endl;
//		}
//	}

	//==Handle the shutdown
	if (isFinalizer)
	{
		//End if that disables forwarding if not necessary
		mySourceOut
			<< "    }" << std::endl
			<< "    else" << std::endl
			<< "    {" << std::endl
			<< "        //Free incoming buffer directly" << std::endl
  		    << "        (*buf_free_function) (free_data, num_bytes, buf);" << std::endl
  		    << "        //Delete the incoming record" << std::endl
  		  	<< recordFreeCode << std::endl
  		  	<< recordDeallocCode << std::endl
			<< "    }" << std::endl
			<< std::endl;

		//Reduce number of remaining processes
		mySourceOut
			<< "    *numRemainingClients = *numRemainingClients -1;" << std::endl
			<< std::endl;
	}

	//==Print end of case
	forwardFunc
		<< "        }" << std::endl
		<< "        break;" << std::endl << std::endl;

	if (isWrapAcross)
	{
	    intraFunc
	    << "        }" << std::endl
	    << "        break;" << std::endl << std::endl;
	}
	else if (isWrapDown)
	{
	    downFunc
	    << "        }" << std::endl
	    << "        break;" << std::endl << std::endl;
	}
	else
	{
	    mySourceOut
	    << "        }" << std::endl
	    << "        break;" << std::endl << std::endl;
	}
	if (pRecord)
	    delete pRecord;

	return true;
}

//=============================
// printExecAnalysis
//=============================
bool ReceivalGenerator::printExecAnalysis (
		std::ostream &out,
		bool isForForwardWithReduction,
		SpecificationNode node,
		int uid,
		gti::I_RecordType *pRecord,
		std::list<std::string> args,
		std::list<std::string> arrayArgs,
        bool isForWrapAcross,
        bool gotAReduction)
{
    static int x = 0;

	SpecificationNode child, subchild;
	AnalysisFunction info;
	std::list<std::string>::iterator argIter;

	//==Child: analysis-id
	preReadExecAnalysis (node, &info, NULL);

	//Start timing (if profiling)
	if (myProfiling)
	{
	    out
	        << "uint64_t tStart_" << getProfilingVariableName(&info) << x <<" = getUsecTime();" << std::endl;
	}

	//==Add if if this execution is added to a the executeForward function and must not be executed if avoidReducibleForwards is true
	if (isForForwardWithReduction)
		out
			<< "            if (!avoidReducibleForwards)" << std::endl
			<< "            {" << std::endl;

	//Take care of the return value if this is a reduction
	out << "            //exec-analysis" << std::endl;
	if (info.info->isReduction)
	{
		out
			<< "            GTI_ANALYSIS_RETURN redReturn = ";
	}

	out << "            analysis" << info.info->index << "->" << info.function << "(";

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
			out << ", ";
		if (isFirst)
			isFirst= false;

		std::string argName = subchild.getNodeContent();
		std::string getCode;

		if (!getArgumentAccessCode (argName, pRecord, args, arrayArgs, &getCode))
			return false;

		//print it
		out << getCode;

		//next
		subchild = subchild.findSiblingNamed("input");
	}

	//If it is a reduction, we have to add the extra information for reductions
	if (info.info->isReduction && !isForWrapAcross)
	{
		if (!isFirst)
			out << ", ";

		out << "chanIdCopy, outChannelIdsToOpen";
	}
	else if (info.needsChannelId)
	{
		if (!isFirst)
			out << ", ";

		if (gotAReduction || isForWrapAcross)
			out << "chanIdCopy";
		else
			out << "recordChannelId";
	}

	out << ");" << std::endl << std::endl;

	//==Close the if for avoidReducibleForwards
	if (isForForwardWithReduction)
		out
			<< "            } /*!avoidReducibleForwards*/" << std::endl;

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
        out
            << "if (myProfiler->getLastWrapperEntryTime() > tStart_" << getProfilingVariableName(&info) << x<< ")" << std::endl
            << "   " << getProfilingVariableName(&info) << " += myProfiler->getLastWrapperEntryTime() - tStart_" << getProfilingVariableName(&info) << x << ";" << std::endl
            << "else" << std::endl
            << "   " << getProfilingVariableName(&info) << " += getUsecTime() - tStart_" << getProfilingVariableName(&info) << x << ";" << std::endl
            << getProfilingCountVariableName(&info) << " += 1;" << std::endl;
        x++;
    }

	return true;
}

//=============================
// readChannelId
//=============================
bool ReceivalGenerator::readChannelId (SpecificationNode node)
{
	std::string fromLevel, numLevels, num64s, bitsPerChannel, startIndexPre, startIndexPost;

	//==Attribute: fromLevel
	if (!node.getAttributeOrErr("fromLevel", "|->Error: the channel-id node has no \"fromLevel\" attribute.", &fromLevel))
		return false;
	myCIdFromLevel = atoi (fromLevel.c_str());

	//==Attribute: numLevels
	if (!node.getAttributeOrErr("numLevels", "|->Error: the channel-id node has no \"numLevels\" attribute.", &numLevels))
		return false;
	myCIdNumLevels = atoi (numLevels.c_str());

	//==Attribute: num64s
	if (!node.getAttributeOrErr("num64s", "|->Error: the channel-id node has no \"num64s\" attribute.", &num64s))
		return false;
	myCIdNum64s = atoi (num64s.c_str());

	//==Attribute: bitsPerChannel
	if (!node.getAttributeOrErr("bitsPerChannel", "|->Error: the channel-id node has no \"bitsPerChannel\" attribute.", &bitsPerChannel))
		return false;
	myCIdBitsPerChannel = atoi (bitsPerChannel.c_str());

	//==Attribute: idArgumentBaseName
	if (!node.getAttributeOrErr("idArgumentBaseName", "|->Error: the channel-id node has no \"idArgumentBaseName\" attribute.", &myCIdArgumentBaseName))
		return false;

	//==Attribute: startIndexPre
	if (!node.getAttributeOrErr("startIndexPre", "|->Error: the channel-id node has no \"startIndexPre\" attribute.", &startIndexPre))
		return false;
	myCIdStartIndexPre = atoi (startIndexPre.c_str());

	//==Attribute: startIndexPost
	if (!node.getAttributeOrErr("startIndexPost", "|->Error: the channel-id node has no \"startIndexPost\" attribute.", &startIndexPost))
		return false;
	myCIdStartIndexPost = atoi (startIndexPost.c_str());

	return true;
}

//=============================
// myGetRecordName
//=============================
std::string ReceivalGenerator::myGetRecordName (void)
{
	return "(*pRecordIn)";
}

//=============================
// preReadExecAnalysis
//=============================
bool ReceivalGenerator::preReadExecAnalysis (SpecificationNode node, AnalysisFunction *outInfo, bool *pOutSupportsReduction)
{
	SpecificationNode child;
	std::string analysisId;
	std::string supportsReductionStr;
	bool supportsReduction = false;

	//==Optional attribute: supports-reduction
	if (node.getAttribute("supports-reduction", &supportsReductionStr))
	{
		if (supportsReductionStr == "yes")
			supportsReduction = true;
	}

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

	if (outInfo)
		*outInfo = myAnalyses[analysisId];

	if (pOutSupportsReduction)
		*pOutSupportsReduction = supportsReduction;

	return true;
}

/*EOF*/
