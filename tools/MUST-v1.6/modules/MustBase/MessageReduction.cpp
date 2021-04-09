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
 * @file MessageReduction.cpp
 *       @see MUST::MessageReduction.
 *
 *  @date 05.08.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include <string.h>

#include "MessageReduction.h"

using namespace must;

mGET_INSTANCE_FUNCTION(MessageReduction)
mFREE_INSTANCE_FUNCTION(MessageReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MessageReduction)

//=============================
// Constructor
//=============================
MessageReduction::MessageReduction (const char* instanceName)
    : gti::ModuleBase<MessageReduction, I_MessageReduction> (instanceName),
      myPIdModule (NULL),
      myLIdModule (NULL),
      myNotify (NULL),
      myReps (),
      myGotFinish (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 3
    if (subModInstances.size() < NUM_MODS_REQUIRED)
    {
        std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
        assert (0);
    }
    if (subModInstances.size() > NUM_MODS_REQUIRED)
    {
        for (std::vector<I_Module*>::size_type i = NUM_MODS_REQUIRED; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    //save sub modules
    myLIdModule = (I_LocationAnalysis*) subModInstances[0];
    myPIdModule = (I_ParallelIdAnalysis*) subModInstances[1];
    myNotify = (I_FinishNotify*) subModInstances[2];

    myNotify->addListener(this);

    getWrapperFunction("handleNewMessageReduced", (GTI_Fct_t*)&myIntroduceMessage);
}

//=============================
// Destructor
//=============================
MessageReduction::~MessageReduction (void)
{
	if (myLIdModule)
		destroySubModuleInstance ((I_Module*) myLIdModule);
	myLIdModule = NULL;

	if (myPIdModule)
		destroySubModuleInstance ((I_Module*) myPIdModule);
	myPIdModule = NULL;

	if (myNotify)
	    destroySubModuleInstance ((I_Module*) myNotify);
	myNotify = NULL;

	myReps.clear();
}

//=============================
// timeout
//=============================
void MessageReduction::timeout (void)
{
    //Forward whatever we accumulated
    std::list<MessageRepresentation>::iterator iter;
    for (iter = myReps.begin(); iter != myReps.end(); iter++)
    {
        iter->forwardRepresentation(myIntroduceMessage);
    }
    myReps.clear();
}

//=============================
// finish
//=============================
void MessageReduction::finish (void)
{
    timeout ();
    myGotFinish = true;
}

//=============================
// reduce
//=============================
GTI_ANALYSIS_RETURN MessageReduction::reduce (
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
        gti::I_ChannelId *thisChannel,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!hasLocation)
        return reduceStrided (msgId, pId, lId, 0, 0, 0, msgType, text, textLen, numReferences, refPIds, refLIds, thisChannel, outFinishedChannels);

    return reduceStrided (msgId, pId, lId, myPIdModule->getInfoForId(pId).rank, 1, 1, msgType, text, textLen, numReferences, refPIds, refLIds, thisChannel, outFinishedChannels);
}

//=============================
// reduceStrided
//=============================
GTI_ANALYSIS_RETURN MessageReduction::reduceStrided (
        int msgId,
        uint64_t pIdRef,
        uint64_t lIdRef,
        int startRank,
        int stride,
        int count,
        int msgType,
        char *text,
        int textLen,
        int numReferences,
        uint64_t* refPIds,
        uint64_t* refLIds,
        gti::I_ChannelId *thisChannel,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    //If we already got a finish, do not try to aggregate anything!
    if (myGotFinish)
        return GTI_ANALYSIS_IRREDUCIBLE;

    //If we can do the aggregation
    std::string callname = "";
    if (count != 0)
        callname = myLIdModule->getInfoForId(pIdRef, lIdRef).callName;

    std::list<MessageRepresentation>::iterator iter;
    for (iter = myReps.begin(); iter != myReps.end(); iter++)
    {
        if (iter->belongsToRepresentation(msgId, callname, msgType, text, startRank, stride, count))
        {
            iter->addToRepresentation(startRank, stride, count);
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    myReps.push_back(
            MessageRepresentation (
                    msgId,
                    pIdRef,
                    lIdRef,
                    callname,
                    startRank,
                    stride,
                    count,
                    msgType,
                    text,
                    textLen,
                    numReferences,
                    refPIds,
                    refLIds));

    return GTI_ANALYSIS_SUCCESS;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//=============================
// MessageRepresentation
//=============================
MessageRepresentation::MessageRepresentation (
        int msgId,
        uint64_t pIdRef,
        uint64_t lIdRef,
        std::string callName,
        int startRank,
        int stride,
        int count,
        int msgType,
        char *text,
        int textLen,
        int numReferences,
        uint64_t* refPIds,
        uint64_t* refLIds)
: myMsgId (msgId),
  myPId (pIdRef),
  myLId (lIdRef),
  myCallName (callName),
  myMsgType (msgType),
  myText (text),
  myNumReferences (numReferences),
  myRefPIds (NULL),
  myRefLIds (NULL),
  myStrides ()
{
    //Copy references arrays
    if (numReferences > 0)
    {
        myRefPIds = new MustParallelId[myNumReferences];
        myRefLIds = new MustLocationId[myNumReferences];

        for (int i = 0; i < myNumReferences; i++)
        {
            myRefPIds[i] = refPIds[i];
            myRefLIds[i] = refLIds[i];
        }
    }

    //Add the strided ranks
    if (count > 0)
        addToRepresentation (startRank, stride, count);
}

//=============================
// MessageRepresentation
//=============================
MessageRepresentation::MessageRepresentation( const MessageRepresentation& other)
: myMsgId (other.myMsgId),
  myPId (other.myPId),
  myLId (other.myLId),
  myCallName (other.myCallName),
  myMsgType (other.myMsgType),
  myText (other.myText),
  myNumReferences (other.myNumReferences),
  myRefPIds (NULL),
  myRefLIds (NULL),
  myStrides (other.myStrides)
{
    //Copy references arrays
    if (myNumReferences > 0)
    {
        myRefPIds = new MustParallelId[myNumReferences];
        myRefLIds = new MustLocationId[myNumReferences];

        for (int i = 0; i < myNumReferences; i++)
        {
            myRefPIds[i] = other.myRefPIds[i];
            myRefLIds[i] = other.myRefLIds[i];
        }
    }
}

//=============================
// ~MessageRepresentation
//=============================
MessageRepresentation::~MessageRepresentation (void)
{
    if (myRefPIds) delete[] myRefPIds;
    myRefPIds = NULL;

    if (myRefLIds) delete[] myRefLIds;
    myRefLIds = NULL;
}

//=============================
// belongsToRepresentation
//=============================
bool MessageRepresentation::belongsToRepresentation (
                int msgId,
                std::string callName,
                int msgType,
                std::string text,
                int startRank,
                int stride,
                int count
                )
{
    //Compare basic properties
    if (msgId != myMsgId)
        return false;

    if (callName.compare(myCallName) != 0)
        return false;

    if (msgType != myMsgType)
        return false;

    if (text.compare(myText) != 0)
        return false;

    //Will the given ranks have basic compatibility with our ones?
    if (myStrides.empty())
        return true;

    std::map<int, std::pair<int, int> >::iterator sIter;
    for (sIter = myStrides.begin(); sIter != myStrides.end(); sIter++)
    {
        int curRank = sIter->first;
        int curStride = sIter->second.first;
        int curCount = sIter->second.second;

        //If both have an actual stride, we only accept an equal stride
        if (curStride != 1 && stride != 1 && curStride != stride)
            return false;

        //Do we overlap
        //a) this is stride=1
        if (curStride == 1)
        {
            //If we somehow overlap, we say no (irrespective of an actual overlap)
            if (curRank+(curCount-1) >= startRank && curRank <= startRank + (count-1)*stride)
                return false;
        }
        //b) other is stride=1
        else if (stride == 1)
        {
            //If we somehow overlap, we say no (irrespective of an actual overlap)
            if (startRank+(count-1) >= curRank && startRank <= curRank + (curCount-1)*curStride)
                return false;
        }
        //c) both have same (!=1) stride
        else
        {
            //If we somehow overlap, we say no (irrespective of an actual overlap)
            if (startRank+(count-1)*stride >= curRank && startRank <= curRank + (curCount-1)*curStride)
                return false;
        }
    }

    return true;
}

//=============================
// addToRepresentation
//=============================
void MessageRepresentation::addToRepresentation (
                int startRank,
                int stride,
                int count)
{
    //Never add an invalid count!
    if (count == 0)
        return;

    //Add
    myStrides[startRank] = std::make_pair(stride,count);
}

//=============================
// forwardRepresentation
//=============================
void MessageRepresentation::forwardRepresentation (
        handleNewMessageReducedP fNewMsg)
{
    /*
     * Rules:
     * - either strided representation uses stride of 1
     * - or both have same strides
     * - no overlaps (even just area overlap) between any two strides
     */
    std::map<int, std::pair<int, int> >::iterator sIter;

    bool have = false;
    int start, stride, count, newStart, newStride, newCount;
    std::map<int, std::pair<int, int> > newStrides;

    //==A) Try to merge the strides we accumulated
    for (sIter = myStrides.begin(); sIter != myStrides.end(); sIter++)
    {
        if (!have)
        {
            start = sIter->first;
            stride = sIter->second.first;
            count = sIter->second.second;
            have = true;
            continue;
        }

        newStart = sIter->first;
        newStride = sIter->second.first;
        newCount = sIter->second.second;

        if (stride == 1 && newStride == 1)
        {
            if (count == 1 && newCount == 1)
            {
                /*
                 * We got two single ranks, we can represent them with a strided representation
                 */
                //start = start;
                stride = newStart - start;
                count = 2;
            }
            else
            {
                /*
                 * Either one is not a single rank:
                 * Both intervals must fit next to each other without a gap
                 */
                if (newStart == start + count)
                {
                    count = count + newCount;
                }
                else
                {
                    newStrides[start] = std::make_pair(stride, count);
                    start = newStart;
                    stride = newStride;
                    count = newCount;
                    have = true;
                }
            }
        }
        else if (stride == 1)
        {
            if (count == 1 && start + newStride == newStart)
            {
                stride = newStride;
                count = newCount + 1;
            }
            else
            {
                newStrides[start] = std::make_pair(stride, count);
                start = newStart;
                stride = newStride;
                count = newCount;
                have = true;
            }
        }
        else if (newStride == 1)
        {
            if (newCount == 1 && start + count*stride == newStart)
            {
                count += 1;
            }
            else
            {
                newStrides[start] = std::make_pair(stride, count);
                start = newStart;
                stride = newStride;
                count = newCount;
                have = true;
            }
        }
        else
        {
            //Two equal strides
            if (newStart == start + count*stride)
            {
                count = count + newCount;
            }
            else
            {
                newStrides[start] = std::make_pair(stride, count);
                start = newStart;
                stride = newStride;
                count = newCount;
                have = true;
            }
        }
    }

    //Add the last stride
    if (have)
    {
        newStrides[start] = std::make_pair(stride, count);
    }

    //==B) Iterate over the new strides
    for (sIter = newStrides.begin(); sIter != newStrides.end(); sIter++)
    {
        if (fNewMsg)
            (*fNewMsg) (
                    myMsgId,
                    myPId,
                    myLId,
                    sIter->first,
                    sIter->second.first,
                    sIter->second.second,
                    myMsgType,
                    (char*)(void*)myText.c_str(), //Casting away a const, should be safe
                    strlen(myText.c_str())+1,
                    myNumReferences,
                    myRefPIds,
                    myRefLIds);
    }

    //==C) Are we a global message?
    if (myStrides.empty())
    {
        if (fNewMsg)
            (*fNewMsg) (
                    myMsgId,
                    myPId,
                    myLId,
                    0,
                    0,
                    0,
                    myMsgType,
                    (char*)(void*)myText.c_str(), //Casting away a const, should be safe
                    strlen(myText.c_str())+1,
                    myNumReferences,
                    myRefPIds,
                    myRefLIds);
    }
}

/*EOF*/
