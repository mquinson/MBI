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
 * @file MsgLoggerReproduce.cpp
 *       @see MUST::MsgLoggerReproduce.
 *
 *  @date 26.05.2014
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "MsgLoggerReproduce.h"

using namespace must;

mGET_INSTANCE_FUNCTION(MsgLoggerReproduce)
mFREE_INSTANCE_FUNCTION(MsgLoggerReproduce)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MsgLoggerReproduce)

//=============================
// Constructor
//=============================
MsgLoggerReproduce::MsgLoggerReproduce (const char* instanceName)
    : gti::ModuleBase<MsgLoggerReproduce, I_MessageLogger> (instanceName),
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
    myLog.open("MUST_Output.repro");

    //Print the header
    myLog << "MPI-Rank;Function-Name;Function-Occurrence-Count;Message-Text;Message-Type" << std::endl;
}

//=============================
// Destructor
//=============================
MsgLoggerReproduce::~MsgLoggerReproduce (void)
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
GTI_ANALYSIS_RETURN MsgLoggerReproduce::log (
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
GTI_ANALYSIS_RETURN MsgLoggerReproduce::logStrided (
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

    //a) build the message text
    //replace all '\n' characters with "<br>" in the text, replace all ';' characters with ",:"
    std::string tempText (text); //copy the text
    std::stringstream textStream;
    std::string::size_type p = 0;

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
        textStream
        << "Representative for ranks ";

        //CASE1: Continuous ranks
        if (stride == 1)
        {
            textStream << startRank << "-" << startRank + (count-1);
        }
        //CASE2: Strided ranks
        else
        {
            int last = startRank;
            for (int x = 0; x < count; x++)
            {
                if (last != startRank)
                    textStream << ", ";

                textStream << last;

                last += stride;

                if (x == 2 && count > 3)
                {
                    textStream << ", ..., " << startRank + (count - 1) * stride;
                    break;
                }
            }
        }

        textStream << ". ";
    }

    //Print the actual text
    textStream << tempText;

    //Add the references
    for (int i = 0; i < numReferences; i++)
    {
        textStream
        << " Reference "<<(i+1)<<": "
        << myLIdModule->toString(refPIds[i], refLIds[i])
        << "@"
        << myPIdModule->toString(refPIds[i])
        << "<br> ";
    }

    //Log the location associated with the message!
    logEntry (
            false,
            myPIdModule->getInfoForId(pId).rank,
            myLIdModule->getInfoForId(pId,lId).callName,
            myLIdModule->getOccurenceCount(lId),
            msgType,
            textStream.str()
            );

    //Log all references!
    for (int i = 0; i < numReferences; i++)
    {
        /*
         * We use the information message type for the references, they add information, thats it
         */
        int curMsgType = (int) MustInformationMessage;

        /*
         * However, some references are crucial and make up an important piece of the overall message.
         * For these we adapt the message type below:
         * @todo This embeds logic of checks into a logger and should be stored elsewhere in the future.
         */
        switch ((MustMessageIdNames)msgId)
        {
        case MUST_ERROR_OVERLAPPED_SEND:
        case MUST_ERROR_OVERLAPPED_RECV:
        case MUST_ERROR_TYPEMATCH_MISSMATCH:
        case MUST_ERROR_TYPEMATCH_MISSMATCH_BYTE:
        case MUST_ERROR_TYPEMATCH_LENGTH:
        case MUST_ERROR_COLLECTIVE_CALL_MISSMATCH:
        case MUST_ERROR_COLLECTIVE_OP_MISSMATCH:
        case MUST_ERROR_COLLECTIVE_ROOT_MISSMATCH:
        case MUST_ERROR_COUNTS_ARRAYS_DIFFER:
            if (i == 0)
                curMsgType = msgType;
            break;
        case MUST_ERROR_DEADLOCK:
            curMsgType = msgType;
            break;
        default:
        	/*Just to get rid of compiler warnings*/
        	break;
        }

        logEntry (
                true,
                myPIdModule->getInfoForId(refPIds[i]).rank,
                myLIdModule->getInfoForId(refPIds[i], refLIds[i]).callName,
                myLIdModule->getOccurenceCount(refLIds[i]),
                curMsgType,
                textStream.str()
            );
    }

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// logEntry
//=============================
bool MsgLoggerReproduce::logEntry (
            bool isReference,
            int rank,
            std::string callName,
            int repCount,
            int msgType,
            std::string text
            )
{

    myLog << rank << ";" << callName << ";" << repCount << ";";

    if (isReference)
    {
        myLog << "This breakpoint reflects a reference within the following message:<br>";
    }

    myLog << text << ";";

    //MESSAGE-TYPE:
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

    //End the line
    myLog << ";" << std::endl;

    //Flush if necessary
    if ((MustMessageType)msgType == MustErrorMessage)
        myLog.flush();

    return true;
}

/*EOF*/
