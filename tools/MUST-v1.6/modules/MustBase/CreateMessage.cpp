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
 * @file CreateMessage.cpp
 *       @see MUST::CreateMessage.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "BaseApi.h"

#include "CreateMessage.h"

using namespace must;

mGET_INSTANCE_FUNCTION(CreateMessage)
mFREE_INSTANCE_FUNCTION(CreateMessage)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CreateMessage)

//=============================
// GInfo::operator <
//=============================
bool CreateMessage::GInfo::operator < (const GInfo& other) const
{
    if (msgId < other.msgId)
        return 1;
    if (msgId > other.msgId)
        return 0;

    if (msgType < other.msgType)
        return 1;

    return 0;
}

//=============================
// LInfo::operator <
//=============================
bool CreateMessage::LInfo::operator < (const LInfo& other) const
{
    if (msgId < other.msgId)
        return 1;
    if (msgId > other.msgId)
        return 0;

    if (msgType < other.msgType)
        return 1;
    if (msgType > other.msgType)
        return 0;

    if (pId < other.pId)
        return 1;
    if (pId > other.pId)
        return 0;

    if ((lId & 0x00000000FFFFFFFF) < (other.lId & 0x00000000FFFFFFFF)) //Kill the occurence count, that one is of no interest here!
        return 1;

    return 0;
}

//=============================
// Constructor
//=============================
CreateMessage::CreateMessage (const char* instanceName)
    : gti::ModuleBase<CreateMessage, I_CreateMessage> (instanceName),
      myGMsgs (),
      myLMsgs ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();
    //No sub modules needed ...
}

//=============================
// createMessage
//=============================
GTI_ANALYSIS_RETURN CreateMessage::createMessage (
		int msgId,
		MustMessageType msgType,
		std::string text,
		std::list <std::pair<MustParallelId, MustLocationId> > refLocations
		)

{
    //Determine whether we filter out this message
    GInfo info;
    info.msgId = msgId;
    info.msgType = msgType;
    std::map<GInfo, int>::iterator pos = myGMsgs.find (info);
    if (pos != myGMsgs.end())
    {
        pos->second = pos->second +1;
        return GTI_ANALYSIS_SUCCESS;
    }

    //We do not filter, create the message
    myGMsgs.insert(std::make_pair (info, 1));
    return createMessage (msgId, 0, 0, 0, msgType, text, refLocations);
}

//=============================
// createMessage
//=============================
GTI_ANALYSIS_RETURN CreateMessage::createMessage (
		int msgId,
		MustParallelId pId,
		MustLocationId lId,
		MustMessageType msgType,
		std::string text,
		std::list <std::pair<MustParallelId, MustLocationId> > refLocations
		)
{
    //Determine whether we filter out this message
    LInfo info;
    info.msgId = msgId;
    info.msgType = msgType;
    info.pId = pId;
    info.lId = lId;
    std::map<LInfo, int>::iterator pos = myLMsgs.find (info);
    if (pos != myLMsgs.end())
    {
        pos->second = pos->second +1;
        return GTI_ANALYSIS_SUCCESS;
    }

    //We do not filter, create the message
    myLMsgs.insert(std::make_pair (info, 1));
	return createMessage (msgId, 1, pId, lId, msgType, text, refLocations);
}


//=============================
// createMessage
//=============================
GTI_ANALYSIS_RETURN CreateMessage::createMessage (
		int msgId,
		int hasLocation,
		MustParallelId pId,
		MustLocationId lId,
		MustMessageType msgType,
		std::string text,
		std::list <std::pair<MustParallelId, MustLocationId> > &refLocations
		)
{
	//Call handleNewMessage
    handleNewMessageP fP;

	if (getWrapperFunction ("handleNewMessage", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
	{
		uint64_t *refPIds = NULL,
					  *refLIds = NULL;

		if (refLocations.size() > 0)
		{
			refPIds = new uint64_t [refLocations.size()];
			refLIds = new uint64_t [refLocations.size()];

			std::list <std::pair<MustParallelId, MustLocationId> >::iterator iter;

			int i = 0;
			for (iter = refLocations.begin(); iter != refLocations.end(); iter++, i++)
			{
				refPIds[i] = iter->first;
				refLIds[i] = iter->second;
			}
		}

		//We cast the cons of the char away here, we can't use const char* in the definintion of the
		//API call, but it will not modify the text
		(*fP) (msgId, hasLocation, pId, lId, (int) msgType, (char*)(void*)text.c_str(), text.length()+1, refLocations.size(), refPIds, refLIds);

		if (refPIds) delete [] refPIds;
		if (refLIds) delete [] refLIds;
	}
	else
	{
		std::cout << "ERROR: failed to get \"handleNewMessage\" function pointer from wrapper, load the MUST base API, logging is not possible as a result!" << std::endl;
	}

	return GTI_ANALYSIS_SUCCESS;
}


/*EOF*/
