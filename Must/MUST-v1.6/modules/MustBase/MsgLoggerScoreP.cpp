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
 * @file MsgLoggerScoreP.cpp
 *       @see MUST::MsgLoggerScoreP.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "MsgLoggerScoreP.h"

using namespace must;

mGET_INSTANCE_FUNCTION(MsgLoggerScoreP)
mFREE_INSTANCE_FUNCTION(MsgLoggerScoreP)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MsgLoggerScoreP)

//=============================
// Constructor
//=============================
MsgLoggerScoreP::MsgLoggerScoreP (const char* instanceName)
    : gti::ModuleBase<MsgLoggerScoreP, I_MessageLogger> (instanceName),
      myPIdModule (NULL),
      myLIdModule (NULL),
      myLog ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    myLIdModule = (I_LocationAnalysis*) subModInstances[0];
    myPIdModule = (I_ParallelIdAnalysis*) subModInstances[1];

    //Open output file
    myLog.open("MUST_Output.scorep");

    //Print the header
    myLog << "MPI-Rank;Function-Name;Function-Occurrence-Count;Message-Text;Message-Type" << std::endl;
}

//=============================
// Destructor
//=============================
MsgLoggerScoreP::~MsgLoggerScoreP (void)
{
	if (myLIdModule)
		destroySubModuleInstance ((I_Module*) myLIdModule);
	myLIdModule = NULL;

	if (myPIdModule)
		destroySubModuleInstance ((I_Module*) myPIdModule);
	myPIdModule = NULL;

	myLog.close();
}

//=============================
// log
//=============================
GTI_ANALYSIS_RETURN MsgLoggerScoreP::log (
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
GTI_ANALYSIS_RETURN MsgLoggerScoreP::logStrided (
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
    if (count == 0)
    {
        /**
         * @todo how will we handle global messages here?
         */
        return GTI_ANALYSIS_SUCCESS;
    }

    //1) RANK: We select the representative pId here for the rank
    myLog << myPIdModule->getInfoForId(pId).rank << ";";

    //2) FUNCTION-NAME:
    std::string callName = myLIdModule->getInfoForId(pId,lId).callName;
    /**
     * @todo below is for current CUDA wrapping workaround and should be
     * removed once we have a final solution.
     */
    std::string::size_type p = 0;
    p = callName.find ("my", p);
    if (p != std::string::npos)
        callName.replace (p,2,"");
    myLog << callName << ";";

    //3) OCCURRENCE-COUNT:
    myLog << myLIdModule->getOccurenceCount(lId) << ";";

    //4) TEXT:
    //replace all '\n' characters with "<br>" in the text, replace all ';' characters with ",:"
    std::string tempText (text); //copy the text

    do {
        p = tempText.find ('\n', p);
        if (p == std::string::npos)
            break;
        tempText.replace (p,1,"<br>");
    }while (p != std::string::npos);
    do {
        p = tempText.find (';', p);
        if (p == std::string::npos)
            break;
        tempText.replace (p,1,",:");
    }while (p != std::string::npos);

    //If this is a representative, tell the user:
    if (count > 1)
    {
        myLog
            << "Representative for ranks ";

        //CASE1: Continuous ranks
        if (stride == 1)
        {
            myLog << startRank << "-" << startRank + (count-1);
        }
        //CASE2: Strided ranks
        else
        {
            int last = startRank;
            for (int x = 0; x < count; x++)
            {
                if (last != startRank)
                    myLog << ", ";

                myLog << last;

                last += stride;

                if (x == 2 && count > 3)
                {
                    myLog << ", ..., " << startRank + (count - 1) * stride;
                    break;
                }
            }
        }

        myLog << ". ";
    }

    //Print the actual text
    myLog << tempText;

    //Add the references
    for (int i = 0; i < numReferences; i++)
    {
        myLog
                << " Reference "<<(i+1)<<": "
                << myLIdModule->toString(refPIds[i], refLIds[i])
                << "@"
                << myPIdModule->toString(refPIds[i])
                << "<br> ";
    }

    //End the field
    myLog << ";";

    //5) MESSAGE-TYPE:
    switch ((MustMessageType)msgType)
	{
	case MustErrorMessage:
	    myLog << "ERROR";
		break;
	case MustWarningMessage:
	    myLog << "WARNING";
		break;
	case MustInformationMessage:
	    myLog << "INFO";
		break;
	default:
	    myLog << "UNKNOWN";
	}

    //6) end the line
    myLog << std::endl;

    //7) Flush if necessary
	if ((MustMessageType)msgType == MustErrorMessage)
		myLog.flush();

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
