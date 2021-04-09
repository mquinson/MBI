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
 * @file MsgLoggerReproduce.h
 *       @see MUST::MsgLoggerReproduce.
 *
 *  @date 26.05.2014
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_MessageLogger.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"

#include <fstream>

#ifndef MSGLOGGERREPRODUCE_H
#define MSGLOGGERREPRODUCE_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_MessageLogger that prints into a
     * special file format that enables reproducing these messages in a second run.
     */
    class MsgLoggerReproduce : public gti::ModuleBase<MsgLoggerReproduce, I_MessageLogger>
    {
    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
			MsgLoggerReproduce (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~MsgLoggerReproduce (void);

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

			/**
			 * Helper function that puts an entry into the actual log
			 */
		    bool logEntry (
		            bool isReference,
		            int rank,
		            std::string callName,
		            int repCount,
		            int msgType,
		            std::string text
		            );

    }; /*class MsgLoggerReproduce */
} /*namespace MUST*/

#endif /*MSGLOGGERREPRODUCE_H*/
