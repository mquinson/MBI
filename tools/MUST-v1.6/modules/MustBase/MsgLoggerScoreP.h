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
 * @file MsgLoggerScoreP.h
 *       @see MUST::MsgLoggerScoreP.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_MessageLogger.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"

#include <fstream>

#ifndef MSGLOGGERSCOREP_H
#define MSGLOGGERSCOREP_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_MessageLogger that prints into a
     * semicolumn seperated file for ScoreP.
     */
    class MsgLoggerScoreP : public gti::ModuleBase<MsgLoggerScoreP, I_MessageLogger>
    {
    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
			MsgLoggerScoreP (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~MsgLoggerScoreP (void);

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

			std::ofstream myLog;
    }; /*class MsgLoggerScoreP */
} /*namespace MUST*/

#endif /*MSGLOGGERSCOREP_H*/
