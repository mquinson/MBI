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
 * @file BlockingP2P.cpp
 *       @see must::BlockingP2P.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "BlockingP2P.h"
#include "BlockingState.h"

using namespace must;

//=============================
// Constructor
//=============================
BlockingP2P::BlockingP2P (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                bool isSend,
                bool isSendRecvSend
        )
 : BlockingOp (state, pId, lId),
   myIsSend (isSend),
   myIsMatched (false),
   myIsSendRecvSend (isSendRecvSend),
   mySecondary (NULL),
   myP2PInfo (NULL)
{
    //Nothing to do
}

//=============================
// Destructor
//=============================
BlockingP2P::~BlockingP2P (void)
{
    mySecondary = NULL;

    if (myP2PInfo) delete myP2PInfo;
    myP2PInfo = NULL;
}

//=============================
// process
//=============================
PROCESSING_RETURN BlockingP2P::process (int rank)
{
    myState->applyNewP2POp(this);
    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN BlockingP2P::print (std::ostream &out)
{
    out << "Blocking ";

    if (myIsSend)
        out << " send";
    else
        out << " receive";

    out << " call ";

    if (myIsMatched)
        out << "that is matched";
    else
        out << "is not matched yet";

    out << ".";

    return GTI_SUCCESS;
}

//=============================
// offerMatchedSend
//=============================
bool BlockingP2P::offerMatchedSend (bool hasRequest, MustRequestType request)
{
    if (myIsSend && !hasRequest && !myIsMatched)
    {
        myIsMatched = true;
        return true;
    }

    return false;
}

//=============================
// offerMatchedReceive
//=============================
bool BlockingP2P::offerMatchedReceive (bool hasRequest, MustRequestType request)
{
    if (!myIsSend && !hasRequest && !myIsMatched)
    {
        myIsMatched = true;
        return true;
    }

    return false;
}

//=============================
// offerMatchedCollective
//=============================
bool BlockingP2P::offerMatchedCollective (void)
{
    //Not interested in collectives
    return false;
}

//=============================
// canComplete
//=============================
bool BlockingP2P::canComplete (void)
{
    //Only if matched
    return myIsMatched;
}

//=============================
// isSend
//=============================
bool BlockingP2P::isSend (void)
{
    return myIsSend;
}

//=============================
// isSrsend
//=============================
bool BlockingP2P::isSrsend (void)
{
    return myIsSendRecvSend;
}

//=============================
// needsSecondary
//=============================
bool BlockingP2P::needsSecondary (void)
{
    return myIsSendRecvSend;
}

//=============================
// registerSecondaryP2P
//=============================
void BlockingP2P::registerSecondaryOp (BlockingOp* secondary)
{
    //P2P ops only hace other P2P ops as secondary, so the cast should be safe here
    mySecondary = (BlockingP2P*) secondary;
}

//=============================
// isMixedOp
//=============================
bool BlockingP2P::isMixedOp (void)
{
    if (myIsMatched && (!mySecondary || mySecondary->myIsMatched)) return false;
    initWfgInfo ();
    if (mySecondary) mySecondary->initWfgInfo();

    //Primary is always the send part, so only a secondary of wc type causes a mix
    if (mySecondary && mySecondary->myP2PInfo && mySecondary->myP2PInfo->isWc)
        return true;

    return false;
}

//=============================
// getWaitType
//=============================
ArcType BlockingP2P::getWaitType (void)
{
    if (myIsMatched && (!mySecondary || mySecondary->myIsMatched)) return ARC_AND;
    initWfgInfo ();
    if (mySecondary) mySecondary->initWfgInfo();

    //If a secondary we are AND, irrespective of whether it causes a mix or not
    if (mySecondary && mySecondary->myP2PInfo)
        return ARC_AND;

    //If this is a wc receive, then its OR
    if (myP2PInfo && myP2PInfo->isWc)
        return ARC_OR;

    return ARC_AND;
}

//=============================
// mixedOpGetNumSubNodes
//=============================
int BlockingP2P::mixedOpGetNumSubNodes (void)
{
    if (myIsMatched && (!mySecondary || mySecondary->myIsMatched)) return 0;
    initWfgInfo ();
    if (mySecondary) mySecondary->initWfgInfo();

    if (mySecondary && mySecondary->myP2PInfo && mySecondary->myP2PInfo->isWc)
        return 1;

    return 0;
}

//=============================
// getWaitedForRanks
//=============================
std::list<int> BlockingP2P::getWaitedForRanks (
        std::list<std::string> *outLabels,
        std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *pReferences,
        std::map<I_Comm*, std::string> &commLabels)
{
    std::list<int> ret;

    if (myIsMatched && (!mySecondary || mySecondary->myIsMatched)) return ret;
    initWfgInfo ();
    if (mySecondary) mySecondary->initWfgInfo();

    std::string labelA = "", labelB = "";
    if (needsSecondary())
    {
        labelA = "send";
        labelB = "receive";
    }

    if (myP2PInfo && !myIsMatched)
        applyP2PToWait (labelA, myP2PInfo, &ret, outLabels, NULL, commLabels);

    if (mySecondary && mySecondary->myP2PInfo && !mySecondary->myP2PInfo->isWc)
        applyP2PToWait (labelB, mySecondary->myP2PInfo, &ret, outLabels, NULL, commLabels);

    return ret;
}

//=============================
// getSubNodeWaitedForRanks
//=============================
std::list<int> BlockingP2P::getSubNodeWaitedForRanks (
        int subId,
        std::string *outLabel,
        bool *outHasReference,
        MustParallelId *outPId,
        MustLocationId *outLId,
        std::map<I_Comm*, std::string> &commLabels)
{
    std::list<int> ret;

    if (myIsMatched && (!mySecondary || mySecondary->myIsMatched)) return ret;
    initWfgInfo ();
    if (mySecondary) mySecondary->initWfgInfo();

    if (mySecondary && mySecondary->myP2PInfo && mySecondary->myP2PInfo->isWc)
    {
        if (outLabel)
        {
            std::stringstream stream;
            stream << "receive, ";

            std::map<I_Comm*, std::string>::iterator labelIter;
            for (labelIter = commLabels.begin(); labelIter != commLabels.end(); labelIter++)
            {
                if (labelIter->first->compareComms(mySecondary->myP2PInfo->comm))
                {
                    stream << "comm=" << labelIter->second<< ", ";
                    break;
                }
            }

            stream << "tag=" << mySecondary->myP2PInfo->tag;

             *outLabel = stream.str();
        }
        if (outHasReference) *outHasReference = false;
        applyP2PToWait ("", mySecondary->myP2PInfo, &ret, NULL, NULL, commLabels);
    }

    return ret;
}

//=============================
// initWfgInfo
//=============================
bool BlockingP2P::initWfgInfo (void)
{
    if (myIsMatched) return true;
    if (myP2PInfo) return true;

    myP2PInfo = new P2PInfo ();
    if (!myState->myP2PMatch->getP2PInfo(myRank, myIsSend, myP2PInfo))
    {
        std::cerr << "Internal Error: BlockingState could not find information for a P2P op in P2PMatch!" << std::endl;
        assert (0);
    }
    return true;
}

//=============================
// dropWfgInfo
//=============================
bool BlockingP2P::dropWfgInfo (void)
{
    if (myP2PInfo) delete myP2PInfo;
    myP2PInfo = NULL;
    return true;
}

//=============================
// getUsedComms
//=============================
std::list<I_Comm*> BlockingP2P::getUsedComms (void)
{
    std::list<I_Comm*> ret;

    initWfgInfo ();
    if (!myP2PInfo) return ret;

    ret.push_back(myP2PInfo->comm);

    return ret;
}

//=============================
// waitsForASend
//=============================
bool BlockingP2P::waitsForASend (int fromRank)
{
    if (myIsSend || myIsMatched)
        return false;

    initWfgInfo ();
    if (!myP2PInfo) return false;

    if (myP2PInfo->isWc || myP2PInfo->target == fromRank)
        return true;

    return false;
}

//=============================
// waitsForAReceive
//=============================
bool BlockingP2P::waitsForAReceive (int fromRank)
{
    if (!myIsSend || myIsMatched)
        return false;

    initWfgInfo ();
    if (!myP2PInfo) return false;

    if (myP2PInfo->target == fromRank)
        return true;

    return false;
}


//=============================
// copy
//=============================
BlockingOp* BlockingP2P::copy (void)
{
    return new BlockingP2P (this);
}

//=============================
// BlockingP2P (from existing)
//=============================
BlockingP2P::BlockingP2P (BlockingP2P* other)
    : BlockingOp (other->myState, other->myPId, other->myLId)
{
    //Init this
    myIsSend = other->myIsSend;
    myIsMatched = other->myIsMatched;
    myIsSendRecvSend = other->myIsSendRecvSend;

    //Pointer to a secondary has to be fixed by the caller!
    mySecondary = NULL;
    /*mySecondary = other->mySecondary();
    if (mySecondary)
        mySecondary = (BlockingP2P*)mySecondary->copy();*/

    if (other->myP2PInfo)
    {
        myP2PInfo = new P2PInfo();
        myP2PInfo->isSend = other->myP2PInfo->isSend;
        myP2PInfo->pId = other->myP2PInfo->pId;
        myP2PInfo->lId = other->myP2PInfo->lId;
        myP2PInfo->target = other->myP2PInfo->target;
        myP2PInfo->isWc = other->myP2PInfo->isWc;
        /** @todo this is not really correct but works as long as we only copy these ops when we do a full checkpoint of the Deadlock detection system*/
        myP2PInfo->comm = other->myP2PInfo->comm;
        myP2PInfo->mode = other->myP2PInfo->mode;
        myP2PInfo->tag = other->myP2PInfo->tag;
    }
    else
    {
        myP2PInfo = NULL;
    }
}

/*EOF*/
