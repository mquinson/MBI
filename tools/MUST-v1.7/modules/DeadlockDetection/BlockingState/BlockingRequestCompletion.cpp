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
 * @file BlockingRequestCompletion.cpp
 *       @see must::BlockingRequestCompletion.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "BlockingRequestCompletion.h"
#include "BlockingState.h"

using namespace must;

MustRequestType request;
std::vector<MustRequestType> requests;


//=============================
// Constructor
//=============================
BlockingRequestCompletion::BlockingRequestCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)
: BlockingOp (state, pId, lId),
  myRequest (request),
  myRequests (),
  myIsInvalid (false)
{
    I_Request* rInfo = state->myRTrack->getRequest(pId, request);

    //We are already done if this is an unknown, NULL, or inactive request
    if (!rInfo || rInfo->isNull() || !rInfo->isActive() || rInfo->isProcNull())
        myIsInvalid = 1;
}

//=============================
// Constructor
//=============================
BlockingRequestCompletion::BlockingRequestCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                int count,
                MustRequestType* requests)
: BlockingOp (state, pId, lId),
  myRequest (0),
  myRequests (),
  myIsInvalid (false)
{
    myRequests.resize (count);
    int actualCount = 0;
    for (int i = 0; i < count; i++)
    {
        I_Request* rInfo = state->myRTrack->getRequest(pId, requests[i]);
        if (!rInfo || rInfo->isNull() || !rInfo->isActive() || rInfo->isProcNull())
            continue;

        myRequests[actualCount] = requests[i];
        actualCount++;
    }

    if (actualCount == 0)
        myIsInvalid = 1;
    else
        myRequests.resize (actualCount);
}

//=============================
// Destructor
//=============================
BlockingRequestCompletion::~BlockingRequestCompletion (void)
{
    myRequests.clear();
}

//=============================
// process
//=============================
PROCESSING_RETURN BlockingRequestCompletion::process (int rank)
{
    myState->applyNewCompletionUpdateOp(this);
    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN BlockingRequestCompletion::print (std::ostream &out)
{
    out << "Request completion update with ";
    if (myRequests.size () == 0)
        out << "1";
    else
        out << myRequests.size();

    out << " requests.";
    return GTI_SUCCESS;
}

//=============================
// offerMatchedSend
//=============================
bool BlockingRequestCompletion::offerMatchedSend (bool hasRequest, MustRequestType request)
{
    /*This is an update nothing that consumes anything at all*/
    return false;
}

//=============================
// offerMatchedReceive
//=============================
bool BlockingRequestCompletion::offerMatchedReceive (bool hasRequest, MustRequestType request)
{
    /*This is an update nothing that consumes anything at all*/
    return false;
}

//=============================
// offerMatchedCollective
//=============================
bool BlockingRequestCompletion::offerMatchedCollective (void)
{
    /*This is an update nothing that consumes anything at all*/
    return false;
}

//=============================
// canComplete
//=============================
bool BlockingRequestCompletion::canComplete (void)
{
    /*Good question, duno, guess so ;)*/
    return true;
}

//=============================
// isArray
//=============================
bool BlockingRequestCompletion::isArray (void)
{
    return myRequests.size () > 0;
}

//=============================
// getRequest
//=============================
MustRequestType BlockingRequestCompletion::getRequest (void)
{
    return myRequest;
}

//=============================
// getRequests
//=============================
std::vector<MustRequestType>* BlockingRequestCompletion::getRequests (void)
{
    return &myRequests;
}

//=============================
// isInvalid
//=============================
bool BlockingRequestCompletion::isInvalid (void)
{
    return myIsInvalid;
}

//=============================
// isMixedOp
//=============================
bool BlockingRequestCompletion::isMixedOp (void)
{
    return false;
}

//=============================
// getWaitType
//=============================
ArcType BlockingRequestCompletion::getWaitType (void)
{
    /*None in fact ...*/
    return ARC_AND;
}

//=============================
// mixedOpGetNumSubNodes
//=============================
int BlockingRequestCompletion::mixedOpGetNumSubNodes (void)
{
    return 0;
}

//=============================
// getWaitedForRanks
//=============================
std::list<int> BlockingRequestCompletion::getWaitedForRanks (
        std::list<std::string> *outLabels,
        std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *pReferences,
        std::map<I_Comm*, std::string> &commLabels)
{
    return std::list<int> ();
}

//=============================
// getSubNodeWaitedForRanks
//=============================
std::list<int> BlockingRequestCompletion::getSubNodeWaitedForRanks (
        int subId,
        std::string *outLabel,
        bool *outHasReference,
        MustParallelId *outPId,
        MustLocationId *outLId,
        std::map<I_Comm*, std::string> &commLabels)
{
    return std::list<int> ();
}

//=============================
// getSubNodeWaitedForRanks
//=============================
std::list<I_Comm*> BlockingRequestCompletion::getUsedComms (void)
{
    return std::list<I_Comm*> ();
}

//=============================
// copy
//=============================
BlockingOp* BlockingRequestCompletion::copy (void)
{
    return new BlockingRequestCompletion (this);
}

//=============================
// BlockingRequestCompletion (from existing)
//=============================
BlockingRequestCompletion::BlockingRequestCompletion (BlockingRequestCompletion* other)
 : BlockingOp (other->myState, other->myPId, other->myLId)
{
     //Init this
    myRequest = other->myRequest;

    std::vector<MustRequestType>::iterator iter;
    myRequests.resize (other->myRequests.size());
    int i = 0;
    for (iter = other->myRequests.begin(); iter != other->myRequests.end(); iter++, i++)
    {
        myRequests[i] = *iter;
    }

    myIsInvalid = other->myIsInvalid;
}

/*EOF*/
