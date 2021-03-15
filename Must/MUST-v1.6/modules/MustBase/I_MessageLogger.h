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
 * @file I_MessageLogger.h
 *       @see I_MessageLogger.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_CreateMessage.h"
#include "I_ChannelId.h"

#include "BaseIds.h"

#ifndef I_MESSAGELOGGER_H
#define I_MESSAGELOGGER_H

/**
 * Interface for logging messages.
 * Implementations may logg messages however they like.
 */
class I_MessageLogger : public gti::I_Module
{
public:
	/**
	 * Logs a new message.
	 * @param msgId unique identifier for this type of message, e.g. 7 for the "Buffer not allocated message".
	 * @param int true if this has an associated location (global if not).
	 * @param pId parallel Id.
	 * @param lId location id.
	 * @param msgType type of the message (error/warning/...) @see MustMessageType.
	 * @param text message text.
	 * @param textLen length of the text.
	 * @param numReferences number of extra location references used in the text.
	 * @param refPIds parallel ids of the extra locations, array of size numReferences.
	 * @param refLIds location ids of the extra locations, array of size numReferences.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN log (
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
    		) = 0;

    /**
     * Logs a new reduced (stride representation) message.
     * @param msgId unique identifier for this type of message, e.g. 7 for the "Buffer not allocated message".
     * @param pId parallel Id.
     * @param lId location id.
     * @param startRank first rank of the strided representation.
     * @param stride of the strided representation.
     * @param count of the strided representation.
     * @param msgType type of the message (error/warning/...) @see MustMessageType.
     * @param text message text.
     * @param textLen length of the text.
     * @param numReferences number of extra location references used in the text.
     * @param refPIds parallel ids of the extra locations, array of size numReferences.
     * @param refLIds location ids of the extra locations, array of size numReferences.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN logStrided (
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
    ) = 0;

}; /*class I_MessageLogger*/

/**
 * Copy of I_MessageLogger, with the difference that these interfaces here use an additional
 * channel id.
 */
class I_MessageLoggerCId : public gti::I_Module
{
public:
    /**
     * Logs a new message.
     * @see I_MessageLogger::log
     */
    virtual gti::GTI_ANALYSIS_RETURN log (
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
            gti::I_ChannelId *cId
            ) = 0;

    /**
     * Logs a new reduced (stride representation) message.
     * @see I_MessageLogger::logStrided
     */
    virtual gti::GTI_ANALYSIS_RETURN logStrided (
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
            gti::I_ChannelId *cId
    ) = 0;

}; /*class I_MessageLoggerCId*/


#endif /*I_MESSAGELOGGER_H*/
