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
 * @file MsgLoggerStdOut.h
 *       @see MUST::MsgLoggerStdOut.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_MessageLogger.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"

#ifndef MSGLOGGERSTDOUT_H
#define MSGLOGGERSTDOUT_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_MessageLogger that prints all logged events to
     * std::cout.
     */
    class MsgLoggerStdOut : public gti::ModuleBase<MsgLoggerStdOut, I_MessageLogger>
    {
    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
			MsgLoggerStdOut (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~MsgLoggerStdOut (void);

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
					uint64_t* refLIds
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
			        uint64_t* refLIds
			);

    protected:
			I_ParallelIdAnalysis *myPIdModule;
			I_LocationAnalysis *myLIdModule;
    private:
            bool myLoggedWarnError;
    }; /*class MsgLoggerStdOut */
} /*namespace MUST*/

#endif /*MSGLOGGERSTDOUT_H*/
