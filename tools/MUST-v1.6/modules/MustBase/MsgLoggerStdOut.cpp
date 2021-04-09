/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file MsgLoggerStdOut.cpp
 *       @see MUST::MsgLoggerStdOut.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "MsgLoggerStdOut.h"

using namespace must;

mGET_INSTANCE_FUNCTION(MsgLoggerStdOut)
mFREE_INSTANCE_FUNCTION(MsgLoggerStdOut)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MsgLoggerStdOut)

//=============================
// Constructor
//=============================
MsgLoggerStdOut::MsgLoggerStdOut (const char* instanceName)
    : gti::ModuleBase<MsgLoggerStdOut, I_MessageLogger> (instanceName),
      myPIdModule (NULL),
      myLIdModule (NULL),
      myLoggedWarnError (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    myLIdModule = (I_LocationAnalysis*) subModInstances[0];
    myPIdModule = (I_ParallelIdAnalysis*) subModInstances[1];
}

//=============================
// Destructor
//=============================
MsgLoggerStdOut::~MsgLoggerStdOut (void)
{
	if (myLIdModule)
		destroySubModuleInstance ((I_Module*) myLIdModule);
	myLIdModule = NULL;

	if (myPIdModule)
		destroySubModuleInstance ((I_Module*) myPIdModule);
	myPIdModule = NULL;
	if (!myLoggedWarnError){

      std::cout << "MUST detected no MPI usage errors nor any suspicious behavior during this application run." << std::endl;
		std::cout.flush();
        }
}

//=============================
// log
//=============================
GTI_ANALYSIS_RETURN MsgLoggerStdOut::log (
          int msgId,
          int hasLocation,
          uint64_t pId,
          uint64_t lId,
          int msgType,
          char *text,
          int textLen,
          int numReferences,
          uint64_t* refPIds,
          uint64_t* refLIds)
{
    if (!hasLocation)
        return logStrided (msgId, pId, lId, 0, 0, 0, msgType, text, textLen, numReferences, refPIds, refLIds);

    return logStrided (msgId, pId, lId, myPIdModule->getInfoForId(pId).rank, 1, 1, msgType, text, textLen, numReferences, refPIds, refLIds);
}

//=============================
// logStrided
//=============================
GTI_ANALYSIS_RETURN MsgLoggerStdOut::logStrided (
					int msgId,
					uint64_t pId,
					uint64_t lId,
					int startRank,
					int stride,
					int count,
					int msgType,
					char *text,
					int textLen,
					int numReferences,
					uint64_t* refPIds,
					uint64_t* refLIds)
{

	switch ((MustMessageType)msgType)
	{
	case MustErrorMessage:
		std::cout << "Error";
		myLoggedWarnError = true;
		break;
	case MustWarningMessage:
		std::cout << "Warning";
		myLoggedWarnError = true;
		break;
	case MustInformationMessage:
		std::cout << "Information";
		break;
	default:
		std::cout << "Unknown";
	}

	if (count > 0)
	{
	    std::cout
	        << ": from: call " << myLIdModule->getInfoForId(pId,lId).callName << "@";

	    //CASE1: A single rank
	    if (count == 1)
	    {
#ifdef ELP_MODIFICATIONS
                ParallelInfo info = myPIdModule->getInfoForId(pId);
                std::cout << startRank << "(" << info.threadid << ")";
#else
                std::cout << startRank;
#endif
	    }
	    //CASE2: Continous ranks
	    else if (stride == 1)
	    {
	        std::cout << startRank << "-" << startRank + (count-1);
	    }
	    //CASE3: Strided ranks
	    else
	    {
	        int last = startRank;
	        for (int x = 0; x < count; x++)
	        {
	            if (last != startRank)
	                std::cout << ", ";

	            std::cout << last;

	            last += stride;

	            if (x == 2 && count > 3)
	            {
	                std::cout << ", ..., " << startRank + (count - 1) * stride;
	                break;
	            }
	        }
	    }

	    std::cout << ": ";
	}
	else
	{
	    std::cout << " global: ";
	}

	std::cout << text;

	std::cout
		<< " References";

//	if (count > 1)
	std::cout << " (possibly of representative)";

    std::cout << ": ";

	for (int i = 0; i < numReferences; i++)
	{
		std::cout
				<< "Reference "<<(i+1)<<": "
				<< myLIdModule->toString(refPIds[i], refLIds[i])
				<< "@"
				<< myPIdModule->toString(refPIds[i])
				<< "; ";
	}

	std::cout
		<< std::endl;

	if ((MustMessageType)msgType == MustErrorMessage)
		std::cout.flush();

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
