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
 * @file DWaitState.cpp
 *       @see DWaitState.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "GtiApi.h"
#include "MustEnums.h"

#include "DWaitState.h"
#include "BaseApi.h"

#include <sstream>
#include <fstream>
#include <stdlib.h>

using namespace must;

mGET_INSTANCE_FUNCTION(DWaitState)
mFREE_INSTANCE_FUNCTION(DWaitState)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DWaitState)

#define MUST_DWS_TRACE_SIZE_BREAK_THRESHOLD 1000000 //1MIL Operations
#define MUST_DWS_TRACE_SIZE_RESUME_THRESHOLD 100000 //100K Operations

//=============================
// Constructor -- DHeadInfo
//=============================
DHeadInfo::DHeadInfo (void)
: trace (),
  activeTS (1),
  nextTS (1),
  uncompletedNBOps (),
  wasDecremented(false)
{
    //Nothing to do
}

//=============================
// Destructor -- DHeadInfo
//=============================
DHeadInfo::~DHeadInfo (void)
{
    //Clean the trace
    std::map<MustLTimeStamp, QOp*>::iterator traceIter;
    for (traceIter = trace.begin(); traceIter != trace.end(); traceIter++)
    {
        if (traceIter->second) traceIter->second->erase();
    }
    trace.clear();

    //Clean lists of uncompleted non-blocking ops
    std::map<MustRequestType, std::list<QOpCommunication*> >::iterator nbIter;
    for (nbIter = uncompletedNBOps.begin(); nbIter != uncompletedNBOps.end(); nbIter++)
    {
        std::list<QOpCommunication*>::iterator iter;
        for (iter = nbIter->second.begin(); iter != nbIter->second.end(); iter++)
        {
            if (*iter)
                (*iter)->erase();
        }
    }
    uncompletedNBOps.clear();
}

//=============================
// DHeadInfo -- getAndIncNextTS
//=============================
MustLTimeStamp DHeadInfo::getAndIncNextTS (void)
{
    MustLTimeStamp temp = nextTS;
    nextTS++;
    return temp;
}

//=============================
// Constructor
//=============================
DWaitState::DWaitState (const char* instanceName)
    : gti::ModuleBase<DWaitState, I_DWaitState> (instanceName),
      myHeads (),
      myFirstWorldRank (-1),
      myNodeId (-1),
      myTraceSize (0),
      myMaxTraceSize (0),
      myStopTime (false),
      myNumOutstandingPingPongs (0),
      myGotEarlyCStateRequest (false),
      myVotedForBreak (false),
      myReadEnvs (false),
      myThresholdBreak (MUST_DWS_TRACE_SIZE_BREAK_THRESHOLD),
      myThresholdResume (MUST_DWS_TRACE_SIZE_RESUME_THRESHOLD)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBS 11
    if (subModInstances.size() < NUM_SUBS)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_SUBS)
    {
    		for (std::vector<I_Module*>::size_type i = NUM_SUBS; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myArgMod = (I_ArgumentAnalysis*) subModInstances[2];
    myLocations = (I_LocationAnalysis*) subModInstances[3];
    myConstants = (I_BaseConstants*) subModInstances[4];
    myDP2P = (I_DP2PMatch*) subModInstances[5];
    myDCollMatch = (I_DCollectiveMatchReduction*) subModInstances[6];
    myCommTrack = (I_CommTrack*) subModInstances[7];
    myRequestTrack = (I_RequestTrack*) subModInstances[8];
    myFloodControl = (I_FloodControl*) subModInstances[9];
    myProfiler = (I_Profiler*) subModInstances[10];

    //Initialize module data
    myDP2P->registerListener(this);
    myDCollMatch->registerListener(this);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("generateCollectiveActiveRequest", (GTI_Fct_t*)&myFCollRequest);
    assert (myFCollRequest); //otherwise this is an inconsistent tool configuration which will not work

    ModuleBase<DWaitState, I_DWaitState>::getWrapAcrossFunction ("generateReceiveActiveRequest", (GTI_Fct_t*)&myFReceiveActiveRequest);
    assert (myFReceiveActiveRequest);

    ModuleBase<DWaitState, I_DWaitState>::getWrapAcrossFunction ("generateReceiveActiveAcknowledge", (GTI_Fct_t*)&myFReceiveActiveAcknowledge);
    assert (myFReceiveActiveAcknowledge);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("provideWaitForInfosEmpty", (GTI_Fct_t*)&myFProvideWaitEmpty);
    assert (myFProvideWaitEmpty);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("provideWaitForInfosSingle", (GTI_Fct_t*)&myFProvideWaitSingle);
    assert (myFProvideWaitSingle);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("provideWaitForInfosMixed", (GTI_Fct_t*)&myFProvideWaitMulti);
    assert (myFProvideWaitMulti);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("provideWaitForInfosColl", (GTI_Fct_t*)&myFProvideWaitColl);
    assert (myFProvideWaitColl);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("provideWaitForInfosNbcColl", (GTI_Fct_t*)&myFProvideWaitNbcColl);
    assert (myFProvideWaitNbcColl);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("provideWaitForNbcBackground", (GTI_Fct_t*)&myFProvideWaitNbcBackground);
    assert (myFProvideWaitNbcBackground);

    ModuleBase<DWaitState, I_DWaitState>::getWrapAcrossFunction ("pingDWaitState", (GTI_Fct_t*)&myFPing);
    assert (myFPing);

    ModuleBase<DWaitState, I_DWaitState>::getWrapAcrossFunction ("pongDWaitState", (GTI_Fct_t*)&myFPong);
    assert (myFPong);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("acknowledgeConsistentState", (GTI_Fct_t*)&myFPAcknowledgeConsistent);
    assert (myFPAcknowledgeConsistent);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("gtiBreakRequest", (GTI_Fct_t*)&myFPBreakRequest);
    assert (myFPBreakRequest);

    ModuleBase<DWaitState, I_DWaitState>::getWrapperFunction("gtiBreakConsume", (GTI_Fct_t*)&myFPBreakConsume);
    assert (myFPBreakConsume);
}

//=============================
// Destructor
//=============================
DWaitState::~DWaitState ()
{
    /*Free other data*/
#ifdef MUST_DWS_DEBUG
        printHeadsAsDot ("debug");
#endif

	if (myPIdMod)
		destroySubModuleInstance ((I_Module*) myPIdMod);
	myPIdMod = NULL;

	if (myLogger)
		destroySubModuleInstance ((I_Module*) myLogger);
	myLogger = NULL;

	if (myArgMod)
		destroySubModuleInstance ((I_Module*) myArgMod);
	myArgMod = NULL;

	if (myLocations)
	    destroySubModuleInstance ((I_Module*) myLocations);
	myLocations = NULL;

	if (myConstants)
	    destroySubModuleInstance ((I_Module*) myConstants);
	myConstants = NULL;

	if (myDP2P)
	    destroySubModuleInstance ((I_Module*) myDP2P);
	myDP2P = NULL;

	if (myDCollMatch)
	    destroySubModuleInstance ((I_Module*) myDCollMatch);
	myDCollMatch = NULL;

	if (myCommTrack)
	    destroySubModuleInstance ((I_Module*) myCommTrack);
	myCommTrack = NULL;

	if (myRequestTrack)
	    destroySubModuleInstance ((I_Module*) myRequestTrack);
	myRequestTrack = NULL;

	if (myFloodControl)
	    destroySubModuleInstance ((I_Module*) myFloodControl);
	myFloodControl = NULL;

	if (myProfiler)
	{
	    myProfiler->reportWrapperAnalysisTime ("DWaitState", "maxTraceSize", 0, myMaxTraceSize);
	    myProfiler->reportWrapperAnalysisTime ("DWaitState", "finalTraceSize", 0, myTraceSize);
	    destroySubModuleInstance ((I_Module*) myProfiler);
	}
	myProfiler = NULL;

	myHeads.clear();
}

//=============================
// getParallelIdAnalysis
//=============================
I_ParallelIdAnalysis* DWaitState::getParallelIdAnalysis ()
{
    return myPIdMod;
}

//=============================
// getLocationlIdAnalysis
//=============================
I_LocationAnalysis* DWaitState::getLocationlIdAnalysis (void)
{
    return myLocations;
}

//=============================
// getNonBlockingOpForRequest
//=============================
QOpCommunication* DWaitState::getNonBlockingOpForRequest(
        MustParallelId pId,
        MustRequestType request)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return NULL;

    std::map<MustRequestType, std::list<QOpCommunication*> >::iterator pos;
    pos = head->uncompletedNBOps.find(request);

    /*We can do heavy asserting here since we have a nice preconditioner that removes any garbage before it can arrive here!*/
    assert (pos != head->uncompletedNBOps.end()); //Info for the request must be there
    assert (!pos->second.empty()); //There must be at least one op (must it be exactly 1?)

    QOpCommunication *op = *(pos->second.rbegin());
    assert (op);
    op->incRefCount();

    return op;
}

//=============================
// initHeads (MustParallelId)
//=============================
void DWaitState::initHeads (MustParallelId pId)
{
    if (myHeads.size()>0) return;

    //Query Module base for the ranks that send to us
    int rank = myPIdMod->getInfoForId(pId).rank;
    initHeads (rank);
}

//=============================
// initHeads (int)
//=============================
void DWaitState::initHeads (int rank)
{
    if (myHeads.size()>0) return;

    //Query Module base for the ranks that send to us
    int begin, end;

    ModuleBase<DWaitState, I_DWaitState>::getReachableRanks (&begin, &end, rank);

    //Resize heads, store which ranks the heads actually represent
    myFirstWorldRank = begin;
    myHeads.resize(end-begin+1);

    //Get own node id if necessary
    getLevelIdForApplicationRank (myFirstWorldRank, &myNodeId);
}

//=============================
// getRankAndHead (MustParallelId, ...)
//=============================
bool DWaitState::getRankAndHead (
        MustParallelId pId,
        int *outRank,
        DHeadInfo **outPHead)
{
    int rank = myPIdMod->getInfoForId(pId).rank;
    return getRankAndHead (rank, outRank, outPHead);
}

//=============================
// getRankAndHead (int, ...)
//=============================
bool DWaitState::getRankAndHead (
        int rank,
        int *outRank,
        DHeadInfo **outPHead)
{
    //Initialize heads (if necessary)
    initHeads (rank);

    DHeadInfo* head;
    if (rank < myFirstWorldRank || rank >= myFirstWorldRank + myHeads.size())
        return false;

    head = &(myHeads[rank-myFirstWorldRank]);

    if (outRank) *outRank = rank;
    if (outPHead) *outPHead = head;

    return true;
}

//=============================
// printHeadsAsDot
//=============================
void DWaitState::printHeadsAsDot (std::string prefix)
{
    std::stringstream stream;
    stream << prefix << "_" << myFirstWorldRank << ".dot";

    std::ofstream out (stream.str().c_str());

    out
        << "digraph heads" << std::endl
        << "{" << std::endl;

    for (std::vector<DHeadInfo>::size_type i = 0; i < myHeads.size(); i++)
    {
        out
            << "subgraph cluster" << i+myFirstWorldRank << std::endl
            << "{" << std::endl
            << "  color=black;" << std::endl
            << "  style=rounded;" << std::endl
            << "  label=\"" << i+myFirstWorldRank << "\";"  << std::endl;

        std::map<MustLTimeStamp, QOp*>::iterator tIter;
        std::string lastNodeName, nodeName;
        for (tIter = myHeads[i].trace.begin(); tIter != myHeads[i].trace.end(); tIter++, lastNodeName = nodeName)
        {
            if (!tIter->second) continue;

            std::string color = "white";

            if (tIter->first == myHeads[i].activeTS)
                color = "yellow";

            if (tIter->first < myHeads[i].activeTS)
                color = "green";

            std::stringstream nodePrefixStream;
            nodePrefixStream << "node_" << i+myFirstWorldRank << "_" << tIter->first;

            nodeName = tIter->second->printAsDot (out, nodePrefixStream.str(), color);

            if (lastNodeName != "")
                out << lastNodeName << "->" << nodeName << "[label=\"" << tIter->first << "\"];" << std::endl;
        }

        //Print uncompletedNBOps
        std::map<MustRequestType, std::list<QOpCommunication*> >::iterator nbIter;

        for (nbIter = myHeads[i].uncompletedNBOps.begin(); nbIter != myHeads[i].uncompletedNBOps.end(); nbIter++)
        {
            out << "uncompletedNB_" << i+myFirstWorldRank << "_" <<  nbIter->first << "[label=\"request=" << nbIter->first << "\"];" << std::endl;

            std::list<QOpCommunication*>::iterator nbs;
            for (nbs = nbIter->second.begin(); nbs != nbIter->second.end(); nbs++)
            {
                QOpCommunication* nb = *nbs;
                out << "uncompletedNB_" << i+myFirstWorldRank << "_" <<  nbIter->first << "->" << "node_" << i+myFirstWorldRank << "_" << nb->getTimeStamp()  << "_op[weight=0, style=dashed];" << std::endl;
            }
        }

        out << "}" << std::endl;
    }

    out << "}" << std::endl;

    out.close();
}

//=============================
// newP2POp
//=============================
MustLTimeStamp DWaitState::newP2POp (
        MustParallelId pId,
        MustLocationId lId,
        I_CommPersistent *comm,
        bool isSend,
        int sourceTarget,
        bool isWc,
        MustSendMode mode,
        int tag,
        bool hasRequest,
        MustRequestType request,
        bool *outIsActive
)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return 0;

    MustLTimeStamp ret = head->getAndIncNextTS();

    //Create Op
    QOpCommunicationP2P* newOp = NULL;

    if (!hasRequest)
    {
        newOp = new QOpCommunicationP2P (
                this,
                pId,
                lId,
                ret,
                comm,
                isSend,
                sourceTarget,
                isWc,
                mode,
                tag
                );
    }
    else
    {
        QOpCommunicationP2PNonBlocking* nbOp = new QOpCommunicationP2PNonBlocking (
                this,
                pId,
                lId,
                ret,
                comm,
                isSend,
                sourceTarget,
                isWc,
                mode,
                tag,
                request
        );

        newOp = nbOp;

        //Add to uncompleted nb ops
        nbOp->incRefCount();
        head->uncompletedNBOps[request].push_back(nbOp);
    }

    //Specials to handle MPI_Sendrecv correctly
    if (myLocations->getInfoForId(pId,lId).callName == "MPI_Sendrecv")
    {
        if (!isSend)
        {
            /*
             * If the receive part is an MPI_PROC_NULL that we drop, the send is just
             * a regular send. (Thats why we do not mark the send as non-blocking immediately)
             *
             * If we encounter the receive part, we must check whether the send is also still
             * around, if so we must unblock it and add it as an associated operation of this
             * receive.
             */
            std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(ret-1);
            if (pos != head->trace.end() && pos->second)
            {
                QOpCommunicationP2P* sendAsP2P = pos->second->asOpCommunicationP2P();

                if (sendAsP2P && myLocations->getInfoForId(sendAsP2P->getPId(), sendAsP2P->getLId()).callName == "MPI_Sendrecv")
                {
                    //We set the sendrecv send as "notblocking"
                    sendAsP2P->setAsSendrecvSend ();
                    sendAsP2P->incRefCount(); //For the association with the receive

                    //Associate with receive
                    newOp->setAsSendrecvRecv(sendAsP2P);

                    //Advance the send part!
                    advanceOp (sendAsP2P, head);
                }
            }
        }
    }

    //Add to Trace
    assert (head->trace.find(ret) == head->trace.end());
    head->trace[ret] = newOp;
    myTraceSize++;
    if (myTraceSize > myMaxTraceSize)
        myMaxTraceSize = myTraceSize;
    uint64_t tempTraceSize = myTraceSize;

    //Test for application break and trace size
    checkForBreakConsumeRequest (myTraceSize);

    //Advance
    advanceOp (newOp, head);

    //If the op is still in the queue, and no other op went, than mark this as a bad event
    if (tempTraceSize == myTraceSize)
        myFloodControl->markCurrentRecordBad();

    //Return whether this op is active
    /**
     * In theory we should adapt the return if head->wasDecremented is true, in that case we just added an op to the trace as head operation
     * But this should only remove wait-for dependencies not add them ...
     */
    if (head->activeTS >= ret && outIsActive)
        *outIsActive = true;
    else if (outIsActive)
        *outIsActive = false;

    return ret;
}

//=============================
// notifyP2PRecvMatch
//=============================
void DWaitState::notifyP2PRecvMatch (
        MustParallelId pIdRecv,
        MustLTimeStamp recvTS,
        MustParallelId pIdSend,
        MustLTimeStamp sendTS
)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pIdRecv, &rank, &head))
        return;

    std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find (recvTS);
    assert (pos != head->trace.end() && pos->second); //Must be there, must not be NULL!

    QOpCommunicationP2P* rop = pos->second->asOpCommunicationP2P ();
    assert (rop);

    rop->setMatchingInformation (pIdSend, sendTS);

    //Advance
    advanceOp (rop, head);
}

//=============================
// newCollectiveOp
//=============================
MustLTimeStamp DWaitState::newCollectiveOp (
        MustParallelId pId,
        MustLocationId lId,
        I_CommPersistent *comm,
        MustCollCommType collType,
        MustLTimeStamp waveNumberInComm,
        bool hasRequest,
        MustRequestType request
)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    QOpCommunicationColl* newOp = NULL;

    if (!getRankAndHead (pId, &rank, &head))
        return 0;

    MustLTimeStamp ret = head->getAndIncNextTS();

    //Create Op
    if (!hasRequest)
    {
        newOp = new QOpCommunicationColl (
                this,
                pId,
                lId,
                ret,
                comm,
                collType,
                waveNumberInComm);
    }
    else
    {
        QOpCommunicationCollNonBlocking* nbOp = new QOpCommunicationCollNonBlocking (
                this,
                pId,
                lId,
                ret,
                comm,
                collType,
                waveNumberInComm,
                request);

        newOp = nbOp;

        //Add to uncompleted nb ops
        nbOp->incRefCount();
        head->uncompletedNBOps[request].push_back(nbOp);
    }

    assert (head->trace.find(ret) == head->trace.end());
    head->trace[ret] = newOp;
    myTraceSize++;
    if (myTraceSize > myMaxTraceSize)
        myMaxTraceSize = myTraceSize;
    uint64_t tempTraceSize = myTraceSize;

    //Test for application break and trace size
    checkForBreakConsumeRequest (myTraceSize);

    //Advance
    advanceOp (newOp, head);

    //If the op is still in the queue, and no other op went, than mark this as a bad event
    if (tempTraceSize == myTraceSize)
        myFloodControl->markCurrentRecordBad();

    return ret;
}

//=============================
// notifyCollectiveLocalComplete
//=============================
void DWaitState::notifyCollectiveLocalComplete (
        std::list<std::pair<MustParallelId, MustLTimeStamp> > &ops
)
{
    std::list<std::pair<MustParallelId, MustLTimeStamp> >::iterator i;

    //Create a match info
    QCollectiveMatchInfo *info = new QCollectiveMatchInfo (ops.size());

    //Link all matching ops with the match info
    for (i = ops.begin(); i != ops.end(); i++)
    {
        MustParallelId pId = i->first;
        MustLTimeStamp ts = i->second;

        //Prepare
        int rank;
        DHeadInfo* head;
        if (!getRankAndHead (pId, &rank, &head))
            return;

        //Get the op
        std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find (ts);
        assert (pos != head->trace.end() && pos->second); //Must be there, must not be NULL!

        QOpCommunicationColl* op = pos->second->asOpCommunicationColl ();
        assert (op);

        op->setMatchInfo (info);

        //Advance
        advanceOp (op, head);
    }

    //Remove the one ownership we had of the match info
    info->erase();
}

//=============================
// getNodeForWorldRank
//=============================
int DWaitState::getNodeForWorldRank (int rank, bool *outIsThisNode)
{
    if (outIsThisNode) *outIsThisNode = false;

    //Get target node id
    /*
     * @todo one might cache this to reduce overheads,
     *            if so we should do the same in DP2PMatch
     */
    int targetPlace;
    if (getLevelIdForApplicationRank (rank, &targetPlace) != GTI_SUCCESS)
        return -1;

    if (myNodeId == targetPlace)
        if (outIsThisNode) *outIsThisNode = true;

    return targetPlace;
}

//=============================
// isMpiAnyTag
//=============================
bool DWaitState::isMpiAnyTag (int tag)
{
    return myConstants->isAnyTag(tag);
}

//=============================
// getCollRequestFunction
//=============================
generateCollectiveActiveRequestP DWaitState::getCollRequestFunction (void)
{
    return myFCollRequest;
}

//=============================
// getReceiveActiveRequestFunction
//=============================
generateReceiveActiveRequestP DWaitState::getReceiveActiveRequestFunction (void)
{
    return myFReceiveActiveRequest;
}

//=============================
// getReceiveActiveAcknowledge
//=============================
generateReceiveActiveAcknowledgeP DWaitState::getReceiveActiveAcknowledgeFunction (void)
{
    return myFReceiveActiveAcknowledge;
}

//=============================
// getProvideWaitSingleFunction
//=============================
provideWaitForInfosSingleP DWaitState::getProvideWaitSingleFunction (void)
{
    return myFProvideWaitSingle;
}

//=============================
// getProvideWaitMultiFunction
//=============================
provideWaitForInfosMixedP DWaitState::getProvideWaitMultiFunction (void)
{
    return myFProvideWaitMulti;
}

//=============================
// getProvideWaitCollFunction
//=============================
provideWaitForInfosCollP DWaitState::getProvideWaitCollFunction (void)
{
    return myFProvideWaitColl;
}

//=============================
// getProvideWaitNbcCollFunction
//=============================
provideWaitForInfosNbcCollP DWaitState::getProvideWaitNbcCollFunction (void)
{
    return myFProvideWaitNbcColl;
}

//=============================
// getProvideWaitNbcBackgroundFunction
//=============================
provideWaitForNbcBackgroundP DWaitState::getProvideWaitNbcBackgroundFunction (void)
{
    return myFProvideWaitNbcBackground;
}

//=============================
// advanceOp
//=============================
void DWaitState::advanceOp (QOp* op, DHeadInfo *head)
{
    /**
     * @todo
     * This function may recurse itself!
     * While I think its stable and sound, I might want to have a stack of heads and ops
     * to process here instead, that would simplify the analysis of this functions behavior.
     *
     * Currently for each given op ("op") we increment its reference count, as to
     * make sure that recursion does not invlidates the op in a prvious call context.
     * Before leaving we then decrement the reference count again.
     */

    assert (head); //must be valid

    //Ignore advancing for ops we have not yet reached
    if (op && (op->getTimeStamp() > head->activeTS))
        return;

    //Advance given op
    if (op)
    {
        //Increment ref count of given op (to avoid losing it due to recursion)
        op->incRefCount();

        //Notify of being active
        op->notifyActive();

        //If not the head op, we check whether we can remove the op from the trace
        if (op->getTimeStamp() != head->activeTS)
        {
            /*IMPORTANT: For debugging purposes, remove this block (It gives you in the dot output we generate with MUST_DWS_DEBUG a visualization of all ops we consider)*/
            if(!op->needsToBeInTrace())
            {
                std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(op->getTimeStamp());
                if (pos != head->trace.end())
                {
                    head->trace.erase(pos);
                    op->erase();
                    myTraceSize--;

                    //Test for application break and trace size
                    checkForBreakConsumeRequest (myTraceSize);
                }
            }
            /*IMPORTANT END*/
        }
    }

    //Advance current op of head
    QOp* headOp = op;
    if (!op || op->getTimeStamp() != head->activeTS)
    {
        //Find the op of the current timestamp, if there is none, return
        std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(head->activeTS);
        if (pos == head->trace.end())
        {
            //If we had a given op, decrease ref count
            if (op) op->erase();

            return;
        }

        //If we have an op we advance
        headOp = pos->second;
        assert (headOp);
        headOp->notifyActive();
    }

    //Are we still the headOp?
    /*This may be recursive so we may have already advanced!*/
    if (headOp->getTimeStamp() == head->activeTS)
    {
        //Check whether current op of head unblocked
        if (!headOp->blocks() && !myStopTime)
        {
            head->activeTS += 1;

            //Can we remove the head op from the trace?
            if(!headOp->needsToBeInTrace())
            {
/*IMPORTANT: For debugging purposes, remove this block (It gives you in the dot output we generate with MUST_DWS_DEBUG a visualization of all ops we consider)*/
                std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(headOp->getTimeStamp());
                if (pos != head->trace.end())
                {
                    head->trace.erase(pos);
                    myTraceSize--;

                    //Test for application break and trace size
                    checkForBreakConsumeRequest (myTraceSize);
                }
                headOp->erase();
            }
/*IMPORTANT END*/

            //If we just activated a P2P send operation, let DP2PMatch know
            std::map<MustLTimeStamp, QOp*>::iterator newPos = head->trace.find(head->activeTS);
            if (newPos != head->trace.end())
            {
                if (newPos->second->asOpCommunicationP2P() != NULL)
                    if (newPos->second->asOpCommunicationP2P()->isSend())
                        myDP2P->notifySendActivated (newPos->second->getIssuerRank(), head->activeTS);
            }

            //Recurse to advance to the next op!
            advanceOp (NULL, head);
        }
    }

    //If we had a given op, decrease ref count
    if (op) op->erase();

#ifdef MUST_DWS_DEBUG
    //static int time = 0;
    //time++;
    //std::stringstream stream;
    //stream << "debug__" << time;
    //printHeadsAsDot (stream.str());
#endif
}

//=============================
// wait
//=============================
GTI_ANALYSIS_RETURN DWaitState::wait (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return GTI_ANALYSIS_SUCCESS;

    MustLTimeStamp ret = head->getAndIncNextTS();

    //Create Op
    QOpCompletion* newOp = new QOpCompletion (
            this,
            pId,
            lId,
            ret,
            request);

    assert (head->trace.find(ret) == head->trace.end());
    head->trace[ret] = newOp;
    myTraceSize++;
    if (myTraceSize > myMaxTraceSize)
        myMaxTraceSize = myTraceSize;
    uint64_t tempTraceSize = myTraceSize;

    //Test for application break and trace size
    checkForBreakConsumeRequest (myTraceSize);

    //Advance
    advanceOp (newOp, head);

    //If the op is still in the queue, and no other op went, than mark this as a bad event
    if (tempTraceSize == myTraceSize)
        myFloodControl->markCurrentRecordBad();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitAny
//=============================
GTI_ANALYSIS_RETURN DWaitState::waitAny (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType* requests,
                int count,
                int numProcNull)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return GTI_ANALYSIS_SUCCESS;

    MustLTimeStamp ret = head->getAndIncNextTS();

    //Create Op
    QOpCompletion* newOp = new QOpCompletion (
            this,
            pId,
            lId,
            ret,
            count,
            requests,
            false,
            (numProcNull > 0));

    assert (head->trace.find(ret) == head->trace.end());
    head->trace[ret] = newOp;
    myTraceSize++;
    if (myTraceSize > myMaxTraceSize)
        myMaxTraceSize = myTraceSize;
    uint64_t tempTraceSize = myTraceSize;

    //Test for application break and trace size
    checkForBreakConsumeRequest (myTraceSize);

    //Advance
    advanceOp (newOp, head);

    //If the op is still in the queue, and no other op went, than mark this as a bad event
    if (tempTraceSize == myTraceSize)
        myFloodControl->markCurrentRecordBad();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitAll
//=============================
GTI_ANALYSIS_RETURN DWaitState::waitAll (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count,
                int numProcNull)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return GTI_ANALYSIS_SUCCESS;

    MustLTimeStamp ret = head->getAndIncNextTS();

    //Create Op
    QOpCompletion* newOp = new QOpCompletion (
            this,
            pId,
            lId,
            ret,
            count,
            requests,
            true,
            (numProcNull > 0));

    assert (head->trace.find(ret) == head->trace.end());
    head->trace[ret] = newOp;
    myTraceSize++;
    if (myTraceSize > myMaxTraceSize)
        myMaxTraceSize = myTraceSize;
    uint64_t tempTraceSize = myTraceSize;

    //Test for application break and trace size
    checkForBreakConsumeRequest (myTraceSize);

    //Advance
    advanceOp (newOp, head);

    //If the op is still in the queue, and no other op went, than mark this as a bad event
    if (tempTraceSize == myTraceSize)
        myFloodControl->markCurrentRecordBad();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitSome
//=============================
GTI_ANALYSIS_RETURN DWaitState::waitSome (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count,
                int numProcNull)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return GTI_ANALYSIS_SUCCESS;

    MustLTimeStamp ret = head->getAndIncNextTS();

    //Create Op
    QOpCompletion* newOp = new QOpCompletion (
            this,
            pId,
            lId,
            ret,
            count,
            requests,
            false,
            (numProcNull > 0));

    assert (head->trace.find(ret) == head->trace.end());
    head->trace[ret] = newOp;
    myTraceSize++;
    if (myTraceSize > myMaxTraceSize)
        myMaxTraceSize = myTraceSize;
    uint64_t tempTraceSize = myTraceSize;

    //Test for application break and trace size
    checkForBreakConsumeRequest (myTraceSize);

    //Advance
    advanceOp (newOp, head);

    //If the op is still in the queue, and no other op went, than mark this as a bad event
    if (tempTraceSize == myTraceSize)
        myFloodControl->markCurrentRecordBad();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completedRequest
//=============================
GTI_ANALYSIS_RETURN DWaitState::completedRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)
{
    return completedRequests (pId, lId, &request, 1);
}

//=============================
// completedRequests
//=============================
GTI_ANALYSIS_RETURN DWaitState::completedRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType *requests,
                int count)
{
    //Prepare
    int rank;
    DHeadInfo* head;
    if (!getRankAndHead (pId, &rank, &head))
        return GTI_ANALYSIS_SUCCESS;

    for (int i = 0; i < count; i++)
    {
        std::map<MustRequestType, std::list<QOpCommunication*> >::iterator pos;
        pos = head->uncompletedNBOps.find(requests[i]);

        /*We can not be too hard on asserts here, since completions are heavily filtered while the
         * completion updates are not*/
        if (pos == head->uncompletedNBOps.end())
            continue;

        if (pos->second.empty())
            continue;

        QOpCommunication *op = *(pos->second.begin());
        assert (op);
        op->erase();
        pos->second.pop_front();
        if (pos->second.empty())
        {
            head->uncompletedNBOps.erase(pos);
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// collectiveAcknowledge
//=============================
GTI_ANALYSIS_RETURN DWaitState::collectiveAcknowledge (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize)
{
    bool wasFinalize = false;

    /*
     * We go over all the heads, Loop over all the operations in the trace,
     * if we find a collective operation we compare its communicator
     * against the given information. If it matches we found our friend.
     * (the waitsForAcknowledge call tests whether an op already got its
     * acknowledge and makes sure we skip it then)
     * We then tell our friend that the acknowledge is there and let it progress,
     * we do this for all heads until we found all the crownies of our friend.
     *
     * @note This is not the best implementation, looking just at the head and
     *       looking at uncompletedNBOps (such that the ones with lower timestamps
     *       get priority) would be more efficient.
     */
    for (std::vector<DHeadInfo>::size_type i = 0; i < myHeads.size(); i++)
    {
        DHeadInfo* head = &(myHeads[i]);

        std::map<MustLTimeStamp, QOp*>::iterator pos;

        //If no op ignore
        for (pos = head->trace.begin(); pos != head->trace.end(); pos++)
        {
            //Is a collective? (We can ask this, it will give NULL if invalid)
            QOpCommunicationColl* op = pos->second->asOpCommunicationColl();

            if (!op) continue;

            //Is waiting for an acknowledge
            if (!op->waitsForAcknowledge(isIntercomm, contextId, localGroupSize, remoteGroupSize))
                continue;

            if (op->isFinalize())
                wasFinalize= true;

            //We should only be acknowledging ops that became active already,
            //otherwise something is off
            assert (head->activeTS >= op->getTimeStamp());

            //Pass the acknowledge on and advance this!
            op->notifyActiveAcknowledge();
            advanceOp(op, head);
            break;
        }
    }

    /*
     * We manually create the finalize event once we finished our analysis, i.e. acknowledged MPI_Finalize
     */
    if (wasFinalize)
    {
        finalizeMUSTP fFinalize;
        getWrapperFunction("finalizeMUST", (GTI_Fct_t*)&fFinalize);
        assert (fFinalize);
        (*fFinalize) ();

        //Make sure we forward the finalize event immediately
        /*Usually we should do that due to choice of strategy, but it can't really hurt*/
        gtiNotifyFlushP fFlush;
        if (getBroadcastFunction("gtiNotifyFlush", (GTI_Fct_t*) &fFlush) == GTI_SUCCESS)
            (*fFlush) ();
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// receiveActiveRequest
//=============================
GTI_ANALYSIS_RETURN DWaitState::receiveActiveRequest (
        int sendRank,
        MustLTimeStamp sendLTS,
        MustLTimeStamp receiveLTS)
{
    //Prepare
    DHeadInfo* head;
    if (!getRankAndHead (sendRank, NULL, &head))
        return GTI_ANALYSIS_SUCCESS;

    std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(sendLTS);
    assert (pos != head->trace.end()); //Must be there, since we already passed the send across
    assert (pos->second);

    QOpCommunicationP2P *op = pos->second->asOpCommunicationP2P();
    assert (op);

    //Notify the op of the ReceiveActiveRequest
    op->notifyGotReceiveActiveRequest (receiveLTS);

    //Advance
    advanceOp(op, head);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// receiveActiveAcknowledge
//=============================
GTI_ANALYSIS_RETURN DWaitState::receiveActiveAcknowledge (
        int receiveRank,
        MustLTimeStamp receiveLTS)
{
    //Prepare
    DHeadInfo* head;
    if (!getRankAndHead (receiveRank, NULL, &head))
        return GTI_ANALYSIS_SUCCESS;

    std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(receiveLTS);
    assert (pos != head->trace.end()); //Must be there, since we already passed the send across
    assert (pos->second);

    QOpCommunicationP2P *op = pos->second->asOpCommunicationP2P();
    assert (op);

    //Notify the op of the ReceiveActiveRequest
    op->notifyGotReceiveActiveAcknowledge();

    //Advance
    advanceOp(op, head);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// requestConsistentState
//=============================
GTI_ANALYSIS_RETURN DWaitState::requestConsistentState (void)
{
    /**
     * We first need to provide consistency, i.e., ensure that
     * P2Pops that are currently blocked are really blocked
     * within the epoch that we selected globaly.
     *
     * For that we will do a double ping-pong communication
     * with each TBON node on this layer for which any P2P op
     * waits. For wc recvs this is all nodes in the layer.
     *
     * For collectives we use the property that an outstanding
     * acknowledge will arrive consistently across all ranks due to
     * the extra up down syncronization that we create with
     * acknowledgeConsistentState.
     */

    //Stop advancing timestamps
    myStopTime = true;
    myNumOutstandingPingPongs = 0;
    std::set<int> pingPongs;

    //==Special handling if we did not init heads yet
    if (myHeads.empty())
    {
        /**
         *@todo: if we did not init heads yet, we can't set
         * head->wasDecremented for all heads.
         * Thus, we should remember that we got a request for CState when we
         * didn't init heads and then evaluate this when we provide WFG data.
         */
    }

    //==Loop over all heads
    // - Determine to whom we need to send ping-pongs
    // - For empty heads, reduce current timestamp by one; This forces us to really not consider any new op that comes in later; We correct this when we provide wait-for information
    for (std::vector<DHeadInfo>::size_type i = 0; i < myHeads.size(); i++)
    {
        DHeadInfo* head = &(myHeads[i]);

        //We must now loop over all currently active operations to gather our ping-pong list
        /**
         * Even non-blocking p2p ops stay in the trace until they got their "recv-became-active",
         * i.e., as long as we must ping-pong for them!
         */
        std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.begin();
        while (pos != head->trace.end() && pos->first <= head->activeTS)
        {
            if (pos->second)//should always be true
            {
                //gather ping-pong candidates
                std::set<int> temp = pos->second->getPingPongNodes();
                std::set<int>::iterator iter;
                for (iter = temp.begin(); iter != temp.end();iter++)
                    pingPongs.insert(*iter);
            }

            //Next
            pos++;
        }

        //We now also need to mark whether we had an active operation or not at the time of the
        //request for consistent state
        pos = head->trace.find(head->activeTS);
        if (pos == head->trace.end() || !pos->second)
        {
            //head->activeTS = head->activeTS - 1;
            head->wasDecremented = true;
            continue;
        }
    }

    //==Start all ping pongs
    std::set<int>::iterator iter;
    for (iter = pingPongs.begin(); iter != pingPongs.end(); iter++)
    {
        myNumOutstandingPingPongs++;

        if (myFPing)
            (*myFPing) (myNodeId, 1, *iter);
    }

    //==Check whether we can immediately acknowledge this request
    if (myNumOutstandingPingPongs == 0)
    {
        int headCount = myHeads.size();

        //If we get this request before we got our very first event we may not have initialized the heads!
        if (myHeads.empty())
        {
            getNumInputChannels(&headCount);
            myGotEarlyCStateRequest = true;
        }

        if (myFPAcknowledgeConsistent) (*myFPAcknowledgeConsistent) (headCount);
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handlePing
//=============================
GTI_ANALYSIS_RETURN DWaitState::handlePing (int fromNode, int pingsRemaining)
{
    //Just return the ping as a pong
    if (myFPong)
        (*myFPong) (myNodeId, pingsRemaining, fromNode);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handlePong
//=============================
GTI_ANALYSIS_RETURN DWaitState::handlePong (int fromNode, int pingsRemaining)
{
    if ((pingsRemaining > 0) && (fromNode >= 0)) //fromNode can be less than 0 if the remote site did not initialize heads yet
    {
        //Just start the next ping-ping
        if (myFPing)
            (*myFPing) (myNodeId, pingsRemaining-1, fromNode);
    }
    else
    {
        //Mark that we fininshed one double ping-pong, check whether we are done
        myNumOutstandingPingPongs--;

        if (myNumOutstandingPingPongs == 0)
            if (myFPAcknowledgeConsistent) (*myFPAcknowledgeConsistent) (myHeads.size());
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// requestWaitForInfos
//=============================
GTI_ANALYSIS_RETURN DWaitState::requestWaitForInfos (void)
{
    //==1) Generate the comm labels
    /**
     * @todo
     * This is broken, we need a global view to do that, either we communicate to exchange labeling,
     * or we put communicator information into our wait-for info replies, such that the root
     * can create labels. (Its a pure cosmetic issue, but needs to be fixed before we put this into
     * production!)
     */
    std::map<I_Comm*, std::string> commLabels = generateActiveCommLabels ();

    //==2) Provide background information on all active but uncompleted NBC ops
    /**
     * @note Keep this ordering of 2) happening before 3), we rely on that fact in
     *       DWaitStateWfgMgr!
     */
    for (std::vector<DHeadInfo>::size_type i = 0; i < myHeads.size(); i++)
    {
        DHeadInfo* head = &(myHeads[i]);

        std::map<MustLTimeStamp, QOp*>::iterator pos;

        //Loop over all ops! We keep uncompleted NBC ops in the trace
        //till they are completed! DO NOT USE uncompletedNBOps, since:
        //we remove there when MPI tells us to, possibly earlier then we
        //ourselves find that out; Thus we could lose meaningful matching
        //information that is relevant at DWaitStateWfgMgr
        for (pos = head->trace.begin(); pos != head->trace.end(); pos++)
        {
            QOp* op = pos->second;

            //Stop when we reach an inactive operation, these aren't part of the background
            if (op->getTimeStamp() > head->activeTS)
                break;

            //Is a collective? (We can ask this, it will give NULL if invalid)
            QOpCommunicationColl* collOp = op->asOpCommunicationColl();

            if (!collOp) continue;

            //Let the op handle the forwarding of the background
            collOp->handleNbcBackgroundForwarding();
        }
    }

    //==3) Loop over all heads and let them forward their wait-for information
    for (int i = 0; i < myHeads.size(); i++)
    {
        DHeadInfo* head = &(myHeads[i]);

        //Check whether we have a blocked op, if not we we send an empty reply for this head
        std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(head->activeTS);

        //If we decremented the activeTS to enforce us to not consider arriving ops,
        //we need to undo this
        bool wasDecrementedTemp = head->wasDecremented;
        //if (head->wasDecremented)
        //    head->activeTS = head->activeTS + 1;
        head->wasDecremented = false;

        bool isHeadSuspended = false;
        isHeadSuspended = myDP2P->isWorldRankSuspended(i+myFirstWorldRank);

        /**
         * @todo if a head is suspended, we must perform a probing approach
         *            this is not implemented yet and part of future work. We can
         *            however, tell the user which request he needs to complete to
         *            make us happy here, which we maybe should do.
         *            Currently we consider a head with a suspension as empty.
         */
        if (myGotEarlyCStateRequest || wasDecrementedTemp || isHeadSuspended || pos == head->trace.end() || !pos->second || !pos->second->blocks())
        {
            (*myFProvideWaitEmpty) (i + myFirstWorldRank);
            continue;
        }

        //If we have a blocked op, let the op decide abot the forwarding
        QOp* op = pos->second;
        op->forwardWaitForInformation(commLabels);
    }

    //==4) Special handling if we did not init heads yet
    if (myHeads.empty())
    {
        int headCount;
        getNumInputChannels(&headCount);
        for (int i = 0; i < headCount; i++)
            (*myFProvideWaitEmpty) (i); //the number argument "i" here is of no interest, its certainly not the right one!
    }

    //==5) Allow progress again and check for all heads whether we can advance
    myStopTime = false;
    myGotEarlyCStateRequest = false;
    for (std::vector<DHeadInfo>::size_type i = 0; i < myHeads.size(); i++)
    {
        DHeadInfo* head = &(myHeads[i]);
        advanceOp(NULL, head);
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// generateActiveCommLabels
//=============================
std::map<I_Comm*, std::string> DWaitState::generateActiveCommLabels (void)
{
    std::map<I_Comm*, std::string> ret;

    char cur = 'A';
    std::list<I_Comm*> comms, temp;
    std::list<I_Comm*>::iterator commIter, tempIter;

    for (std::vector<DHeadInfo>::size_type i = 0; i < myHeads.size(); i++)
    {
        DHeadInfo* head = &(myHeads[i]);

        std::map<MustLTimeStamp, QOp*>::iterator pos = head->trace.find(head->activeTS);
        if (pos == head->trace.end() || !pos->second || !pos->second->blocks())
            continue;

        QOp* op = pos->second;
        temp = op->getUsedComms();

        for (tempIter = temp.begin(); tempIter != temp.end(); tempIter++)
        {
            //Check wehther we already have this comm in our list
            for (commIter = comms.begin(); commIter != comms.end(); commIter++)
            {
                if ((*commIter)->compareComms(*tempIter))
                    break;
            }

            //If not add it
            if (commIter == comms.end())
            {
                comms.push_back(*tempIter);
                ret.insert (std::make_pair(*tempIter, std::string (1, cur)));
                cur++;
            }
        }
    }

    return ret;
}

//=============================
// checkForBreakConsumeRequest
//=============================
void DWaitState::checkForBreakConsumeRequest (int newTraceSize)
{
    //Overwrite thresholds with envs if given
    if (!myReadEnvs)
    {
        if (getenv("MUST_DWS_TRACE_SIZE_BREAK_THRESHOLD") != NULL)
        {
            myThresholdBreak = atol(getenv("MUST_DWS_TRACE_SIZE_BREAK_THRESHOLD"));
        }

        if (getenv("MUST_DWS_TRACE_SIZE_RESUME_THRESHOLD") != NULL)
        {
            myThresholdResume = atol(getenv("MUST_DWS_TRACE_SIZE_RESUME_THRESHOLD"));
        }

        assert (myThresholdBreak > myThresholdResume);
        myReadEnvs = true;
    }

    //Apply thrsholds
    if (!myVotedForBreak && newTraceSize > myThresholdBreak)
    {
        //Request a break
        (*myFPBreakRequest) ();
        myVotedForBreak = true;
        //std::cout << getpid () << "Requesting a break with size (" << newTraceSize << ")!" << std::endl;
    }
    else
    if (myVotedForBreak && newTraceSize < myThresholdResume)
    {
        //Request a consume
        (*myFPBreakConsume) ();
        myVotedForBreak = false;
        //std::cout << getpid () << "Requesting a consume with size (" << newTraceSize << ")!" << std::endl;
    }
}

/*EOF*/
