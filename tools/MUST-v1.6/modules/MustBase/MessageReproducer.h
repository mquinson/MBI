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
 * @file MessageReproducer.h
 *       @see MUST::MessageReproducer.
 *
 *  @date 26.05.2014
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_MessageReproducer.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"
#include "I_CreateMessage.h"

#include <fstream>

#ifndef MESSAGEREPRODUCER_H
#define MESSAGEREPRODUCER_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_MessageReproducer that reads the format of MsgLoggerReproducer.
     */
    class MessageReproducer : public gti::ModuleBase<MessageReproducer, I_MessageReproducer>
    {
    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
            MessageReproducer (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~MessageReproducer (void);

			/**
			 * @see I_MessageReproducer::testForMatch.
			 */
			GTI_ANALYSIS_RETURN testForMatch (
			        uint64_t pId,
			        uint64_t lId
			    );

    protected:
			I_ParallelIdAnalysis *myPIdModule;
			I_LocationAnalysis *myLIdModule;
			I_CreateMessage *myLogger;

			typedef std::pair
                    <
                    std::pair <int /*rank*/, int /*occCount*/>,
                    std::string /*callName*/
                    > KeyType;

			typedef std::pair<std::string /*text*/, MustMessageType /*type*/> ValueType;

			std::map<KeyType, ValueType> myTriggers;

			/**
			 * Helper to read an existing message log for reproducing it.
			 */
			void readLog (void);

    }; /*class MessageReproducer */
} /*namespace MUST*/

#endif /*MESSAGEREPRODUCER_H*/
