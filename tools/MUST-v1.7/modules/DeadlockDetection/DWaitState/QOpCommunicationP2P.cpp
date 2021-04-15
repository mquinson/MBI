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
 * @file QOpCommunicationP2P.cpp
 *       @see must::QOpCommunicationP2P.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "QOpCommunicationP2P.h"

#include "DWaitState.h"

#include <string.h>

using namespace must;

//=============================
// QOpCommunicationP2P
//=============================
QOpCommunicationP2P::QOpCommunicationP2P (
        DWaitState *dws,
        MustParallelId pId,
        MustLocationId lId,
        MustLTimeStamp ts,
        I_CommPersistent *comm,
        bool isSend,
        int sourceTarget,
        bool isWc,
        MustSendMode mode,
        int tag
)
: QOpCommunication (dws, pId, lId, ts, comm),
  myIsSend (isSend),
  mySourceTarget (sourceTarget),
  myTag (tag),
  myGotRecvBecameActive (false),
  myMode (mode),
  mySentRecvBecameActiveAcknowledge(false),
  myTSOfMatchingReceive (0),
  myIsSendrecvSend (false),
  myGotRecvMatchUpdate (false),
  myTSOfMatchingSend (0),
  myIsWc (isWc),
  mySentBecameActiveRequest (false),
  myGotActiveAcknowledge (false),
  myAssociatedSend (NULL)
{
    //Nothing to do
}

//=============================
// ~QOpCommunicationP2P
//=============================
QOpCommunicationP2P::~QOpCommunicationP2P (void)
{
    if (myAssociatedSend)
        myAssociatedSend->erase();
}

//=============================
// printVariablesAsLabelString
//=============================
std::string QOpCommunicationP2P::printVariablesAsLabelString (void)
{
    std::stringstream stream;
    stream
        << "|sourceTarget=" << mySourceTarget
        << "|isWC=" << myIsWc
        << "|mode=" << myMode
        << "|tag=" << myTag;

    if (myIsSend)
        stream
            << "|gotRecvBecameActive=" << myGotRecvBecameActive
            << "|tsOfReceive=" << myTSOfMatchingReceive
            << "|sentActiveAck=" << mySentRecvBecameActiveAcknowledge;
    else
        stream
            << "|tsOfMatchingSend=" << myTSOfMatchingSend
            << "|sentActiveRequest=" << mySentBecameActiveRequest
            << "|gotActiveAck=" << myGotActiveAcknowledge;

    return QOpCommunication::printVariablesAsLabelString() + stream.str();
}

//=============================
// asOpCommunicationP2P
//=============================
QOpCommunicationP2P* QOpCommunicationP2P::asOpCommunicationP2P (void)
{
    return this;
}

//=============================
// setMatchingInformation
//=============================
void QOpCommunicationP2P::setMatchingInformation (
        MustParallelId pIdSend,
        MustLTimeStamp sendTS)
{
    myGotRecvMatchUpdate = true;

    myTSOfMatchingSend = sendTS;
    if (myIsWc)
    {
        mySourceTarget = myState->getParallelIdAnalysis()->getInfoForId(pIdSend).rank;
    }

    /*
     * If we are matched with a remote send, then we already know that this send is
     * active, since we only pass sends once they are active.
     */
    bool isThisNode;
    myState->getNodeForWorldRank (mySourceTarget, &isThisNode);
    if (!isThisNode)
    {
        myGotActiveAcknowledge = true;
    }
}

//=============================
// isNonBlockingP2P
//=============================
bool QOpCommunicationP2P::isNonBlockingP2P (void)
{
    return false;
}

//=============================
// notifyActive
//=============================
void QOpCommunicationP2P::notifyActive (void)
{
    if (myIsSend)
    {
        //SEND
        //Do we have to send a ReceiveActiveAcknowledge?
        if (myGotRecvBecameActive && !mySentRecvBecameActiveAcknowledge)
        {
            //TODO
            bool isThisNode;
            myState->getNodeForWorldRank(mySourceTarget, &isThisNode);
            mySentRecvBecameActiveAcknowledge = true;

            if (!isThisNode)
            {
                //If receive is at remote node -> Nothing to do we only passed the send acros once it was active, so the receiver already knows we are active
                /*
                 * August 1, 2013: modified this such that we only use two messages for DWS (rather than 3)
                 */
            }
            else
            {
                //If receive is at local node -> active the across events handler
                /*
                 * Warning this causes recursion in DWaitState::advanceOp!
                 */
                myState->receiveActiveAcknowledge(
                        mySourceTarget,
                        myTSOfMatchingReceive);
            }
        }
    }
    else
    {
        //RECEIVE
        //Do we have to send a ReceiveActiveRequest
        if (myGotRecvMatchUpdate && !mySentBecameActiveRequest)
        {
            bool isThisNode;
            int toNode = myState->getNodeForWorldRank (mySourceTarget, &isThisNode);
            mySentBecameActiveRequest = true;

            if (!isThisNode)
            {
                //If receive is at remote node -> generate across event
                generateReceiveActiveRequestP f = myState->getReceiveActiveRequestFunction();
                if (f)
                {
                    (*f) (
                            mySourceTarget,
                            myTSOfMatchingSend,
                            myTS,
                            toNode
                    );
                }
            }
            else
            {
                //If receive is at local node -> active the across events handler
                /*
                 * Warning this causes recursion in DWaitState::advanceOp!
                 */
                myState->receiveActiveRequest(
                        mySourceTarget,
                        myTSOfMatchingSend,
                        myTS);
            }
        }
    }
}

//=============================
// blocks
//=============================
bool QOpCommunicationP2P::blocks (void)
{
    //Non blocking sends and buffered mode sends never block
    if (isNonBlockingP2P() ||
            (myIsSend && myIsSendrecvSend) ||
            (myIsSend && myMode == MUST_BUFFERED_SEND))
        return false;

    //Special case for sendrecv receives: check whether the associated send is matched
    if (myAssociatedSend)
    {
        if (!myAssociatedSend->isMatchedWithActiveOps())
            return true;
    }

    //Are we matched?
    if (myIsSend)
    {
        if (myGotRecvBecameActive)
            return false;
    }
    else
    {
        if (myGotActiveAcknowledge)
            return false;
    }

    //Default is we block
    return true;
}

//=============================
// notifyGotReceiveActiveRequest
//=============================
void QOpCommunicationP2P::notifyGotReceiveActiveRequest (MustLTimeStamp receiveLTS)
{
    assert (myIsSend);

    myGotRecvBecameActive = true;
    myTSOfMatchingReceive = receiveLTS;
}

//=============================
// notifyGotReceiveActiveRequest
//=============================
void QOpCommunicationP2P::notifyGotReceiveActiveAcknowledge (void)
{
    assert (!myIsSend);

    myGotActiveAcknowledge = true;
}

//=============================
// isMatchedWithActiveOps
//=============================
bool QOpCommunicationP2P::isMatchedWithActiveOps (void)
{
    if (myIsSend)
    {
        if (myGotRecvBecameActive)
            return true;
    }
    else
    {
        if (myGotActiveAcknowledge)
            return true;
    }

    return false;
}

//=============================
// isMatchedWithActiveOps
//=============================
void QOpCommunicationP2P::setAsSendrecvSend (void)
{
    myIsSendrecvSend = true;
}

//=============================
// setAsSendrecvRecv
//=============================
void QOpCommunicationP2P::setAsSendrecvRecv (QOpCommunicationP2P* send)
{
    myAssociatedSend = send;
}

//=============================
// needsToBeInTrace
//=============================
bool QOpCommunicationP2P::needsToBeInTrace (void)
{
    /*
     * Once we performed all of our tasks:
     * Send: get the receive became active and sent the acknowledge
     * Receive: got the acknowledge
     */
    if (myIsSend)
    {
        if (myGotRecvBecameActive && mySentRecvBecameActiveAcknowledge)
            return false;
    }
    else
    {
        if (myGotActiveAcknowledge && mySentBecameActiveRequest)
            return false;
    }

    return true;
}

//=============================
// forwardWaitForInformation
//=============================
void QOpCommunicationP2P::forwardWaitForInformation (
        std::map<I_Comm*, std::string> &commLabels)
{
    //If we do not block, we have nothing to do here
    if (!blocks())
        return;

    //Get function pointers for mixed case
    provideWaitForInfosMixedP fMixed = myState->getProvideWaitMultiFunction();

    if (myIsWc && !isMatchedWithActiveOps() && myAssociatedSend && !myAssociatedSend->isMatchedWithActiveOps())
    {
        //Special case where this is a wildcard receive part of a sendreceive where both operations are not matched with an
        //active operation yet
        MustParallelId tempPIds[] = {0,0};
        MustParallelId tempLIds[] = {0,0};
        char labels[] = "send\nreceive\n";

        //This node with two sub-nodes
        (*fMixed) (
                myRank,
                myPId,
                myLId,
                2,
                (int) ARC_AND,
                tempPIds,
                tempLIds,
                strlen (labels)+1,
                labels
                );

        //Send part
        myAssociatedSend->forwardThisOpsWaitForInformation (0, commLabels);

        //Receive part
        forwardThisOpsWaitForInformation (1, commLabels);
    }
    else
    {
        //Regular case, just forward this ops information
        forwardThisOpsWaitForInformation (-1, commLabels);
    }
}

//=============================
// forwardThisOpsWaitForInformation
//=============================
void QOpCommunicationP2P::forwardThisOpsWaitForInformation (
                int subIdToUse,
                std::map<I_Comm*, std::string> &commLabels)
{
    //==Get function
    provideWaitForInfosSingleP fSingle = myState->getProvideWaitSingleFunction();

    //==Prepare comm label
    std::string commLabel = "";

    std::map<I_Comm*, std::string>::iterator iter;
    for (iter = commLabels.begin(); iter != commLabels.end(); iter++)
    {
        if (myComm->compareComms(iter->first))
        {
            commLabel = iter->second;
            break;
        }
    }

    //==Prepare targets, arc-type, labels
    int count = 1;
    ArcType arcT = ARC_AND;
    if (!myIsSend && myIsWc)
    {
        if (!myComm->isIntercomm())
            count = myComm->getGroup()->getSize();
        else
            count = myComm->getRemoteGroup()->getSize();

        arcT = ARC_OR;
    }

    int *targets = new int[count];
    MustParallelId *labelPIds = new MustParallelId[count];
    MustLocationId *labelLIds = new MustLocationId[count];
    targets[0] = mySourceTarget;
    std::stringstream labels;

    if (!myIsSend && myIsWc)
    {
        if (!myComm->isIntercomm())
        {
            for (int i = 0; i < count; i++)
                myComm->getGroup()->translate(i,&(targets[i]));
        }
        else
        {
            for (int i = 0; i < count; i++)
                myComm->getRemoteGroup()->translate(i,&(targets[i]));
        }
    }

    for (int i = 0; i < count; i++)
    {
        labelPIds[i] = labelLIds[i] = 0;

        if (myTag == myState->isMpiAnyTag(myTag))
            labels << "comm=" << commLabel << ", tag=MPI_ANY_TAG" << std::endl;
        else
            labels << "comm=" << commLabel << ", tag=" << myTag << std::endl;
    }

    size_t length = strlen(labels.str().c_str())+1;
    char *labelsConcat = new char[length];
    strcpy (labelsConcat, labels.str().c_str());

    //==Push everything out
    (*fSingle) (
            myRank,
            myPId,
            myLId,
            subIdToUse,
            count,
            (int) arcT,
            targets,
            labelPIds,
            labelLIds,
            length,
            labelsConcat
    );

    //==Clean up
    if (targets)
        delete[] targets;
    if (labelPIds)
        delete[] labelPIds;
    if (labelLIds)
        delete[] labelLIds;
    if (labelsConcat)
        delete[] labelsConcat;
}

//=============================
// getPingPongNodes
//=============================
std::set<int> QOpCommunicationP2P::getPingPongNodes (void)
{
    std::set<int> ret;

    if (myIsSend)
    {
        bool isThisNode;
        int targetNodeId = myState->getNodeForWorldRank(this->mySourceTarget, &isThisNode);

        if (!isThisNode)
            ret.insert(targetNodeId);
    }

    return ret;
}

//=============================
// isSend
//=============================
bool QOpCommunicationP2P::isSend (void)
{
    return myIsSend;
}

//=============================
// getSourceTarget
//=============================
int QOpCommunicationP2P::getSourceTarget (bool *pOutIsWc)
{
    if (pOutIsWc)
    {
        if (myIsWc)
            *pOutIsWc = true;
        else
            *pOutIsWc = false;
    }

    return mySourceTarget;
}

/*EOF*/
