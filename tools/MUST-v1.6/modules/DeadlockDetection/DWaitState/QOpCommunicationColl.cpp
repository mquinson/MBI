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
 * @file QOpCommunicationColl.cpp
 *       @see must::QOpCommunicationColl.
 *
 *  @date 01.03.2013
 *  @author Tobias Hilbrich
 */

#include "QOpCommunicationColl.h"
#include "DWaitState.h"

using namespace must;

//=============================
// QCollectiveMatchInfo -- QCollectiveMatchInfo
//=============================
QCollectiveMatchInfo::QCollectiveMatchInfo (
        int numRanksInComm)
 : myRefCount (1),
   myNumRanksInComm (numRanksInComm),
   myNumActive (0),
   myJoinedRanks ()
{
    //Nothing to do
}

//=============================
// QCollectiveMatchInfo -- incRefCount
//=============================
int QCollectiveMatchInfo::incRefCount (void)
{
    myRefCount++;
    return myRefCount;
}

//=============================
// QCollectiveMatchInfo -- erase
//=============================
int QCollectiveMatchInfo::erase (void)
{
    myRefCount--;
    int temp = myRefCount;
    if (myRefCount == 0)
        delete (this);

    return temp;
}

//=============================
// QCollectiveMatchInfo -- addAsActive
//=============================
void QCollectiveMatchInfo::addAsActive (int rank)
{
    myJoinedRanks.push_back (rank);
    myNumActive++;
}

//=============================
// QCollectiveMatchInfo -- allActive
//=============================
bool QCollectiveMatchInfo::allActive (void)
{
    return myNumActive == myNumRanksInComm;
}

//=============================
// QCollectiveMatchInfo -- getNumActive
//=============================
int QCollectiveMatchInfo::getNumActive (void)
{
    return myNumActive;
}

//=============================
// QCollectiveMatchInfo -- getNumRanksInComm
//=============================
int QCollectiveMatchInfo::getNumRanksInComm (void)
{
    return myNumRanksInComm;
}

//=============================
// QCollectiveMatchInfo -- ~QCollectiveMatchInfo
//=============================
QCollectiveMatchInfo::~QCollectiveMatchInfo (void)
{
    //Nothing to do
}

//=============================
// QOpCommunicationColl
//=============================
QOpCommunicationColl::QOpCommunicationColl (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                I_CommPersistent *comm,
                MustCollCommType collType,
                MustLTimeStamp waveNumberInComm
        )
 : QOpCommunication (dws, pId, lId, ts, comm),
   myCollType (collType),
   myWaveNumberInComm (waveNumberInComm),
   myMatchInfo (NULL),
   myActiveAndAddedToLocalMatch (false),
   myGotActiveAcknowledge(false)
{
    //Nothing to do
}

//=============================
// QOpCommunicationColl
//=============================
QOpCommunicationColl::~QOpCommunicationColl ()
{
    if (myMatchInfo)
        myMatchInfo->erase();
}

//=============================
// printVariablesAsLabelString
//=============================
std::string QOpCommunicationColl::printVariablesAsLabelString (void)
{
    std::stringstream stream;
    stream << "|waveNumberInComm=" << myWaveNumberInComm;

    if (myMatchInfo)
    {
        stream << "|Active=" << myMatchInfo->getNumActive() << "/" << myMatchInfo->getNumRanksInComm();

        if (myGotActiveAcknowledge)
            stream << "|GotAcknowledge";
        else
            stream << "|WaitsForAcknowledge";
    }
    else
    {
        stream << "|NO-MATCH-INFO";
    }

    return QOpCommunication::printVariablesAsLabelString() + stream.str();
}

//=============================
// asOpCommunicationColl
//=============================
QOpCommunicationColl* QOpCommunicationColl::asOpCommunicationColl (void)
{
    return this;
}

//=============================
// asOpCommunicationColl
//=============================
void QOpCommunicationColl::setMatchInfo (QCollectiveMatchInfo* info)
{
    if (myMatchInfo) myMatchInfo->erase (); //Should not happen ...

    myMatchInfo = info;
    myMatchInfo->incRefCount();
}

//=============================
// notifyActiveAcknowledge
//=============================
void QOpCommunicationColl::notifyActiveAcknowledge (void)
{
    myGotActiveAcknowledge = true;
}

//=============================
// notifyActive
//=============================
void QOpCommunicationColl::notifyActive (void)
{
    //Do we still need to contribute to become active in this collective?
    if (!myActiveAndAddedToLocalMatch && myMatchInfo)
    {
        myMatchInfo->addAsActive(myRank);
        myActiveAndAddedToLocalMatch=true;

        //If we are the last rank to join, we must create the
        //CollectiveActive request
        if (myMatchInfo->allActive())
        {
            if (myState->getCollRequestFunction())
            {
                int localSize = 0, remoteSize = 0, firstRankOfW = 0;
                if (myComm->getGroup())
                {
                    localSize = myComm->getGroup()->getSize();
                    myComm->getGroup()->translate(0,&firstRankOfW);
                }
                if (myComm->getRemoteGroup())
                {
                    remoteSize = myComm->getRemoteGroup()->getSize();
                    firstRankOfW = 0; /*Invalidate that for intercomms*/
                }

                (*(myState->getCollRequestFunction())) (
                        (int)myComm->isIntercomm(),
                        myComm->getContextId()+firstRankOfW,
                        myCollType,
                        localSize,
                        remoteSize,
                        myMatchInfo->getNumRanksInComm()
                        );
            }
        }
    }
}

//=============================
// blocks
//=============================
bool QOpCommunicationColl::blocks (void)
{
    if (myGotActiveAcknowledge)
        return false;
    return true;
}

//=============================
// waitsForAcknowledge
//=============================
bool QOpCommunicationColl::waitsForAcknowledge (
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize)
{
    if (myGotActiveAcknowledge) //Say "no" if we got out part already!
        return false;

    if (!myMatchInfo)
        return false;

    if (!myMatchInfo->allActive())
        return false;

    if (isIntercomm != myComm->isIntercomm())
        return false;

    int localGroupSize2 = 0, remoteGroupSize2 = 0;
    if (myComm->getGroup()) localGroupSize2 = myComm->getGroup()->getSize();
    if (myComm->getRemoteGroup()) remoteGroupSize2 = myComm->getRemoteGroup()->getSize();
    unsigned long contextId2 = myComm->getContextId();

    if (myComm->getGroup() && !myComm->getRemoteGroup())
    {
        int firstOfW;
        myComm->getGroup()->translate(0, &firstOfW);
        contextId2 += firstOfW;
    }

    if (!isIntercomm)
    {
        //For intracomms
        if (localGroupSize == localGroupSize2 && contextId == contextId2)
            return true;
    }
    else
    {
        //For intercomms
        if (contextId != contextId2)
            return false;

        if ( (localGroupSize == localGroupSize2 && remoteGroupSize == remoteGroupSize2) ||
                (localGroupSize == remoteGroupSize2 && remoteGroupSize == localGroupSize2))
            return true;
    }

    return false;
}

//=============================
// isFinalize
//=============================
bool QOpCommunicationColl::isFinalize (void)
{
    if (myCollType == MUST_COLL_FINALIZE)
        return true;
    return false;
}

//=============================
// needsToBeInTrace
//=============================
bool QOpCommunicationColl::needsToBeInTrace (void)
{
    /*
     * We ned to be in the trace as long as we didn't got our acknowledge,
     * we can be removed from the trace afterwards.
     */
    if (!myGotActiveAcknowledge)
        return true;
    return false;
}

//=============================
// forwardWaitForInformation
//=============================
void QOpCommunicationColl::forwardWaitForInformation (
        std::map<I_Comm*, std::string> &commLabels)
{
    //Do we really still block?
    if (!blocks())
        return;

    //Provide it!
    provideWaitForInfosCollP f = myState->getProvideWaitCollFunction();

    int localSize = 0, remoteSize = 0, firstRankOfW = 0;
    if (myComm->getGroup())
    {
        localSize = myComm->getGroup()->getSize();
        myComm->getGroup()->translate(0,&firstRankOfW);
    }
    if (myComm->getRemoteGroup())
    {
        remoteSize = myComm->getRemoteGroup()->getSize();
        firstRankOfW = 0;
    }

    (*f) (
            myRank,
            myPId,
            myLId,
            (int)myCollType,
            (int)myComm->isIntercomm(),
            myComm->getContextId()+firstRankOfW,
            localSize,
            remoteSize);
}

//=============================
// forwardThisOpsWaitForInformation
//=============================
void QOpCommunicationColl::forwardThisOpsWaitForInformation (
        int subIdToUse,
        std::map<I_Comm*, std::string> &commLabels)
{
    //Do we really still lack an acknowledge?
    if (myGotActiveAcknowledge)
        return;

    //Provide it!
    provideWaitForInfosNbcCollP f = myState->getProvideWaitNbcCollFunction();

    int localSize = 0, remoteSize = 0, firstRankOfW = 0;
    if (myComm->getGroup())
    {
        localSize = myComm->getGroup()->getSize();
        myComm->getGroup()->translate(0,&firstRankOfW);
    }
    if (myComm->getRemoteGroup())
    {
        remoteSize = myComm->getRemoteGroup()->getSize();
        firstRankOfW = 0;
    }

    (*f) (
            myRank,
            myPId,
            myLId,
            subIdToUse,
            (int)myWaveNumberInComm,
            (int)myComm->isIntercomm(),
            myComm->getContextId()+firstRankOfW,
            localSize,
            remoteSize);
}

//=============================
// isMatchedWithActiveOps
//=============================
bool QOpCommunicationColl::isMatchedWithActiveOps (void)
{
    if (myGotActiveAcknowledge)
        return true;

    return false;
}

//=============================
// handleNbcBackgroundForwarding
//=============================
void QOpCommunicationColl::handleNbcBackgroundForwarding (void)
{
    //Only valid for NBC ops
    if (!hasRequest())
        return;

    //Do we really still wait for the acknowledge?
    if (myGotActiveAcknowledge)
        return;

    //Provide it!
    provideWaitForNbcBackgroundP f = myState->getProvideWaitNbcBackgroundFunction();

    int localSize = 0, remoteSize = 0, firstRankOfW = 0;
    if (myComm->getGroup())
    {
        localSize = myComm->getGroup()->getSize();
        myComm->getGroup()->translate(0,&firstRankOfW);
    }
    if (myComm->getRemoteGroup())
    {
        remoteSize = myComm->getRemoteGroup()->getSize();
        firstRankOfW = 0;
    }

    (*f) (
            myRank,
            myPId,
            myLId,
            (int)myWaveNumberInComm,
            (int)myComm->isIntercomm(),
            myComm->getContextId()+firstRankOfW,
            localSize,
            remoteSize);
}

/*EOF*/
