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
 * @file I_CreateMessage.h
 *       Interface for creating new log messages.
 *
 *  @date 19.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "BaseIds.h"

#include <list>

#ifndef I_CREATEMESSAGE_H
#define I_CREATEMESSAGE_H

enum MustMessageType
{
	MustErrorMessage = 0,
	MustWarningMessage = 1,
	MustInformationMessage = 2
};

/**
 * Interface that provides logging of new messages.
 * The messages are forwarded to a an implementation
 * of the I_MessageLogger interface, if one is present.
 *
 * Implementations use the wrapp-everywhere call
 * "handleNewMessage".
 */
class I_CreateMessage : public gti::I_Module
{
public:
	typedef std::list <std::pair<MustParallelId, MustLocationId> > RefListType;

	/**
	 * Creates a new message with location of origin.
	 * @param msgId unique identifier for this type of message, e.g. 7 for the "Buffer not allocated message".
	 * @param pId parallel Id.
	 * @param lId location id.
	 * @param msgType type of the message (error/warning/...).
	 * @param text message text.
	 * @param refLocations extra locations of parallel entities referenced in the text order of the list must be keept when printing, first item gets name "reference1", and so on.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN createMessage (
    		int msgId,
    		MustParallelId pId,
    		MustLocationId lId,
    		MustMessageType msgType,
    		std::string text,
    		std::list <std::pair<MustParallelId, MustLocationId> > refLocations = RefListType ()
    		) = 0;

    /**
     * Creates a global message.
     * @param msgId unique identifier for this type of message, e.g. 7 for the "Buffer not allocated message".
     * @param msgType type of the message (error/warning/...).
     * @param text message text.
     * @param refLocations extra locations of parallel entities referenced in the text order of the list must be keept when printing, first item gets name "reference1", and so on.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN createMessage (
    		int msgId,
    		MustMessageType msgType,
    		std::string text,
    		std::list <std::pair<MustParallelId, MustLocationId> > refLocations = RefListType ()
    ) = 0;

}; /*class I_CreateMessage*/

#endif /*I_CREATEMESSAGE_H*/
