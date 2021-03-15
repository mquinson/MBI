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
 * @file DCollectiveWave.cpp
 *       @see must::DCollectiveWave.
 *
 *  @date 26.04.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#include "DCollectiveWave.h"

#include <assert.h>

using namespace must;

//=============================
// Constructor
//=============================
DCollectiveWave::DCollectiveWave (
        MustCollCommType collId,
        int numReachableRanks,
        int waveNumber)
: myNumReachableRanks (numReachableRanks),
  myNumJoinedSendRanks (0),
  myNumJoinedReceiveRanks (0),
  mySendIsRooted (false),
  myReceiveIsRooted (false),
  myNumExpectedIntraTypeMatchInfos (0),
  myNumJoinedIntraTypeMatchInfos (0),
  myRoot (-1),
  myRootReachable (false),
  myCollId (collId),
  myRootOp (NULL),
  mySendCompletion (NULL),
  myReceiveCompletion (NULL),
  mySendTransfers (),
  myReceiveTransfers (),
  mySendChannels (),
  myReceiveChannels (),
  myTimedOut (false),
  myWaveNumber (waveNumber),
  myRemoteTypeMatchInfos ()
{
    switch (myCollId)
    {
    case MUST_COLL_GATHERV:
        myNumExpectedIntraTypeMatchInfos = 1;
    case MUST_COLL_GATHER:
    case MUST_COLL_REDUCE:
        myNumExpectedSendRanks = myNumReachableRanks;
        myNumExpectedReceiveRanks = 1;
        myReceiveIsRooted = true;
        break;
    case MUST_COLL_SCATTERV:
        myNumExpectedIntraTypeMatchInfos = 1;
    case MUST_COLL_BCAST:
    case MUST_COLL_SCATTER:
        myNumExpectedSendRanks = 1;
        myNumExpectedReceiveRanks = myNumReachableRanks;
        mySendIsRooted = true;
        break;
    case MUST_COLL_ALLTOALLV:
    case MUST_COLL_ALLTOALLW:
        myNumExpectedIntraTypeMatchInfos = myNumReachableRanks; /*place holder, real value set in addNewOp (commSize - numReachableRanks)*/
    case MUST_COLL_ALLGATHER:
    case MUST_COLL_ALLGATHERV:
    case MUST_COLL_ALLTOALL:
    case MUST_COLL_ALLREDUCE:
    case MUST_COLL_REDUCE_SCATTER:
    case MUST_COLL_REDUCE_SCATTER_BLOCK:
    case MUST_COLL_SCAN:
    case MUST_COLL_EXSCAN:
        myNumExpectedSendRanks = myNumReachableRanks;
        myNumExpectedReceiveRanks = myNumReachableRanks;
        break;
    case MUST_COLL_BARRIER:
    case MUST_COLL_CART_CREATE:
    case MUST_COLL_CART_SUB:
    case MUST_COLL_COMM_CREATE:
    case MUST_COLL_COMM_DUP:
    case MUST_COLL_COMM_FREE:
    case MUST_COLL_COMM_SPLIT:
    case MUST_COLL_FINALIZE:
    case MUST_COLL_GRAPH_CREATE:
    case MUST_COLL_INTERCOMM_CREATE:
    case MUST_COLL_INTERCOMM_MERGE:
        myNumExpectedSendRanks = myNumReachableRanks;
        myNumExpectedReceiveRanks = 0;
        break;
    default:
        assert (0); //INVALID collId
    }
}

//=============================
// Destructor
//=============================
DCollectiveWave::~DCollectiveWave ()
{
    if (mySendCompletion)
        delete (mySendCompletion);
    mySendCompletion = NULL;

    if (myReceiveCompletion)
        delete (myReceiveCompletion);
    myReceiveCompletion = NULL;

    std::list<DCollectiveOp*>::iterator iter;

    for (iter = mySendTransfers.begin (); iter != mySendTransfers.end(); iter++)
    {
        if (*iter)
            delete (*iter);
    }
    mySendTransfers.clear ();

    for (iter = myReceiveTransfers.begin (); iter != myReceiveTransfers.end(); iter++)
    {
        if (*iter)
            delete (*iter);
    }
    myReceiveTransfers.clear ();

    std::list<DCollectiveTypeMatchInfo*>::iterator rIter;
    for (rIter = myRemoteTypeMatchInfos.begin(); rIter != myRemoteTypeMatchInfos.end(); rIter++)
    {
        if (*rIter)
            delete (*rIter);
    }
    myRemoteTypeMatchInfos.clear();
}

//=============================
// initCompletions
//=============================
void DCollectiveWave::initCompletions (I_ChannelId *cId)
{
    if (!mySendCompletion && cId)
        mySendCompletion = new CompletionTree (cId->getNumUsedSubIds()-1, cId->getSubIdNumChannels(cId->getNumUsedSubIds()-1));
    if (!myReceiveCompletion && cId)
        myReceiveCompletion = new CompletionTree (cId->getNumUsedSubIds()-1, cId->getSubIdNumChannels(cId->getNumUsedSubIds()-1));
}

//=============================
// belongsToWave
//=============================
bool DCollectiveWave::belongsToWave (I_ChannelId *cId, DCollectiveOp *op)
{
    initCompletions (cId);

    /*
     * Question is am I waiting for these operations ranks (not whether this operation
     * fits the wave, we correctness check that later on)
     */
    if (myNumExpectedSendRanks > myNumJoinedSendRanks)
    {
        if (
            /*Either we need a send op from all; or this must be the root op*/
            (!mySendIsRooted || (myRoot >= 0 && myRoot == op->getIssuerRank())) &&
            /*Must not already be present in the send wave*/
            ((!cId && myNumJoinedSendRanks == 0)  || (mySendCompletion && !mySendCompletion->wasCompleted(cId)))
            )
            return true;
    }

    if (myNumExpectedReceiveRanks > myNumJoinedReceiveRanks)
    {
        if (    /*Special for BCast, we are lacking a receive event from the root here*/
                (myCollId != MUST_COLL_BCAST || myRoot != op->getIssuerRank()) &&
                /*Either we need a receive op from all; or the this is the root op*/
                (!myReceiveIsRooted || (myRoot >= 0 && myRoot == op->getIssuerRank())) &&
                /*Must not already be present in the receive wave*/
                ((!cId && myNumJoinedReceiveRanks == 0)  || (myReceiveCompletion && !myReceiveCompletion->wasCompleted(cId)))
            )
            return true;
    }

    return false;
}

//=============================
// isCompleted
//=============================
bool DCollectiveWave::isCompleted (void)
{
    return (    myNumJoinedSendRanks == myNumExpectedSendRanks &&
                    myNumJoinedReceiveRanks == myNumExpectedReceiveRanks);
}

//=============================
// addNewOp
//=============================
GTI_ANALYSIS_RETURN DCollectiveWave::addNewOp (
        I_DCollectiveListener *listener,
        I_ChannelId *cId,
        std::list<I_ChannelId*> *outFinishedChannels,
        DCollectiveOp *op,
        bool runIntraChecks,
        bool ancestorRunsIntraChecks,
        int commStride,
        int commOffset)
{
    initCompletions (cId);

    //==0) First op?
    if (myNumJoinedSendRanks == 0 && myNumJoinedReceiveRanks == 0)
    {
        myCollId = op->getCollId();

        //Update intra comm expectations
        if (!runIntraChecks || ancestorRunsIntraChecks)
        {
            myNumExpectedIntraTypeMatchInfos = 0;
        }
        else
        {
            if (    op->getCollId() == MUST_COLL_ALLTOALLV ||
                    op->getCollId() == MUST_COLL_ALLTOALLW)
                myNumExpectedIntraTypeMatchInfos = op->getCommSize() - myNumExpectedIntraTypeMatchInfos /*numReachableRanks*/;
        }
    }

    //==1) Do we have a root yet?
    if (op->hasRoot() && myRoot == -1)
    {
        myRoot = op->getRoot();

        /*
         * If the root of this collective is not reachable on this TBON node we must
         * adapt our expectations for the number of ops to get!
         */
        int groupRoot;
        if (!op->getComm()->getGroup()->containsWorldRank(myRoot, &groupRoot))
            assert (0);

        if (!op->getComm()->isRankReachable(groupRoot))
        {
            if (op->isSendTransfer())
                myNumExpectedReceiveRanks--;
            else
                myNumExpectedSendRanks--;

            myRootReachable = false;
        }
        else
        {
            myRootReachable = true;

            //MPI_Bcast (root provides no receive)
            if (op->getCollId() == MUST_COLL_BCAST)
                myNumExpectedReceiveRanks--;

            //Root needs not wait for any intra communication (SCATTERV, GATHERV)
            if (op->getCollId() == MUST_COLL_SCATTERV || op->getCollId() == MUST_COLL_GATHERV)
                myNumExpectedIntraTypeMatchInfos = 0;
        }
    }

    //==2alpha) Do we actually have to check anything for this op?
    bool needToCheckThisOp = true;
    CompletionTree *treeToUse = NULL;

    if (op->isSendTransfer() || op->isNoTransfer())
    {
        treeToUse = mySendCompletion;
    }
    else
    {
        treeToUse = myReceiveCompletion;
    }

    if (cId && treeToUse)
    {
        bool newChild = treeToUse->createsNewChild(cId);

        if (!newChild && (!op->needsIntraCommToCheck() || ancestorRunsIntraChecks))
            needToCheckThisOp = false;
    }

    DCollectiveOp *transfer = op;

    if (needToCheckThisOp)
    {
        //==2) Is the collective-id right?
        if (myCollId != op->getCollId ())
        {
            //Get first participant to retrieve mismatch partner
            DCollectiveOp *other;
            if (!mySendTransfers.empty())
                other = mySendTransfers.front();
            else
                other = myReceiveTransfers.front();

            other->printCollectiveMismatch (op);

            //We should also open all our outFinishedChannels
            abort (outFinishedChannels);

            return GTI_ANALYSIS_FAILURE;
        }

        //==2b) Are both collectives either blocking or non-blocking
        //      (MPI_X collectives must not be mixed with MPI_Ix collectives)
        if (myNumJoinedSendRanks > 0 || myNumJoinedReceiveRanks > 0) //We need at least one other operation
        {
            DCollectiveOp *other;
            if (!mySendTransfers.empty())
                other = mySendTransfers.front();
            else
                other = myReceiveTransfers.front();
            assert (other);

            if (other->hasRequest() != op->hasRequest ())
            {
                other->printBlockingNonBlockingMismatch (op);

                //We should also open all our outFinishedChannels
                abort (outFinishedChannels);
                return GTI_ANALYSIS_FAILURE;
            }
        }

        //==3) Validate the new op
        //A) Op must match
        if (op->hasOp() &&
            (myNumJoinedSendRanks > 0 || myNumJoinedReceiveRanks > 0))
        {
            DCollectiveOp *other;
            if (!mySendTransfers.empty())
                other = mySendTransfers.front();
            else
                other =myReceiveTransfers.front();
            assert (other);

            I_Op *a = op->getOp(),
                    *b = other->getOp();

            if (*a  != *b)
            {
                op->printOpMismatch(other);

                //We should also open all our outFinishedChannels
                abort (outFinishedChannels);

                return GTI_ANALYSIS_FAILURE;
            }
        }

        //B) Root must match
        /** @todo What about intercomms here? .*/
        if (op->hasRoot())
        {
            if (myRoot != op->getRoot())
            {
                DCollectiveOp *other;
                if (!mySendTransfers.empty())
                    other = mySendTransfers.front();
                else
                    other =myReceiveTransfers.front();
                assert (other);

                op->printRootMismatch(other);

                //We should also open all our outFinishedChannels
                abort (outFinishedChannels);

                return GTI_ANALYSIS_FAILURE;
            }
        }

        //C) Types must match
        /** @todo What about intercomms here, the matching rules are probably different for them? */
        DCollectiveOp *send = NULL, *receive = NULL;

        if (!transfer->isNoTransfer() &&
            (!transfer->needsIntraCommToCheck() || !ancestorRunsIntraChecks)) //for ops that are already intra communication checked we don't need to do anything
        {
            if(transfer->isSendTransfer())
                send = transfer;
            else
                receive = transfer;

            if (transfer->isToOne())
            {
                //to-1 transfer, check whether we got the root op
                if (myRootReachable)
                {
                    //If we can receive the root op we check against that one
                    if (myRootOp)
                    {
                        if (transfer->isSendTransfer())
                            receive = myRootOp;
                        else
                            send = myRootOp;

                        assert (send && receive && send->isSendTransfer() && receive->isReceiveTransfer());
                        send->validateTypeMatch(receive); //We do not switch of for type mismatches, if the app doen't crashs we should be able to continue
                    }
                }
                else
                {
                    //If we can't receive the root op, we check against another
                    //transfer of the same type (if not something we have to
                    //take care of with intra layer communication)
                    if (!transfer->needsIntraCommToCheck() &&
                        (myNumJoinedSendRanks + myNumJoinedReceiveRanks > 0))
                    {
                        if (transfer->isSendTransfer())
                            receive = mySendTransfers.front();
                        else
                            send = myReceiveTransfers.front();

                        //IMPORTANT: this is a send-send or a recv-recv comparison in fact!
                        assert (send && receive);
                        send->validateTypeMatch(receive); //We do not switch of for type mismatches, if the app doen't crashs we should be able to continue
                    }
                }
            }
            else
            {
                //to-N transfer
                if (!transfer->hasRoot() && !transfer->collectiveUsesMultipleCounts())
                {
                    /*
                     * A: If all transfers are to-N transfers with a single type, we can use type equality transitivity
                     * I.e. we only compare to one type that was already compared instead of to all types
                     */
                    if (transfer->isSendTransfer() && !myReceiveTransfers.empty())
                        receive = myReceiveTransfers.front();
                    else
                    if (transfer->isReceiveTransfer() && !mySendTransfers.empty())
                        send = mySendTransfers.front();

                    if (send && receive)
                    {
                        assert (send && receive && send->isSendTransfer() && receive->isReceiveTransfer());
                        send->validateTypeMatch(receive); //We do not switch of for type mismatches, if the app doen't crashs we should be able to continue
                    }
                }
                else
                {
                    /*
                     * B: If we are the send/receive-N needed for a rooted operation we have to compare to all
                     *    OR if we use multiple counts
                     */
                    std::list<DCollectiveOp*> *listToUse;
                    std::list<DCollectiveOp*>::iterator iter;

                    if (transfer->isSendTransfer())
                        listToUse = &myReceiveTransfers;
                    else
                        listToUse = &mySendTransfers;

                    for (iter = listToUse->begin(); iter != listToUse->end(); iter++)
                    {
                        if (transfer->isSendTransfer())
                            receive = *iter;
                        else
                            send = *iter;

                        assert (send && receive && send->isSendTransfer() && receive->isReceiveTransfer());
                        send->validateTypeMatch(receive); //We do not switch off for type mismatches, if the app doesn't crashs we should be able to continue
                    }

                    /*
                     * C: If we use multiple counts and are transitive (i.e. don't need intra communication)
                     *     than all signatures resulting from counts+types in this transfer are equal to any
                     *     other transfers with their respective types+counts.
                     * Examples:
                     *    MPI_Allgatherv
                     *    MPI_Reduce_scatter
                     */
                    if (!transfer->hasRoot() && !transfer->needsIntraCommToCheck() && transfer->hasMultipleCounts())
                    {
                        DCollectiveOp *partner = NULL;
                        if (transfer->isSendTransfer() && !mySendTransfers.empty())
                            partner = mySendTransfers.front();

                        if (transfer->isReceiveTransfer() && !myReceiveTransfers.empty())
                            partner = myReceiveTransfers.front();

                        if (partner)
                        {
                            if (transfer->getCollId() == MUST_COLL_REDUCE_SCATTER)
                                transfer->validateJustCountsArrayEquality(partner);
                            else if (transfer->getCollId() == MUST_COLL_ALLGATHERV)
                                transfer->validateCountsArrayEquality (partner);
                            else assert (0); //Check mapping of this check!
                        }
                    }
                }
            }//Is-to-one?
        }//Is the new op an actual transfer?
    }//Do we need to check anything for this op?

    //4) Add the op
    GTI_ANALYSIS_RETURN ret;

    //Is it the root op?
    if (!myRootOp && transfer->hasRoot() && !transfer->isToOne() && transfer->getRoot() == transfer->getIssuerRank())
    {
        myRootOp = transfer;
    }

    //Add to our list of transfers, apply to completions
    if (transfer->isSendTransfer() || transfer->isNoTransfer())
    {
        if (needToCheckThisOp)
            mySendTransfers.push_back(transfer);

        if (cId)
            mySendCompletion->addCompletion(cId);
        myNumJoinedSendRanks += transfer->getNumRanks();

        assert (myNumJoinedSendRanks <= myNumExpectedSendRanks);

        /*
         * We can create a reduced record if:
         * - we are not timed out
         * - we have a list of outFinishedChannels
         * - the op either doesn't needs intra comm to check OR an ancestor/we are do the intra comm
         */
        if (!myTimedOut && outFinishedChannels &&
            (!transfer->needsIntraCommToCheck() || (ancestorRunsIntraChecks || runIntraChecks)) &&
            !(myNumJoinedSendRanks == 1 && myNumExpectedSendRanks == 1 && op->getCollId() != MUST_COLL_BCAST) /*Last condition avoids unnecessary cId based coarsening for single op aggregations, exception is the BCAST send part (its a single op)!*/
            )
        {
            if (myNumJoinedSendRanks == myNumExpectedSendRanks)
            {
                //This is complete
                ret = GTI_ANALYSIS_SUCCESS;

                outFinishedChannels->splice(outFinishedChannels->end(), mySendChannels);
                createReducedRecord(true, commStride, commOffset);
            }
            else
            {
                ret = GTI_ANALYSIS_WAITING;

                if (cId)
                    mySendChannels.push_back(cId);
            }
        }
        else
        {
            ret = GTI_ANALYSIS_IRREDUCIBLE;
        }
    }
    else
    {
        if (needToCheckThisOp)
            myReceiveTransfers.push_back(transfer);

        if (cId)
            myReceiveCompletion->addCompletion(cId);
        myNumJoinedReceiveRanks += transfer->getNumRanks();

        assert (myNumJoinedReceiveRanks <= myNumExpectedReceiveRanks);

        /*
         * We can create a reduced record if:
         * - we are not timed out
         * - we have a list of outFinishedChannels
         * - the op either doesn't needs intra comm to check OR an ancestor or we are do the intra comm
         */
        if (!myTimedOut && outFinishedChannels &&
            (!transfer->needsIntraCommToCheck() || (ancestorRunsIntraChecks || runIntraChecks)) &&
            !(myNumJoinedReceiveRanks == 1 && myNumExpectedReceiveRanks == 1) /*Last condition avoids unnecessary cId based coarsening for single op aggregations!*/
            )
        {
            if (myNumJoinedReceiveRanks == myNumExpectedReceiveRanks)
            {
                //This is complete
                ret = GTI_ANALYSIS_SUCCESS;

                outFinishedChannels->splice(outFinishedChannels->end(), myReceiveChannels);
                createReducedRecord(false, commStride, commOffset);
            }
            else
            {
                ret = GTI_ANALYSIS_WAITING;

                if (cId)
                    myReceiveChannels.push_back(cId);
            }
        }
        else
        {
            ret = GTI_ANALYSIS_IRREDUCIBLE;
        }
    }

    //5alpha) notify the listener if the wave is complete (both send and receive part)
    if (    listener &&
            myNumJoinedReceiveRanks == myNumExpectedReceiveRanks &&
            myNumJoinedSendRanks == myNumExpectedSendRanks)
    {
        std::list<std::pair<MustParallelId, MustLTimeStamp> > opInfos;
        std::list<DCollectiveOp*> *list;
        std::list<DCollectiveOp*>::iterator opIter;

        for (int i=0;i<2;i++)
        {
            if (i == 0)
                list = &mySendTransfers;
            else
                list = &myReceiveTransfers;

            for (opIter = list->begin(); opIter != list->end(); opIter++)
            {
                DCollectiveOp* curOp = *opIter;
                if (curOp->getLTimeStamp() != 0)
                    opInfos.push_back (std::make_pair(curOp->getPId(), curOp->getLTimeStamp()));
            }
        }

        listener->notifyCollectiveLocalComplete(opInfos);
    }

    //5) Do we need to start intra-layer communication?
    if (runIntraChecks)
    {
        if (op->needsIntraCommToCheck())
            op->intraCommunicateTypeMatchInfos (myWaveNumber);
    }

    //== Did we got all and everything? If so run intra layer type matching
    if (    myNumJoinedIntraTypeMatchInfos == myNumExpectedIntraTypeMatchInfos &&
            isCompleted())
        intraLayerTypeMatching ();

    //== If we did not add this op to our op lists, we must now free it
    if (!needToCheckThisOp)
        delete op;

    return ret;
}

//=============================
// createReducedRecord
//=============================
void DCollectiveWave::createReducedRecord (bool forSend, int commStride, int commOffset)
{
    if (forSend)
    {
        assert (!mySendTransfers.empty());
        DCollectiveOp* op = mySendTransfers.front ();
        op->createReducedRecord (myNumJoinedSendRanks, commStride, commOffset);
    }
    else
    {
        assert (!myReceiveTransfers.empty());
        DCollectiveOp* op = myReceiveTransfers.front ();
        op->createReducedRecord (myNumJoinedReceiveRanks, commStride, commOffset);
    }
}

//=============================
// printAsDot
//=============================
std::ostream& DCollectiveWave::printAsDot (std::ostream& out, std::string nodeNamePrefix, I_LocationAnalysis *locations)
{
    int i = 0;

    DCollectiveOp *firstOp = NULL;
    if (!mySendTransfers.empty())
        firstOp = mySendTransfers.front();
    if (!myReceiveTransfers.empty())
        firstOp = myReceiveTransfers.front ();

    out
        << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
        << "{" << std::endl
        << "color=black;" << std::endl
        << "style=rounded;" << std::endl
        << "label=\"";

    if (!firstOp)
        out << "Empty Wave";
    else
        out << locations->getInfoForId(firstOp->getPId(), firstOp->getLId()).callName;

    out << "\";" << std::endl;

    out
        << nodeNamePrefix << "_" << ++i << "_Node [label=\"{root=" << myRoot << "|myRootReachable=" << myRootReachable << "|myNumReachableRanks=" << myNumReachableRanks << "| myNumJoinedSendRanks=" << myNumJoinedSendRanks << "/" << myNumExpectedSendRanks << "| myNumJoinedReceiveRanks=" << myNumJoinedReceiveRanks << "/" << myNumExpectedReceiveRanks<< "}\", shape=record];";

    if (mySendCompletion && !mySendTransfers.empty())
    {
        out
            << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
            << "{" << std::endl
            << "color=black;" << std::endl
            << "style=rounded;" << std::endl
            << "label=\"Send-Wave\";" << std::endl;

        std::stringstream stream;
        stream << nodeNamePrefix << "_" << ++i;
        mySendCompletion->printAsDot(out, stream.str());

        out << "}";
    }

    if (myReceiveCompletion && !myReceiveTransfers.empty())
    {
        out
        << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
        << "{" << std::endl
        << "color=black;" << std::endl
        << "style=rounded;" << std::endl
        << "label=\"Receive-Wave\";" << std::endl;

        std::stringstream stream;
        stream << nodeNamePrefix << "_" << ++i;
        myReceiveCompletion->printAsDot(out, stream.str());

        out << "}";
    }

    out << "}" << std::endl;

    return out;
}

//=============================
// timeout
//=============================
void DCollectiveWave::timeout (void)
{
    //Store that we timed out, drop all channel lists
    myTimedOut = true;
    mySendChannels.clear();
    myReceiveChannels.clear();
}

//=============================
// abort
//=============================
void DCollectiveWave::abort (std::list<I_ChannelId*> *outFinishedChannels)
{
    if (outFinishedChannels)
    {
        outFinishedChannels->splice (outFinishedChannels->begin(), mySendChannels);
        outFinishedChannels->splice (outFinishedChannels->begin(), myReceiveChannels);
    }

    timeout ();
}

//=============================
// myNumExpectedIntraTypeMatchInfos
//=============================
bool DCollectiveWave::waitsForIntraTypeMatchInfos (void)
{
    if (myNumJoinedIntraTypeMatchInfos < myNumExpectedIntraTypeMatchInfos)
        return true;
    return false;
}

//=============================
// getWaveNumber
//=============================
int DCollectiveWave::getWaveNumber (void)
{
    return myWaveNumber;
}

//=============================
// addNewTypeMatchInfo
//=============================
void DCollectiveWave::addNewTypeMatchInfo (std::list<DCollectiveTypeMatchInfo*> &matches)
{
    //== Account for it
    myNumJoinedIntraTypeMatchInfos+=matches.size();
    myRemoteTypeMatchInfos.splice(myRemoteTypeMatchInfos.end(), matches);

    //== Did we got all and everything? If so run intra layer type matching
    if (    myNumJoinedIntraTypeMatchInfos == myNumExpectedIntraTypeMatchInfos &&
            isCompleted())
        intraLayerTypeMatching ();
}

//=============================
// addNewTypeMatchInfo
//=============================
void DCollectiveWave::addNewTypeMatchInfo (DCollectiveTypeMatchInfo *matchInfo)
{
    //== Account for it
    myNumJoinedIntraTypeMatchInfos++;
    myRemoteTypeMatchInfos.push_back (matchInfo);

    //== Did we got all and everything? If so run intra layer type matching
    if (    myNumJoinedIntraTypeMatchInfos == myNumExpectedIntraTypeMatchInfos &&
            isCompleted())
        intraLayerTypeMatching ();
}

//=============================
// intraLayerTypeMatching
//=============================
void DCollectiveWave::intraLayerTypeMatching (void)
{
    if (myCollId == MUST_COLL_GATHERV || myCollId == MUST_COLL_SCATTERV)
    {
        //Gatherv, Scatterv
        //=1) Basic setup
        if (myRemoteTypeMatchInfos.empty()) return;
        DCollectiveTypeMatchInfo *remoteInfo = myRemoteTypeMatchInfos.front();
        if (remoteInfo->getCollId() != myCollId)
            return;

        //=3) Loop over incoming counts and compare
        std::list<DCollectiveOp*> *listToUse;
        std::list<DCollectiveOp*>::iterator iter;
        if (myCollId == MUST_COLL_GATHERV)
        {
            listToUse = &mySendTransfers;
        }
        else
        {
            listToUse = &myReceiveTransfers;
        }

        for (iter = listToUse->begin(); iter != listToUse->end(); iter++)
        {
            DCollectiveOp* op = *iter;

            if (op->getCollId() != remoteInfo->getCollId()) return;

            if (op)
                op->validateTypeMatch(remoteInfo->getPId(), remoteInfo->getLId(), remoteInfo->getType(), remoteInfo->getCounts()[op->getIssuerRank()- remoteInfo->getFirstRank()]);
            else
                assert (0); //Operation should be there, otherwise we would not run the intra-layer type matching
        }
    }
    else
    {
        //Alltoallv, Alltoallw
        std::list<DCollectiveTypeMatchInfo*>::iterator infoIter;
        std::list<DCollectiveOp*>::iterator receiveIter;

        for (infoIter = myRemoteTypeMatchInfos.begin(); infoIter != myRemoteTypeMatchInfos.end(); infoIter++)
        {
            DCollectiveTypeMatchInfo *cur = *infoIter;

            //If there is a collective mismatch, just stop, we handle that separately
            if (cur->getCollId() != myCollId)
                return;

            for (receiveIter = myReceiveTransfers.begin(); receiveIter != myReceiveTransfers.end(); receiveIter++)
            {
                DCollectiveOp* op = *receiveIter;

                if (op->getCollId() != cur->getCollId()) return;

                I_Datatype* typeToUse = cur->getType();
                if (cur->hasTypes())
                    typeToUse = cur->getTypes()[op->getIssuerRank()-cur->getFirstRank()];

                op->validateTypeMatch(cur->getPId(), cur->getLId(), typeToUse, cur->getCounts()[op->getIssuerRank()-cur->getFirstRank()]);
            }
        }
    }//else Gatherv/Scatterv VS Alltoallv/Alltoallw
}

/*EOF*/
