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
 * @file QOpCommpletion.cpp
 *       @see must::QOpCommpletion.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "QOpCompletion.h"

#include "DWaitState.h"
#include <string>
#include <fstream>
#include <string.h>

using namespace must;

//=============================
// RequestInfo -- Constructor
//=============================
RequestInfo::RequestInfo (
        MustRequestType request,
        QOpCommunication* nonBlockingOp)
 : request(request),
   nonBlockingOp (nonBlockingOp),
   completed (false)
{
    //Nothing to do
    assert (!nonBlockingOp || nonBlockingOp->hasRequest());
}

//=============================
// RequestInfo -- Constructor
//=============================
RequestInfo::RequestInfo ()
 : request(0),
   nonBlockingOp (NULL),
   completed (false)
{
    //Nothing to do
}

//=============================
// RequestInfo -- Destructor
//=============================
RequestInfo::~RequestInfo (void)
{
    if (nonBlockingOp) nonBlockingOp->erase();
    nonBlockingOp = NULL;
}

//=============================
// QOpCompletion -- Constructor
//=============================
QOpCompletion::QOpCompletion (
        DWaitState *dws,
        MustParallelId pId,
        MustLocationId lId,
        MustLTimeStamp ts,
        MustRequestType request
)
: QOp (dws, pId, lId, ts),
  myRequest (),
  myRequests (),
  myWaitsForAll (true),
  myNumCompleted (0),
  myMatchIndex (-1)
{
    QOpCommunication* nonBlockingOp = dws->getNonBlockingOpForRequest(pId, request);

    myRequest.request = request;
    myRequest.nonBlockingOp = nonBlockingOp;

    //We are already done if this is an unknown, NULL, or inactive request
    if (!nonBlockingOp)
    {
        myNumCompleted = 1;
        myRequest.completed = true;
    }
}

//=============================
// QOpCompletion -- Constructor
//=============================
QOpCompletion::QOpCompletion (
        DWaitState *dws,
        MustParallelId pId,
        MustLocationId lId,
        MustLTimeStamp ts,
        int count,
        MustRequestType *requests,
        bool waitsForAll,
        bool hadProcNullReqs
)
: QOp (dws, pId, lId, ts),
  myRequest (),
  myRequests (),
  myWaitsForAll (waitsForAll),
  myNumCompleted (0),
  myMatchIndex (-1)
{
    myRequests.resize(count);
    for (int i = 0; i < count;i++)
    {
        myRequests[i].request = requests[i];
        myRequests[i].nonBlockingOp = dws->getNonBlockingOpForRequest(pId, requests[i]);
    }

    if (hadProcNullReqs && !waitsForAll)
    {
        myNumCompleted++;
    }

    //If count was 0 we will think this was a single request not an array, we prepare for that
    if (count == 0)
    {
        myNumCompleted = 1;
        myRequest.completed = true;
        myRequest.nonBlockingOp = NULL;
    }
}

//=============================
// QOpCompletion -- Destructor
//=============================
QOpCompletion::~QOpCompletion (void)
{
    /*Destructors of the request info classes should free the
     * ops we use*/
    myRequests.clear();
}

//=============================
// printAsDot
//=============================
std::string QOpCompletion::printAsDot (std::ofstream &out, std::string nodePrefix, std::string color)
{
    std::string ret = QOp::printAsDot (out, nodePrefix, color);

    if (myRequests.size() == 0)
    {
        if (myRequest.nonBlockingOp)
        {
            out << ret << "->" << "node_" << myRank << "_" << myRequest.nonBlockingOp->getTimeStamp() << "_op" << " [style=dashed, weight=0];" << std::endl;
        }
    }
    else
    {
        for (std::vector<RequestInfo>::size_type i = 0; i < myRequests.size(); i++)
        {
            if (!myRequests[i].nonBlockingOp)
                continue;
            out << ret << "->" << "node_" << myRank << "_" << myRequests[i].nonBlockingOp->getTimeStamp() << "_op" << " [style=dashed, weight=0];" << std::endl;
        }
    }

    return ret;
}

//=============================
// printVariablesAsLabelString
//=============================
std::string QOpCompletion::printVariablesAsLabelString (void)
{
    std::stringstream stream;
    stream << "|waitsForAll=" << myWaitsForAll << "|numCompleted=" << myNumCompleted << "|myMatchIndex=" << myMatchIndex;
    return QOp::printVariablesAsLabelString() + stream.str();
}

//=============================
// notifyActive
//=============================
void QOpCompletion::notifyActive (void)
{
    /*
     * We look at all of our kids here and check on their status
     * This is a bit expensive for multicompletions,
     * if non-blocking ops would notify us we could avoid
     * a good amount of looping
     */
    if (myRequests.size() == 0)
    {
        if (!myRequest.completed)
        {
            if (myRequest.nonBlockingOp && myRequest.nonBlockingOp->isMatchedWithActiveOps())
            {
                myRequest.completed = true;
                myNumCompleted++;
            }
        }
    }
    else
    {
        for (std::vector<RequestInfo>::size_type i = 0; i < myRequests.size(); i++)
        {
            if (myRequests[i].completed)
                continue;

            if (myRequests[i].nonBlockingOp && myRequests[i].nonBlockingOp->isMatchedWithActiveOps())
            {
                myRequests[i].completed = true;
                myNumCompleted++;

                if (myMatchIndex < 0)
                    myMatchIndex = i;
            }
        }
    }
}

//=============================
// blocks
//=============================
bool QOpCompletion::blocks (void)
{
    //If only one request, then we block as long as no request is completed
    if (myRequests.size() == 0)
        return myNumCompleted == 0;

    //For multiple requests we look at ANY vs ALL
    if (myWaitsForAll)
        return !(myRequests.size() == myNumCompleted);
    else
        return myNumCompleted == 0;

    //For the compiler:
    return true;
}

//=============================
// needsToBeInTrace
//=============================
bool QOpCompletion::needsToBeInTrace (void)
{
    /*
     * We need to remain as long as we block.
     * After that we are of no interest any more.
     */
    if (blocks())
        return true;
    return false;
}

//=============================
// forwardWaitForInformation
//=============================
void QOpCompletion::forwardWaitForInformation (
        std::map<I_Comm*, std::string> &commLabels)
{
    //==Do we block at all?
    if (!blocks())
        return;

    //==Get the function
    provideWaitForInfosMixedP fMixed = myState->getProvideWaitMultiFunction();

    //==How many sub nodes?
    int numSubs = 0;
    std::stringstream labelstream;
    if (myRequests.size() == 0)
    {
        if (myRequest.completed || !myRequest.nonBlockingOp || myRequest.nonBlockingOp->isMatchedWithActiveOps())
            return;

        numSubs = 1;
        labelstream << "request" << std::endl;
    }
    else
    {
        /**
         * @label  [REDUCTION]
         * We apply some WFG reductions here to avoid the use of too many irrelevant sub-nodes:
         * - For P2POps that don't use a wildcard we can skip a request if we already had a similar
         *    sub-node (i.e. one with the same target rank); Irrespective of it being an all/any wait
         * - For wildcard P2POps (irrespective of it being an all/any wait) we only need one sub-node
         *    of the wildcard type (This only applies if the wc-recvs use the same communicator)
         */
        std::set<int> usedTargetRanks;
        bool hadWCNode = false;
        I_Comm *wcComm = NULL;

        for (std::vector<RequestInfo>::size_type i = 0; i < myRequests.size(); i++)
        {
            //Is this a relevant request (unmatched, valid, ...)
            if (myRequests[i].completed || !myRequests[i].nonBlockingOp || myRequests[i].nonBlockingOp->isMatchedWithActiveOps())
                continue;

            if (myRequests[i].nonBlockingOp->asOpCommunicationP2P())
            {
                ////HANDLING FOR P2P Ops with reduction mentioned above
                QOpCommunicationP2P *p2pOp = myRequests[i].nonBlockingOp->asOpCommunicationP2P ();
                assert (p2pOp);

                //Can we apply a WFG reduction for this request (does it adds relevant or redundant WFG information)
                bool isWc = false;
                int sourceTarget = p2pOp->getSourceTarget(&isWc);

                if (isWc)
                {
                    //We skip a wc sub-node if its come is equal to the comm of the last wc-node we had
                    if (hadWCNode)
                    {
                        if (wcComm->compareComms(p2pOp->getComm()))
                            continue;
                    }
                    hadWCNode = true;
                    wcComm = p2pOp->getComm();
                }
                else
                {
                    if (usedTargetRanks.find(sourceTarget) != usedTargetRanks.end())
                        continue;
                    usedTargetRanks.insert(sourceTarget);
                }

                //Its valid and important, so it counts!
                numSubs++;
                labelstream << "request[" << i << "]" << std::endl;
            }
            else if (myRequests[i].nonBlockingOp->asOpCommunicationColl())
            {
                //HANDLING FOR COLLS No reductions included currently
                numSubs++;
                labelstream << "request[" << i << "]" << std::endl;
            }
        }
    }

    //==Prepare
    MustParallelId *labelPIds = new MustParallelId[numSubs];
    MustLocationId *labelLIds = new MustLocationId[numSubs];

    for (int i = 0; i < numSubs; i++)
        labelPIds[i] = labelLIds[0] = 0;

    size_t length = strlen(labelstream.str().c_str())+1;
    char *labelsConcat = new char[length];
    strcpy (labelsConcat, labelstream.str().c_str());
    ArcType arcT = ARC_AND;
    if (!myWaitsForAll)
        arcT = ARC_OR;

    //==Create the sub nodes
    (*fMixed) (
            myRank,
            myPId,
            myLId,
            numSubs,
            (int) arcT,
            labelPIds,
            labelLIds,
            length,
            labelsConcat
            );

    //==Let the P2P ops print themselves
    if (myRequests.size() == 0)
    {
        myRequest.nonBlockingOp->forwardThisOpsWaitForInformation(0, commLabels);
    }
    else
    {
        /**
         * See [REDUCTION] comment above
         */
        std::set<int> usedTargetRanks;
        bool hadWCNode = false;
        I_Comm *wcComm = NULL;

        int subId = 0;
        for (std::vector<RequestInfo>::size_type i = 0; i < myRequests.size(); i++)
        {
            //Is this a relevant request (unmatched, valid, ...)
            if (myRequests[i].completed || !myRequests[i].nonBlockingOp || myRequests[i].nonBlockingOp->isMatchedWithActiveOps())
                continue;

            if (myRequests[i].nonBlockingOp->asOpCommunicationP2P())
            {
                ////HANDLING FOR P2P Ops with reduction mentioned above
                QOpCommunicationP2P *p2pOp = myRequests[i].nonBlockingOp->asOpCommunicationP2P ();
                assert (p2pOp);

                //Can we apply a WFG reduction for this request (does it adds relevant or redundant WFG information)
                bool isWc = false;
                int sourceTarget = p2pOp->getSourceTarget(&isWc);

                if (isWc)
                {
                    //We skip a wc sub-node if its come is equal to the comm of the last wc-node we had
                    if (hadWCNode)
                    {
                        if (wcComm->compareComms(p2pOp->getComm()))
                            continue;
                    }
                    hadWCNode = true;
                    wcComm = p2pOp->getComm();
                }
                else
                {
                    if (usedTargetRanks.find(sourceTarget) != usedTargetRanks.end())
                        continue;
                    usedTargetRanks.insert(sourceTarget);
                }

                //Its valid and important, so it counts!
                p2pOp->forwardThisOpsWaitForInformation(subId, commLabels);
                subId++;
            }
            else if (myRequests[i].nonBlockingOp->asOpCommunicationColl())
            {
                //HANDLING FOR COLLS No reductions included currently
                myRequests[i].nonBlockingOp->forwardThisOpsWaitForInformation(subId, commLabels);
                subId++;
            }
        }
    }

    //==Clean up
    if (labelPIds)
        delete[] labelPIds;
    if (labelLIds)
        delete[] labelLIds;
    if (labelsConcat)
        delete[] labelsConcat;
}

//=============================
// getUsedComms
//=============================
std::list<I_Comm*> QOpCompletion::getUsedComms (void)
{
    std::list<I_Comm*> ret;
    std::list<I_Comm*>::iterator iter;

    if (myRequests.size() == 0)
    {
        //Just add the single comm (if we have one)
        if (myRequest.nonBlockingOp)
        {
            ret.push_back(myRequest.nonBlockingOp->getComm());
        }
    }
    else
    {
        //Loop over all nb ops and check that we do not add duplicate comms
        for (std::vector<RequestInfo>::size_type i = 0; i < myRequests.size(); i++)
        {
            if (!myRequests[i].nonBlockingOp)
                continue;

            I_Comm *c = myRequests[i].nonBlockingOp->getComm();

            for (iter = ret.begin(); iter != ret.end(); iter++)
            {
                if ((*iter)->compareComms(c))
                    break;
            }

            if (iter == ret.end())
                ret.push_back (c);
        }
    }

    return ret;
}

//=============================
// getPingPongNodes
//=============================
std::set<int> QOpCompletion::getPingPongNodes (void)
{
    std::set<int> ret;

    if (myRequests.size() == 0)
    {
        if (myRequest.nonBlockingOp)
        {
            return myRequest.nonBlockingOp->getPingPongNodes();
        }
    }
    else
    {
        for (std::vector<RequestInfo>::size_type i = 0; i < myRequests.size(); i++)
        {
            if (!myRequests[i].nonBlockingOp)
                continue;

            std::set<int> temp = myRequests[i].nonBlockingOp->getPingPongNodes();
            std::set<int>::iterator iter;
            for (iter = temp.begin(); iter != temp.end(); iter++)
                ret.insert(*iter);
        }
    }

    return ret;
}


/*EOF*/
