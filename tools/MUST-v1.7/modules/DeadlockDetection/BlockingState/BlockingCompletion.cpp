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
 * @file BlockingCompletion.cpp
 *       @see must::BlockingCompletion.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "BlockingCompletion.h"
#include "BlockingState.h"

#include <sstream>

using namespace must;

//=============================
// Constructor -- RequestWaitInfo
//=============================
RequestWaitInfo::RequestWaitInfo()
 : isCompleted (false),
   request (0),
   info (NULL)
{
    //Nothing to do
}

//=============================
// Destructor -- RequestWaitInfo
//=============================
RequestWaitInfo::~RequestWaitInfo()
{
    if (info) delete info;
    info = NULL;
}

//=============================
// copy -- RequestWaitInfo
//=============================
RequestWaitInfo RequestWaitInfo::copy ()
{
    RequestWaitInfo ret;
    ret.isCompleted = isCompleted;
    ret.request = request;

    if (info)
    {
        ret.info = new P2PInfo ();
        /**
         * @todo this is not completely correct, but should work as intended atm.
         * We copy an I_Comm here, it might not be available at a later point,
         * however as we do a full copy also of the P2PMatch state when we do this
         * copy, this I_Comm should not cease to exist.
         */
        *(ret.info) = *info;
    }
    else
    {
        ret.info = NULL;
    }

    return ret;
}

//=============================
// Constructor -- WfgInfo
//=============================
WfgInfo::WfgInfo ()
 : type (ARC_AND),
   isMixed (false),
   subNodeToReq ()
{
    //Nothing to do
}

//=============================
// Constructor
//=============================
BlockingCompletion::BlockingCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)
: BlockingOp (state, pId, lId),
  myRequest (),
  myRequests (),
  minReq (0),
  maxReq (0),
  myIsForAll (true),
  myNumCompleted (0),
  myMatchIndex (-1),
  hadAnActualCompletion (false),
  myWfgInfo (NULL)
{
    I_Request* rInfo = state->myRTrack->getRequest(pId, request);

    myRequest.request = request;

    //We are already done if this is an unknown, NULL, or inactive request
    if (!rInfo || rInfo->isNull() || !rInfo->isActive() || rInfo->isProcNull())
    {
        myNumCompleted = 1;
        myRequest.isCompleted = true;
    }

    if (rInfo && rInfo->isProcNull())
        hadAnActualCompletion = true;
}

//=============================
// Constructor
//=============================
BlockingCompletion::BlockingCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                int count,
                MustRequestType *requests,
                bool isForAll,
                bool hadProcNullReqs)
: BlockingOp (state, pId, lId),
  myRequest (),
  myRequests (),
  minReq (0),
  maxReq (0),
  myIsForAll (isForAll),
  myNumCompleted (0),
  myMatchIndex (-1),
  hadAnActualCompletion (false),
  myWfgInfo (NULL)
{
    myRequests.resize(count);
    for (int i = 0; i < count;i++)
    {
        /**
         * We can remove the request tracker querry here as this is already done by
         * the CompletionConditon preconditioner, so we only get valid and active
         * requests here that are not targeting MPI_PROC_NULL
         */
        //I_Request* rInfo = state->myRTrack->getRequest(pId, requests[i]);

        myRequests[i].request = requests[i];

        /*//We are already done if this is an unknown, NULL, or inactive request
        if (!rInfo || rInfo->isNull() || !rInfo->isActive() || rInfo->isProcNull())
        {
            //Is an invalid/inactive request
            if (isForAll || rInfo->isProcNull()) //an invalid/inactive/null request won't complete an MPI_Waitany/some call!!!!
                myNumCompleted++;

            if (rInfo->isProcNull())
                hadAnActualCompletion = true;

            myRequests[i].isCompleted = true;
        }
        else
        {*/
            //Is a valid request
            if (i == 0 || requests[i] < minReq)
                minReq = requests[i];

            if (i == 0 || requests[i] > maxReq)
                maxReq = requests[i];
        /*}*/
    }

    if (hadProcNullReqs && !isForAll)
    {
        myNumCompleted++;
        hadAnActualCompletion = true;
    }

    //If count was 0 we will think this was a single request not an array, we prepare for that
    if (count == 0)
    {
        myNumCompleted = 1;
        myRequest.isCompleted = true;
    }
}

//=============================
// Destructor
//=============================
BlockingCompletion::~BlockingCompletion (void)
{
    myRequests.clear();

    if (myWfgInfo) delete myWfgInfo;
    myWfgInfo = NULL;
}


//=============================
// process
//=============================
PROCESSING_RETURN BlockingCompletion::process (int rank)
{
    myState->applyNewCompletionOp(this);
    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN BlockingCompletion::print (std::ostream &out)
{
    out << "Completion with ";

    if (myRequests.size() == 0)
        out << "1";
    else
        out << myRequests.size();

    out << " requests of which " << myNumCompleted << " completed.";

    return GTI_SUCCESS;
}

//=============================
// offerMatchedSend
//=============================
bool BlockingCompletion::offerMatchedSend (bool hasRequest, MustRequestType request)
{
    if (!hasRequest)
        return false;

    return addMatchedRequest(request);
}

//=============================
// offerMatchedReceive
//=============================
bool BlockingCompletion::offerMatchedReceive (bool hasRequest, MustRequestType request)
{
    if (!hasRequest)
        return false;

    return addMatchedRequest(request);
}

//=============================
// offerMatchedCollective
//=============================
bool BlockingCompletion::offerMatchedCollective (void)
{
    //We are not interested in these
    return false;
}

//=============================
// canComplete
//=============================
bool BlockingCompletion::canComplete (void)
{
    if (myRequests.size() == 0)
        return myNumCompleted == 1;

    if (myIsForAll)
        return myNumCompleted == myRequests.size();

    return hadAnActualCompletion;
}

//=============================
// addMatchedRequest
//=============================
bool BlockingCompletion::addMatchedRequest (MustRequestType request)
{
    if (canComplete()) return false;

    //IMPORTANT: invalidate the WFG info if something might change.
    dropWfgInfo ();

    if (myRequests.size() == 0)
    {
        if (myNumCompleted == 0 && request == myRequest.request)
        {
            myNumCompleted++;
            hadAnActualCompletion = true;
            return true;
        }
    }
    else
    {
        if (request >= minReq || request <= maxReq)
        {
            for (std::vector<RequestWaitInfo>::size_type i = 0; i < myRequests.size(); i++)
            {
                if (myRequests[i].isCompleted)
                    continue;

                if (myRequests[i].request == request)
                {
                    myNumCompleted++;
                    hadAnActualCompletion = true;
                    myRequests[i].isCompleted = true;

                    if (myMatchIndex < 0)
                        myMatchIndex = i;

                    return true;
                }
            }
        }
    }

    return false;
}

//=============================
// isMixedOp
//=============================
bool BlockingCompletion::isMixedOp (void)
{
    initWfgInfo ();
    if (canComplete()) return false;

    return myWfgInfo->isMixed;
}

//=============================
// getWaitType
//=============================
ArcType BlockingCompletion::getWaitType (void)
{
    initWfgInfo ();
    if (canComplete()) return ARC_AND;

    return myWfgInfo->type;
}

//=============================
// mixedOpGetNumSubNodes
//=============================
int BlockingCompletion::mixedOpGetNumSubNodes (void)
{
    initWfgInfo ();
    if (canComplete()) return 0;

    return myWfgInfo->subNodeToReq.size();
}

//=============================
// getWaitedForRanks
//=============================
std::list<int> BlockingCompletion::getWaitedForRanks (
        std::list<std::string> *outLabels,
        std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *pReferences,
        std::map<I_Comm*, std::string> &commLabels)
{
    std::list<int> ret;

    //Init
    initWfgInfo ();
    if (canComplete()) return ret;

    //Process
    if (myRequests.size() == 0)
    {
        applyP2PToWait ("request", myRequest.info, &ret, outLabels, pReferences, commLabels);
        return ret;
    }//If just a single request

    //Array of requests
    for (std::vector<RequestWaitInfo>::size_type i = 0; i < myRequests.size(); i++)
    {
        if (myRequests[i].isCompleted)
            continue;

        //Is this a request of a sub node for a split op ?
        if (myWfgInfo->isMixed && myRequests[i].info->isWc)
            continue;

        std::stringstream stream;
        stream << "[" << i << "]";
        applyP2PToWait (stream.str(), myRequests[i].info, &ret, outLabels, pReferences, commLabels);
    }

    return ret;
}

//=============================
// getSubNodeWaitedForRanks
//=============================
std::list<int> BlockingCompletion::getSubNodeWaitedForRanks (
        int subId,
        std::string *outLabel,
        bool *outHasReference,
        MustParallelId *outPId,
        MustLocationId *outLId,
        std::map<I_Comm*, std::string> &commLabels)
{
    std::list<int> ret;

    initWfgInfo ();
    if (canComplete()) return ret;

    if (subId >= myWfgInfo->subNodeToReq.size()) return ret;

    int rIndex = myWfgInfo->subNodeToReq[subId];

    std::stringstream stream;
    stream << "[" << rIndex << "]";
    if (outLabel) *outLabel = stream.str();
    if (outHasReference) *outHasReference = true;
    if (outPId) *outPId = myRequests[rIndex].info->pId;
    if (outLId) *outLId = myRequests[rIndex].info->lId;

    applyP2PToWait ("", myRequests[rIndex].info, &ret, NULL, NULL, commLabels);

    return ret;
}

//=============================
// initRequestInfo
//=============================
bool BlockingCompletion::initRequestInfo (MustRequestType request, P2PInfo **outInfo)
{
    if (!outInfo) return false; //invalid call
    if (*outInfo) return true;  //already initialized

    *outInfo = new P2PInfo ();
    if (!myState->myP2PMatch->getP2PInfo(myRank, request, *outInfo))
    {
        std::cerr << "Internal Error: BlockingState could not find information for a P2P op in P2PMatch!" << std::endl;
        assert (0);
    }

    return true;
}

//=============================
// initWfgInfo
//=============================
bool BlockingCompletion::initWfgInfo (void)
{
    if (myWfgInfo) return true; //Already initialized
    if (canComplete()) return true; //Already completed, we should not do this at all!

    myWfgInfo = new WfgInfo ();

    //A single request (no array)
    if (myRequests.size() == 0)
    {
        initRequestInfo (myRequest.request, &(myRequest.info));

        myWfgInfo->isMixed = false;
        myWfgInfo->type = ARC_AND;

        if (myRequest.info->isWc)
            myWfgInfo->type = ARC_OR;
        return true;
    }

    //Expected type
    myWfgInfo->type = ARC_AND;
    if (!myIsForAll)
        myWfgInfo->type = ARC_OR;;
    myWfgInfo->isMixed = false;

    //Array: Initialize all infos
    for (std::vector<RequestWaitInfo>::size_type i = 0; i < myRequests.size(); i++)
    {
        if (myRequests[i].isCompleted)
            continue;

        initRequestInfo (myRequests[i].request, &(myRequests[i].info));

        //Do we need to mix ?
        //Expected type
        if (myRequests.size() > 1 && myRequests[i].info->isWc && myWfgInfo->type == ARC_AND)
        {
            myWfgInfo->isMixed = true;
            myWfgInfo->subNodeToReq.insert (std::make_pair (myWfgInfo->subNodeToReq.size(), i));
        }

        //Special case a Waitall with exactly one wc receive request -> we change the expected type in this case to OR
        if (myRequests.size() == 1 && myRequests[i].info->isWc && myWfgInfo->type == ARC_AND)
        {
            myWfgInfo->type = ARC_OR;
        }
    }

    return true;
}

//=============================
// dropWfgInfo
//=============================
bool BlockingCompletion::dropWfgInfo (void)
{
    if (myWfgInfo) delete myWfgInfo;
    myWfgInfo = NULL;
    return true;
}

//=============================
// getUsedComms
//=============================
std::list<I_Comm*> BlockingCompletion::getUsedComms (void)
{
    std::list<I_Comm*> ret;
    std::list<I_Comm*>::iterator iter;

    //Load all information on the requests
    initWfgInfo ();

    //Single request completeion
    if (myRequests.size() == 0)
    {
        I_Comm *comm = NULL;

        if (myRequest.info)
            comm = myRequest.info->comm;
        if (comm)
            ret.push_back(comm);
    }

    //Multi request completion
    for (std::vector<RequestWaitInfo>::size_type i = 0; i < myRequests.size(); i++)
    {
        I_Comm *comm = NULL;

        if (myRequests[i].info)
            comm = myRequests[i].info->comm;
        if (comm)
        {
            for (iter = ret.begin(); iter != ret.end(); iter++)
            {
                if (*iter == comm)
                    break;
            }

            if (iter == ret.end())
                ret.push_back(comm);
        }
    }

    return ret;
}

//=============================
// waitsForASend
//=============================
bool BlockingCompletion::waitsForASend (int fromRank)
{
    initWfgInfo ();

    //Single request completeion
    if (myRequests.size() == 0)
    {
        if (myRequest.info)
        {
            if (!myRequest.info->isSend && (myRequest.info->isWc || myRequest.info->target == fromRank))
                return true;
        }
    }

    //Multi request completion
    for (std::vector<RequestWaitInfo>::size_type i = 0; i < myRequests.size(); i++)
    {
        if (myRequests[i].info)
        {
            if (!myRequests[i].info->isSend && (myRequests[i].info->isWc || myRequests[i].info->target == fromRank))
                return true;
        }
    }

    return false;
}

//=============================
// waitsForAReceive
//=============================
bool BlockingCompletion::waitsForAReceive (int fromRank)
{
    initWfgInfo ();

    //Single request completeion
    if (myRequests.size() == 0)
    {
        if (myRequest.info)
        {
            if (myRequest.info->isSend && myRequest.info->target == fromRank)
                return true;
        }
    }

    //Multi request completion
    for (std::vector<RequestWaitInfo>::size_type i = 0; i < myRequests.size(); i++)
    {
        if (myRequests[i].info)
        {
            if (myRequests[i].info->isSend && myRequests[i].info->target == fromRank)
                return true;
        }
    }

    return false;
}

//=============================
// copy
//=============================
BlockingOp* BlockingCompletion::copy (void)
{
    return new BlockingCompletion (this);
}

//=============================
// BlockingCompletion (from existing)
//=============================
BlockingCompletion::BlockingCompletion (BlockingCompletion* other)
 : BlockingOp (other->myState, other->myPId, other->myLId)
{
     //Init this
    myRequest = other->myRequest.copy();

    myRequests.resize(other->myRequests.size());
    std::vector<RequestWaitInfo>::iterator iter;
    int i = 0;
    for (iter = other->myRequests.begin(); iter != other->myRequests.end(); iter++, i++)
    {
        myRequests[i] = iter->copy();
    }

    minReq = other->minReq;
    maxReq = other->maxReq;

    myIsForAll = other->myIsForAll;
    myNumCompleted = other->myNumCompleted;
    myMatchIndex = other->myMatchIndex;
    hadAnActualCompletion = other->hadAnActualCompletion;

    if (other->myWfgInfo)
    {
        myWfgInfo = new WfgInfo ();
        myWfgInfo->type = other->myWfgInfo->type;
        myWfgInfo->isMixed = other->myWfgInfo->isMixed;

        std::map<int, int>::iterator nTRIter;

        for (nTRIter = other->myWfgInfo->subNodeToReq.begin(); nTRIter != other->myWfgInfo->subNodeToReq.end(); nTRIter++)
        {
            myWfgInfo->subNodeToReq.insert(std::make_pair(nTRIter->first,nTRIter->second));
        }
    }
    else
    {
        myWfgInfo = NULL;
    }
}

/*EOF*/
