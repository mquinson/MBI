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
 * @file BlockingColl.cpp
 *       @see must::BlockingColl.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "BlockingColl.h"
#include "BlockingState.h"

using namespace must;

//=============================
// Constructor
//=============================
BlockingColl::BlockingColl (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm)
 : BlockingOp (state, pId, lId),
   myCollId (collId),
   myIsCompleted (false),
   myComm (comm)
{
    /*Nothing to do*/
}

//=============================
// Destructor
//=============================
BlockingColl::~BlockingColl (void)
{
    if (myComm) myComm->erase();
    myComm = NULL;
}

//=============================
// process
//=============================
PROCESSING_RETURN BlockingColl::process (int rank)
{
    myState->applyNewCollectiveOp(this);
    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN BlockingColl::print (std::ostream &out)
{
    out << "Collective id=" << myCollId << ".";

    return GTI_SUCCESS;
}

//=============================
// offerMatchedSend
//=============================
bool BlockingColl::offerMatchedSend (bool hasRequest, MustRequestType request)
{
    //This does not care about P2P
    return false;
}

//=============================
// offerMatchedReceive
//=============================
bool BlockingColl::offerMatchedReceive (bool hasRequest, MustRequestType request)
{
    //This does not care about P2P
    return false;
}

//=============================
// offerMatchedCollective
//=============================
bool BlockingColl::offerMatchedCollective (void)
{
    if (!myIsCompleted)
    {
        myIsCompleted = true;
        return true;
    }

    return false;
}

//=============================
// canComplete
//=============================
bool BlockingColl::canComplete (void)
{
    return myIsCompleted;
}

//=============================
// isMixedOp
//=============================
bool BlockingColl::isMixedOp (void)
{
    /*Collectives are always just AND type*/
    return false;
}

//=============================
// getWaitType
//=============================
ArcType BlockingColl::getWaitType (void)
{
    /*Collectives are always just AND type*/
    return ARC_AND;
}

//=============================
// mixedOpGetNumSubNodes
//=============================
int BlockingColl::mixedOpGetNumSubNodes (void)
{
    /*Collectives are always just AND type*/
    return 0;
}

//=============================
// getWaitedForRanks
//=============================
std::list<int> BlockingColl::getWaitedForRanks (
        std::list<std::string> *outLabels,
        std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *pReferences,
        std::map<I_Comm*, std::string> &commLabels)
{
    std::list<int> ret;

    I_GroupTable *group = myComm->getGroup();
    int commSize = group->getSize();

    for (int i = 0; i < commSize; i++)
    {
        int wRank;
        /**@todo Think about intercomms here (MPI2)*/
        group->translate(i, &wRank);

        HeadInfo* head = &(myState->myHeads[wRank]);

        //Is it a primary with matching comm ?
        if (head->primary)
        {
            if (head->primary->isMatchingColl (myCollId, myComm))
                continue;
        }

        //If there is no primary or a not matching one we are waiting for this rank
        ret.push_back(wRank);
        if (outLabels) outLabels->push_back(""); //Colls need no labels
        if (pReferences) pReferences->push_back (std::make_pair (false, std::make_pair (0,0)));
    }

    return ret;
}

//=============================
// getSubNodeWaitedForRanks
//=============================
std::list<int> BlockingColl::getSubNodeWaitedForRanks (
        int subId,
        std::string *outLabel,
        bool *outHasReference,
        MustParallelId *outPId,
        MustLocationId *outLId,
        std::map<I_Comm*, std::string> &commLabels)
{
    /*Collectives are always just AND type*/
    if (outHasReference) *outHasReference = false;
    return std::list<int> ();
}

//=============================
// isMatchingColl
//=============================
bool BlockingColl::isMatchingColl (MustCollCommType collId, I_Comm *comm)
{
    if (myComm->compareComms(comm) && collId == myCollId)
        return true;
    return false;
}

//=============================
// getUsedComms
//=============================
std::list<I_Comm*> BlockingColl::getUsedComms (void)
{
    std::list<I_Comm*> ret;

    ret.push_back(myComm);

    return ret;
}

//=============================
// copy
//=============================
BlockingOp* BlockingColl::copy (void)
{
    return new BlockingColl (this);
}

//=============================
// Constructor (from existing op)
//=============================
BlockingColl::BlockingColl (BlockingColl *other)
    : BlockingOp (other->myState, other->myPId, other->myLId)
{
    //Init this
    myCollId = other->myCollId;
    myIsCompleted = other->myIsCompleted;
    myComm = other->myComm;
    if (myComm) myComm->copy();
}

//=============================
// isCollective
//=============================
bool BlockingColl::isCollective (void)
{
    return true;
}

/*EOF*/
