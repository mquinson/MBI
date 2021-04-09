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
 * @file MsgLoggerDdt.cpp
 *       @see MUST::MsgLoggerDdt.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "MsgLoggerDdt.h"

using namespace must;

mGET_INSTANCE_FUNCTION(MsgLoggerDdt)
mFREE_INSTANCE_FUNCTION(MsgLoggerDdt)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MsgLoggerDdt)

//=============================
// Constructor
//=============================
MsgLoggerDdt::MsgLoggerDdt (const char* instanceName)
    : ModuleBase<MsgLoggerDdt, I_MessageLoggerCId> (instanceName),
      myPIdModule (NULL),
      myLIdModule (NULL)
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
MsgLoggerDdt::~MsgLoggerDdt (void)
{
  if (myLIdModule)
    destroySubModuleInstance ((I_Module*) myLIdModule);
  myLIdModule = NULL;

  if (myPIdModule)
    destroySubModuleInstance ((I_Module*) myPIdModule);
  myPIdModule = NULL;
}

//=============================
// log
//=============================
GTI_ANALYSIS_RETURN MsgLoggerDdt::log (
          int msgId,
          int hasLocation,
          uint64_t pId,
          uint64_t lId,
          int msgType,
          char *text,
          int textLen,
          int numReferences,
          uint64_t* refPIds,
          uint64_t* refLIds,
          I_ChannelId *cId)
{
    if (!hasLocation)
        return logStrided (msgId, pId, lId, 0, 0, 0, msgType, text, textLen, numReferences, refPIds, refLIds, cId);

    return logStrided (msgId, pId, lId, myPIdModule->getInfoForId(pId).rank, 1, 1, msgType, text, textLen, numReferences, refPIds, refLIds, cId);
}

//=============================
// logStrided
//=============================
GTI_ANALYSIS_RETURN MsgLoggerDdt::logStrided (
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
              uint64_t* refLIds,
              I_ChannelId *cId
      )
{
    int levelId;
    ModuleBase<MsgLoggerDdt, I_MessageLoggerCId>::getLevelId (&levelId);

    // On the application processes we can directly log anything
    if (levelId == 0)
    {
        switch ((MustMessageType)msgType)
        {
        case MustErrorMessage:
            myDdtBreakpointFunctionError(text);
            break;
        case MustWarningMessage:
            myDdtBreakpointFunctionWarning(text);
            break;
        case MustInformationMessage:
            myDdtBreakpointFunctionInfo(text);
            break;
        default:
            return GTI_ANALYSIS_SUCCESS;
        }

        return GTI_ANALYSIS_SUCCESS;
    }

    //Otherwise we should be on the root (layouts should not put this elsewhere)
    //FIRST: remove the local errors we could raise on the application processes
    if (cId && levelId == cId->getNumUsedSubIds())
    {
        //This is a process local message, we already logged it
        return GTI_ANALYSIS_SUCCESS;
    }

    //We got a non-local message, raise attention to the tool user!
    static bool raisedAttention = false;
    if (!raisedAttention)
    {
        raisedAttention = true;
        std::stringstream textStream;
        textStream
                << "MUST detected an MPI usage error that involves multiple MPI processes. "
                << "Since MUST detects such errors on extra compute resources, we can't immediately issue a breakpoint when the error occured. "
                << "Use a second application run in order to breakpoint on the MPI calls that are involved in the detected error. "
                << "Add the command line switch \"--must:reproduce\" to the mustrun command of this second run. "
                << "(MUST wrote a log file that details the MPI calls that cause the error, the second run reads this log and issues breakpoints on these calls) "
                << "The below text summarizes the error message of MUST, this message will only appear once and MUST will silently log all further errors that involve multiple processes. "
                << "<br>"
                << "Text of the original message:"
                << "<br>"
                << text;

        myDdtBreakpointFunctionInfo(textStream.str().c_str());
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// myDdtBreakpointFunctionError
//=============================
extern "C" __attribute__ ((noinline)) void myDdtBreakpointFunctionError(const char* text)
{
	asm("");
#ifdef MUST_DEBUG
    std::cout << "MUST: " << getpid () << " has breakpoint for an error with text " << text << std::endl;
#endif
}

//=============================
// myDdtBreakpointFunctionWarning
//=============================
extern "C" __attribute__ ((noinline)) void myDdtBreakpointFunctionWarning(const char* text)
{
	asm("");
#ifdef MUST_DEBUG
    std::cout << "MUST: " << getpid () << " has breakpoint for a warning with text " << text << std::endl;
#endif
}

//=============================
// myDdtBreakpointFunctionInfo
//=============================
extern "C" __attribute__ ((noinline)) void myDdtBreakpointFunctionInfo(const char* text)
{
	asm("");
#ifdef MUST_DEBUG
    std::cout << "MUST: " << getpid () << " has breakpoint for an information with text " << text << std::endl;
#endif
}

/*EOF*/
