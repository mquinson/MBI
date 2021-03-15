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
 * @file MsgLoggerDdt.h
 *       @see MUST::MsgLoggerDdt.
 *
 *  @date 11.10.2012
 *  @author Joachim Protze
 */

#include "ModuleBase.h"
#include "I_MessageLogger.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"

#ifndef MSGLOGGERDDT_H
#define MSGLOGGERDDT_H

using namespace gti;

extern "C" void myDdtBreakpointFunctionError(const char* text);
extern "C" void myDdtBreakpointFunctionWarning(const char* text);
extern "C" void myDdtBreakpointFunctionInfo(const char* text);

namespace must
{
  /**
     * Implementation of I_MessageLogger that prints all logged events to
     * std::cout.
     */
    class MsgLoggerDdt : public gti::ModuleBase<MsgLoggerDdt, I_MessageLoggerCId>
    {
    public:
      /**
       * Constructor.
       * @param instanceName name of this module instance.
       */
      MsgLoggerDdt (const char* instanceName);

      /**
       * Destructor.
       */
      virtual ~MsgLoggerDdt (void);

      /**
       * @see I_MessageLogger::log.
       */
      GTI_ANALYSIS_RETURN log (
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
          I_ChannelId *cId
      );

      /**
       * @see I_MessageLogger::logStrided.
       */
      GTI_ANALYSIS_RETURN logStrided (
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
      );

    protected:
      I_ParallelIdAnalysis *myPIdModule;
      I_LocationAnalysis *myLIdModule;
    }; /*class MsgLoggerDdt */
} /*namespace MUST*/

#endif /*MSGLOGGERDDT_H*/
