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
 * @file CreateMessage.h
 *       Implementation for I_CreateMessage.
 *
 *  @date 19.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_CreateMessage.h"

#ifndef CREATEMESSAGE_H
#define CREATEMESSAGE_H

using namespace gti;

namespace must
{
	/**
     * Implementation for the I_CreateMessage interface.
     * This is used to create new logging events
     */
    class CreateMessage : public gti::ModuleBase<CreateMessage, I_CreateMessage>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		CreateMessage (const char* instanceName);

    		/**
    		 * @see I_MessageLogger::createMessage.
    		 */
    		GTI_ANALYSIS_RETURN createMessage (
    				int msgId,
    				MustParallelId pId,
    				MustLocationId lId,
    				MustMessageType msgType,
    				std::string text,
    				std::list <std::pair<MustParallelId, MustLocationId> > refLocations
    				);

    		/**
    		 * @see I_MessageLogger::createMessage.
    		 */
    		GTI_ANALYSIS_RETURN createMessage (
    				int msgId,
    				MustMessageType msgType,
    				std::string text,
    				std::list <std::pair<MustParallelId, MustLocationId> > refLocations
    		);

    protected:
    		/**
    		 * Internal function to propagate messages with location along with
    		 * global messages.
    		 * @see I_MessageLogger::createMessage.
    		 */
    		GTI_ANALYSIS_RETURN createMessage (
    				int msgId,
    				int hasLocation,
    				MustParallelId pId,
    				MustLocationId lId,
    				MustMessageType msgType,
    				std::string text,
    				std::list <std::pair<MustParallelId, MustLocationId> > &refLocations
    				);

    		class GInfo
    		{
    		public:
    		    int msgId;
    		    MustMessageType msgType;

    		    bool operator < (const GInfo& other) const;
    		};

    		class LInfo
    		{
    		public:
    		    int msgId;
    		    MustMessageType msgType;
    		    MustParallelId pId;
    		    MustLocationId lId;

    		    bool operator < (const LInfo& other) const;
    		};

    		std::map<GInfo, int> myGMsgs; /**< Count of global messages of a certain type, used to filter.*/
    		std::map<LInfo, int> myLMsgs; /**< Count of local messages of a certain type, used to filter.*/

    }; /*class CreateMessage */
} /*namespace MUST*/

#endif /*CREATEMESSAGE_H*/
