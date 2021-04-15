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
 * @file DWaitStateWfgMgr.cpp
 *       @see DWaitStateWfgMgr.
 *
 *  @date 08.03.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "MustDefines.h"
#include "Wfg.h"

#include "DWaitStateWfgMgr.h"

#include <assert.h>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

using namespace must;

mGET_INSTANCE_FUNCTION(DWaitStateWfgMgr)
mFREE_INSTANCE_FUNCTION(DWaitStateWfgMgr)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DWaitStateWfgMgr)

//10s in usec
#ifndef DWAITSTATEWFGMGR_TIMEOUT
#define DWAITSTATEWFGMGR_TIMEOUT 10000000
#endif

//=============================
// Constructor
//=============================
DWaitStateWfgMgr::DWaitStateWfgMgr (const char* instanceName)
    : gti::ModuleBase<DWaitStateWfgMgr, I_DWaitStateWfgMgr> (instanceName),
      myCommInfos (),
      myNodeInfos(),
      myWorldSize(-1),
      myNumReplies(0),
      myExpectedReplies (0),
      myNumConsistentReplies(0),
      myWfgRequestActive (false),
      myTSyncStart (0),
      myTSyncEnd (0),
      myTWfgInfoArrived (0),
      myTPrepFin (0),
      myTWfgCheckFin (0),
      myTOutputFin (0),
      myTDotFin (0)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBS 4
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
    myLIdMod = (I_LocationAnalysis*) subModInstances[1];
    myLogger = (I_CreateMessage*) subModInstances[2];
    myDColl = (I_DCollectiveMatchReduction*)  subModInstances[3];

    //Initialize module data
    ModuleBase<DWaitStateWfgMgr, I_DWaitStateWfgMgr>::getBroadcastFunction("requestWaitForInfos", (GTI_Fct_t*)&myFRequestInfos);
    assert (myFRequestInfos); //Must be there, otherwise we have a mapping error for this tool configuration

    ModuleBase<DWaitStateWfgMgr, I_DWaitStateWfgMgr>::getBroadcastFunction("requestConsistentState", (GTI_Fct_t*)&myFRequestConsistentState);
    assert (myFRequestConsistentState); //Must be there, otherwise we have a mapping error for this tool configuration

    //Register us to get infos about comms used in collectives
    myDColl->registerCommListener(this);

    //How many world ranks do we have
    int begin, end;
    getReachableRanks(&begin,&end,0);
    myWorldSize = end+1;

    //Init timeout start time
    myLastActivity = getUsecTime();
}

//=============================
// Destructor
//=============================
DWaitStateWfgMgr::~DWaitStateWfgMgr ()
{
    //Free module data
    std::list<commInfo>::iterator iter;
    for (iter = myCommInfos.begin(); iter != myCommInfos.end(); iter++)
        iter->comm->erase();
    myCommInfos.clear();
    myNodeInfos.clear();

    /*Free other data*/
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myLIdMod)
        destroySubModuleInstance ((I_Module*) myLIdMod);
    myLIdMod = NULL;

    if (myLogger)
        destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myDColl)
        destroySubModuleInstance ((I_Module*) myDColl);
    myDColl = NULL;
}

//=============================
// newCommInColl
//=============================
void DWaitStateWfgMgr::newCommInColl (
                MustParallelId pId,
                I_CommPersistent* comm)
{
    std::list<commInfo>::iterator iter;

    for (iter = myCommInfos.begin(); iter != myCommInfos.end(); iter++)
    {
        if (iter->comm->compareComms(comm))
            return;
    }

    commInfo newInfo;
    newInfo.comm = comm;
    comm->copy();
    myCommInfos.push_back (newInfo);
}

//=============================
// timeout
//=============================
void DWaitStateWfgMgr::timeout (void)
{
    if (myWfgRequestActive)
        return;

    //Did enough time pass to issue a request for WFT information
    if (getUsecTime() - myLastActivity > DWAITSTATEWFGMGR_TIMEOUT)
    {
        myExpectedReplies = myWorldSize;
        myNumReplies = 0;
        myWfgRequestActive = true;
        myNumConsistentReplies = 0;

        //Timing for experiments
        myTSyncStart = getUsecTime ();

        //We timed out; Request a consistent state!
        if (myFRequestConsistentState)
            (*myFRequestConsistentState) ();
    }

    return;
}

//=============================
// collActivityNotify
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::collActivityNotify (void)
{
    myLastActivity = getUsecTime();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// acknowledgeConsistentState
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::acknowledgeConsistentState (
                int numHeads)
{
    myNumConsistentReplies += numHeads;

    if (myNumConsistentReplies == myWorldSize)
    {
        //Timing for experiments
        myTSyncEnd = getUsecTime ();

        //If everyone is at a consistent state, request wait-for information
        if (myFRequestInfos)
            (*myFRequestInfos) ();
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitForInfoEmpty
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::waitForInfoEmpty (
                    int worldRank
            )
{
    /*
     * IMPORTANT:
     * worldRank may be broken if we send a request for consistent state before
     * DWaitState got its first event!
     */

    //Adapt number of replies
    myNumReplies++;

    if (myNumReplies == myWorldSize)
        compileCheckReport ();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitForInfoSingle
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::waitForInfoSingle (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int subId,
        int count,
        int type,
        int* toRanks,
        MustParallelId *labelPIds,
        MustLocationId *labelLIds,
        int labelsSize,
        char *labels
)
{
    //Calculate node id
    int nodeId = worldRank;
    if (subId != -1)
        nodeId = worldRank + (subId+1) * myWorldSize;

    //Create new info
    myNodeInfos.insert(std::make_pair(nodeId, nodeInfo()));
    nodeInfo *info = &(myNodeInfos[nodeId]);

    //Fill new info
    info->worldRank = worldRank;
    if (subId != -1)
        info->isSubNode = true;
    else
        info->isSubNode = false;
    info->lId = lId;
    info->pId = pId;
    info->nodeId = nodeId;
    info->type = (ArcType) type;

    //Seperate the labels
    std::istringstream stream (labels);
    std::string label;
    while (std::getline(stream, label, '\n'))
    {
        info->toLabels.push_back(label);
    }

    //Add to the toRanks
    for (int i = 0; i < count; i++)
    {
        info->toNodes.push_back(toRanks[i]);
    }

    //Adapt number of replies
    myNumReplies++;

    if (myNumReplies == myExpectedReplies)
        compileCheckReport ();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitForInfoMixed
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::waitForInfoMixed (
         int worldRank,
         MustParallelId pId,
         MustLocationId lId,
         int numSubs,
         int type,
         MustParallelId *labelPIds,
         MustLocationId *labelLIds,
         int labelsSize,
         char *labels
 )
{
    //Calculate node id
    int nodeId = worldRank;

    //Create new info
    myNodeInfos.insert(std::make_pair(nodeId, nodeInfo()));
    nodeInfo *info = &(myNodeInfos[nodeId]);

    //Fill new info
    info->worldRank = worldRank;
    info->isSubNode = false;
    info->lId = lId;
    info->pId = pId;
    info->nodeId = nodeId;
    info->type = (ArcType) type;

    //Seperate the labels
    std::istringstream stream (labels);
    std::string label;
    while (std::getline(stream, label, '\n')) {
        info->toLabels.push_back(label);
    }

    //Add to the toRanks
    for (int i = 0; i < numSubs; i++)
    {
        info->toNodes.push_back((i+1)*myWorldSize+worldRank);
    }

    //Adapt number of replies
    myExpectedReplies+=numSubs;
    myNumReplies++;

    if (myNumReplies == myExpectedReplies)
        compileCheckReport ();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitForInfoColl
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::waitForInfoColl (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int collType,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
)
{
    //Get the comm info
    commInfo* info = getCommInfo (
            isIntercomm,
            contextId,
            localGroupSize,
            remoteGroupSize);
    assert (info); //We should know about all the comms ...

    //Add to the comm info
    info->colls[(MustCollCommType)collType][worldRank] = std::make_pair(pId,lId);

    //Adapt number of replies
    myNumReplies++;

    if (myNumReplies == myExpectedReplies)
        compileCheckReport ();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitForBackgroundNbc
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::waitForBackgroundNbc (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int waveNumInColl,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
)
{
    //Get the comm info
    commInfo* info = getCommInfo (
            isIntercomm,
            contextId,
            localGroupSize,
            remoteGroupSize);
    assert (info); //We should know about all the comms ...

    //Add to active Nbcs
    info->activeNbcs[waveNumInColl][worldRank] = std::make_pair(pId,lId);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitForInfoNbcColl
//=============================
GTI_ANALYSIS_RETURN DWaitStateWfgMgr::waitForInfoNbcColl (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int subId,
        int waveNumInColl,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
)
{
    //Get the comm info
    commInfo* info = getCommInfo (
            isIntercomm,
            contextId,
            localGroupSize,
            remoteGroupSize);
    assert (info); //We should know about all the comms ...

    //Calculate node id
    int nodeId = worldRank;
    if (subId != -1)
        nodeId = worldRank + (subId+1) * myWorldSize;

    //Add to waiting Nbc ops
    info->waitingNbcs[nodeId][waveNumInColl] = std::make_pair(pId,lId);

    //Adapt number of replies
    myNumReplies++;

    if (myNumReplies == myExpectedReplies)
        compileCheckReport ();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// compileCheckReport
//=============================
void DWaitStateWfgMgr::compileCheckReport (void)
{
    //Timing for experiments
    myTWfgInfoArrived = getUsecTime ();

    //==1) Compile collectives into regular nodes
    std::list<commInfo>::iterator citer;
    for (citer = myCommInfos.begin(); citer != myCommInfos.end(); citer++)
    {
        if (citer->colls.size() == 0 && citer->waitingNbcs.size() == 0)
            continue;

        I_Comm* comm = citer->comm;

        //Sizes
        int groupSize = comm->getGroup()->getSize(),
            remoteSize = 0;

        if (comm->getRemoteGroup())
            remoteSize = comm->getRemoteGroup()->getSize();

        /*
         * @todo think about intercommunicators here.
         */
        std::map<MustCollCommType, std::map<int, std::pair<MustParallelId, MustLocationId> > >::iterator typeIter;
        for (typeIter = citer->colls.begin(); typeIter != citer->colls.end(); typeIter++)
        {
            std::map<int, std::pair<MustParallelId, MustLocationId> > &participants = typeIter->second;
            /*
             * If all ranks in comm wait in this collective, nothing to do,
             * we just didn't notice that they unblocked
             */
            if (participants.size() == groupSize)
                break;

            std::map<int, std::pair<MustParallelId, MustLocationId> >::iterator pIter;
            for (pIter = participants.begin();  pIter != participants.end(); pIter++)
            {
                //Add a node for this head
                myNodeInfos.insert(std::make_pair(pIter->first, nodeInfo()));
                nodeInfo *info = &(myNodeInfos[pIter->first]);

                info->isSubNode = false;
                info->lId = pIter->second.second;
                info->pId = pIter->second.first;
                info->nodeId = pIter->first;
                info->type = ARC_AND;
                info->worldRank = pIter->first;

                //Add arcs
                for (int i = 0; i < groupSize; i++)
                {
                    int wRank;
                    comm->getGroup()->translate(i,&wRank);

                    //If this rank is in this collective group, continue
                    if (participants.find(wRank) != participants.end())
                        continue;

                    info->toNodes.push_back(wRank);
                    info->toLabels.push_back("");
                }
            }
        }

        //-----------------------------------------------------------
        //B) Compile NBC information into nodes
        //-----------------------------------------------------------
        //B.I) Which wave IDs have waiting operations
        std::set<int> wavesWithWaits;

        std::map<int, std::map<int, std::pair<MustParallelId, MustLocationId> > >::iterator waitingIter;
        for (waitingIter = citer->waitingNbcs.begin(); waitingIter != citer->waitingNbcs.end(); waitingIter++)
        {
            std::map<int, std::pair<MustParallelId, MustLocationId> >::iterator waitingWaveIter;
            for (waitingWaveIter = waitingIter->second.begin();
                    waitingWaveIter != waitingIter->second.end();
                    waitingWaveIter++)
            {
                wavesWithWaits.insert(waitingWaveIter->first);
            }
        }

        //B.II) For each used wave ID: Compile a list of which world ranks are missing in the NBC
        std::map<int, std::list<int> > waveIdMissingRanks; //Maps wave ID to a list of missing ranks

        std::set<int>::iterator waveIdIter;
        for (waveIdIter = wavesWithWaits.begin(); waveIdIter != wavesWithWaits.end(); waveIdIter++)
        {
            int waveId = *waveIdIter;
            waveIdMissingRanks[waveId] = std::list<int> ();

            //Find who is missing
            for (int i = 0; i < groupSize; i++)
            {
                int wRank;
                comm->getGroup()->translate(i, &wRank);

                //If this rank is in this collective group, continue
                if (citer->activeNbcs[waveId].find(wRank) == citer->activeNbcs[waveId].end())
                    waveIdMissingRanks[waveId].push_back(wRank);
            }

        }

        //B.III) For waiting NBC operation, go over the list of missing world ranks and add their nodes
        for (waitingIter = citer->waitingNbcs.begin(); waitingIter != citer->waitingNbcs.end(); waitingIter++)
        {
            int fromNodeId = waitingIter->first;

            //Add a node for this head
            myNodeInfos.insert(std::make_pair(fromNodeId, nodeInfo()));
            nodeInfo *info = &(myNodeInfos[fromNodeId]);

            info->isSubNode = false;
            if (fromNodeId >= myWorldSize)
                info->isSubNode = true;
            info->nodeId = fromNodeId;
            info->worldRank = info->nodeId % myWorldSize;
            info->type = ARC_AND;

            assert (waitingIter->second.size () == 1); // Otherwise there would be two NBC dependency sets being applied to the same subId!

            std::map<int, std::pair<MustParallelId, MustLocationId> >::iterator waitingWaveIter;
            for (waitingWaveIter = waitingIter->second.begin();
                    waitingWaveIter != waitingIter->second.end();
                    waitingWaveIter++)
            {
                info->lId = waitingWaveIter->second.second;
                info->pId = waitingWaveIter->second.first;

                int waveId = waitingWaveIter->first;

                std::list<int>::iterator toWRankIter;
                for (toWRankIter = waveIdMissingRanks[waveId].begin(); toWRankIter != waveIdMissingRanks[waveId].end(); toWRankIter++)
                {
                    info->toNodes.push_back(*toWRankIter);
                    info->toLabels.push_back("");
                }
            }
        }
    }

    //Clean up all collective infos for any participants
    for (citer = myCommInfos.begin(); citer != myCommInfos.end(); citer++)
    {
        citer->colls.clear();
        citer->activeNbcs.clear();
    }

    //==2) Build a WFG and do the check
    Wfg wfg;

    std::map<int, nodeInfo>::iterator nIter;
    for (nIter = myNodeInfos.begin(); nIter != myNodeInfos.end(); nIter++)
    {
        std::list<int>::iterator toIter;
        for (toIter = nIter->second.toNodes.begin(); toIter != nIter->second.toNodes.end(); toIter++)
            wfg.addArc(nIter->second.nodeId, *toIter, nIter->second.type);
    }

    //Timing for experiments
    myTPrepFin = getUsecTime ();

    bool hasDeadlock = false;
    std::list<int> deadlockedNodes;
    wfg.detectDeadlock(&hasDeadlock, &deadlockedNodes);

    ////////DEBUGGING: Enforce that we print the whole WFG
    //hasDeadlock = true;
    //std::map<int, nodeInfo>::iterator debugNodeIter;
    //deadlockedNodes.clear();
    //for (debugNodeIter = myNodeInfos.begin(); debugNodeIter != myNodeInfos.end(); debugNodeIter++)
    //{
    //    deadlockedNodes.push_back(debugNodeIter->first);
    //}
    ////////END DEBUGGING

    //Timing for experiments
    myTWfgCheckFin = getUsecTime ();

    //==3) Do we have a deadlock
    if (hasDeadlock)
    {
        reportDeadlock (deadlockedNodes);
    }

    //==4) Reset
    myExpectedReplies = myWorldSize;
    myNumReplies = 0;
    myWfgRequestActive = false;
    myNodeInfos.clear();
    struct timeval t;
    gettimeofday(&t, NULL);
    myLastActivity = t.tv_sec * 1000000 + t.tv_usec;
}

//=============================
// reportDeadlock
//=============================
/**
 * @todo a good amount of this stuff is copy&paste from BlockingState.cpp.
 *
 * I want to have independence over the two versions for the moment, but in the long
 * run this is rather unwanted. Basic question that is still open is, is whether we can
 * ultimately remove all of the centralized implementations and make the distributed
 * ones the default, but this requires extensive testing and some more performance
 * study.
 */
void DWaitStateWfgMgr::reportDeadlock (std::list<int> nodes)
{
    std::list<int>::iterator nodeIter;

    //==1) Build DOT graph
    /**
     * @todo centralized case has a message queue graph
     * and the message queue graph component view as a parallel call stack.
     */
    //gamma) locations to include in a parallel call stack (if we have stack tracing)
    std::list<std::pair<MustParallelId, MustLocationId> > callStackLocations;

    //a) Mark all deadlocked nodes
    for (nodeIter = nodes.begin(); nodeIter != nodes.end(); nodeIter++)
    {
        int node = *nodeIter;
        myNodeInfos[node].isDeadlocked = true;
    }

    //b) Prepare the DOT output
    static int printCount = 0;
    std::ofstream out;
    MUST_OUTPUT_DIR_CHECK
    if (printCount == 0)
    {
        out.open ((((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_Deadlock.dot")).c_str());
    }
    else
    {
        std::stringstream stream;
        stream << MUST_OUTPUT_DIR << "MUST_Deadlock_" << printCount << ".dot";
        out.open (stream.str().c_str());
    }
    printCount++;
    out << "digraph Deadlock {" << std::endl;
#ifdef DOT
        out << "graph [bgcolor=transparent]" << std::endl;
#endif

    //c) Iterate over all deadlocked nodes
    for (nodeIter = nodes.begin(); nodeIter != nodes.end(); nodeIter++)
    {
        int from = *nodeIter;
        nodeInfo* info = &(myNodeInfos[from]);

        std::string callName = "";

        //add to call stack locations
        if (info->pId != 0 || info->lId != 0)
        {
            callStackLocations.push_back(std::make_pair(info->pId, info->lId));
            callName = myLIdMod->getInfoForId(info->pId, info->lId).callName;
        }

        //Print this node
        out << from << " [label=\"";
        if (!info->isSubNode)
            out << "{";
        out << info->worldRank << ": " << callName;

        /*
         * @todo for collectives we should label the comms
         */

        if (!info->isSubNode)
            out << "}\", shape=record];" << std::endl;
        else
            out << "\", shape=hexagon];" << std::endl;

        std::string style = "solid";
        if (info->type == ARC_OR) style = "dashed";

        //Iterate over all to arcs
        std::list<int>::iterator toIter;
        std::list<std::string>::iterator labelIter;

        for (toIter = info->toNodes.begin(), labelIter = info->toLabels.begin();
                toIter != info->toNodes.end();
                toIter++, labelIter++)
        {
            int to = *toIter;

            //Is an arc to a deadlocked node ?
            if (!myNodeInfos[to].isDeadlocked) continue;

            //Labels
            std::string label = "";

            if (labelIter != info->toLabels.end()) label = *labelIter;

            //Add to graph
            out
                << from << "->" << to << "[label=\"" << label << "\", style=" << style << "];" << std::endl;
        }
    }

    //d) Close the DOT output
    out << std::endl << "}" << std::endl;
    out.close ();

    //------------------------------------
    //3b) Create a DOT legend

    out.open ( (((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_DeadlockLegend.dot")).c_str() );

    out
        << "digraph DeadlockLegend {" << std::endl;
#ifdef DOT
        out << "graph [bgcolor=transparent]" << std::endl;
#else
    out
        << "  subgraph cluster0" << std::endl
        << "  {" << std::endl
        << "    color = black;" << std::endl
        << "    style = rounded;" << std::endl
        << "    label = \"Legend\";" << std::endl
        << "    " << std::endl;
#endif
    out
        << "    box [label=\"Active MPI Call\", shape=box];" << std::endl
        << "    dia [label=\"Sub Operation\", shape=hexagon];" << std::endl
        << "    A [label=\"A\", shape=box];" << std::endl
        << "    B [label=\"B\", shape=box];" << std::endl
        << "    C [label=\"C\", shape=box];" << std::endl
        << "    A2 [label=\"A\", shape=box];" << std::endl
        << "    B2 [label=\"B\", shape=box];" << std::endl
        << "    C2 [label=\"C\", shape=box];" << std::endl
        << "    " << std::endl
        << "    box->dia [style=invis];" << std::endl
        << "    dia->A [style=invis];" << std::endl
        << "    dia->B [style=invis];" << std::endl
        << "    " << std::endl
        << "    {rank=same; A ; B };" << std::endl
        << "    " << std::endl
        << "    A->B [label=\"A waits for B and C\"];" << std::endl
        << "    A->C [label=\"\"];" << std::endl
        << "    " << std::endl
        << "    B->C [style=invis, weight=10]" << std::endl
        << "    " << std::endl
        << "    C->B2 [style=invis];" << std::endl
        << "    A->A2 [style=invis];" << std::endl
        << "    " << std::endl
        << "    A2->C2 [style=dashed];" << std::endl
        << "    A2->B2 [label=\"A waits for B or C\" style=dashed];" << std::endl
        << "    " << std::endl
        << "    B2->C2 [style=invis, weight=10]" << std::endl
        << "    " << std::endl
        << "    {rank=same; A2 ; B2 };" << std::endl;
#ifndef DOT
        out << "  }" << std::endl;
#endif /*DOT*/
    out << "}" << std::endl;
    out.close ();

    //------------------------------------
    //3c) create a parallel call stack graph
#ifdef USE_CALLPATH
    /**
     * @todo generate a call stack see BlockingState
     * We already have the list of pid,lid pairs for that !
     */
#endif /*USE_CALLPATH*/

      /*
       * @todo communicator overview view
       */
      //Print the two maps as dot
#ifdef DOT
      out.open ((((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_Deadlock.html")).c_str());

      char buf[128];
      struct tm *ptr;
      time_t tm;
      tm = time(NULL);
      ptr = localtime(&tm);
      strftime(buf ,128 , "%c.\n",ptr);

      //print the header
      out
      << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">" << std::endl
      << "<html>" << std::endl
      << "<head>" << std::endl
      << "<title>MUST Outputfile</title>" << std::endl
      << "<style type=\"text/css\">" << std::endl
      << "td,td,table {border:thin solid black}" << std::endl
      << "td.ee1{ background-color:#FFDDDD; text-align:center; vertical-align:middle;}" << std::endl
      << "td.ee2{ background-color:#FFEEEE; text-align:center; vertical-align:middle;}" << std::endl
      << "</style>" << std::endl
      << "</head>" << std::endl
      << "<body>" << std::endl
      << "<p> <b>MUST Deadlock Details</b>, date: "
      << buf
      << "</p>" << std::endl
      << "<a href=\"../MUST_Output.html\" title=\"MUST error report\">Back to MUST error report</a><br>" << std::endl
      << "<table border=\"0\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">" << std::endl


      << "<tr>" << std::endl
      << "<td align=\"center\" bgcolor=\"#9999DD\" colspan=\"2\">" << std::endl
      << "<b>Message</b>"<< std::endl
      << "</td>" << std::endl
      << "</tr>" << std::endl
      << "<tr>" << std::endl
      << "<td class=\"ee2\" colspan=\"3\" >" << std::endl
      << "The application issued a set of MPI calls that can cause a deadlock!" << std::endl
      << " The graphs below show details on this situation." << std::endl
      << " This includes a wait-for graph that shows" << std::endl
      << " active wait-for dependencies between the processes that cause the deadlock." << std::endl
      << " Note that this process set only includes processes that cause the deadlock and no further processes." << std::endl
      << " A legend details the wait-for graph components in addition" << std::endl
#ifdef USE_CALLPATH
//      << " , while a parallel call stack view summarizes the locations of the MPI calls that cause the deadlock" << std::endl
#endif /*USE_CALLPATH*/
//      << ". Below these graphs, a message queue graph shows active and unmatched point-to-point communications." << std::endl
//      << " This graph only includes operations that could have been intended to match a point-to-point operation that is relevant to the deadlock situation."
#ifdef USE_CALLPATH
//      << "  Finally, a parallel call stack shows the locations of any operation in the parallel call stack." << std::endl
//      << " The leafs of this call stack graph show the components of the message queue graph that they span." << std::endl
#endif /*USE_CALLPATH*/
      << " The application still runs, if the deadlock manifested" << std::endl
      << " (e.g. caused a hang on this MPI implementation) you can attach to the involved ranks" << std::endl
      << " with a debugger or abort the application (if necessary)." << std::endl
      << "</td>" << std::endl
      << "</tr>"  << std::endl

/*      << "<!-- ACTIVE COMMS -->" << std::endl
      << "<tr>" << std::endl
      << "<td align=\"center\" bgcolor=\"#7777BB\" colspan=\"2\">" << std::endl
      << "<b>Active Communicators</b>" << std::endl
      << "</td>" << std::endl
      << "</tr>" << std::endl
      << "<tr>" << std::endl
      << "<td class=\"ee1\" colspan=\"2\" >" << std::endl
      << "<!-- Embedded table for communicator overview -->" << std::endl
      << commOverview->str() << std::endl
      << "<!-- End embedded table -->" << std::endl
      << "</td>" << std::endl
      << "</tr>" << std::endl
*/
      << "<tr>" << std::endl
      << "<td align=\"center\" bgcolor=\"#7777BB\">"
      << "<b>Wait-for Graph</b>"
      << "</td>" << std::endl
      << "<td align=\"center\" bgcolor=\"#9999DD\">"
      << "<b>Legend</b>"
      << "</td>" << std::endl
      << "</tr>" << std::endl
      << "<tr>" << std::endl
      << "<td class=\"ee2\" ><img src=\"MUST_Deadlock.png\" alt=\"deadlock\"></td>" << std::endl
#ifdef  USE_CALLPATH
      << "<td class=\"ee1\" rowspan=\"3\" >" << std::endl
#else
      << "<td class=\"ee1\" >" << std::endl
#endif
      << "<img src=\"MUST_DeadlockLegend.png\" alt=\"legend\"></td>" << std::endl
      << "</tr>" << std::endl
#ifdef  USE_CALLPATH
      << "<tr>" << std::endl
      << "<td align=\"center\" bgcolor=\"#9999DD\"><b>Call Stack</b></td>" << std::endl
      << "</tr>" << std::endl
      << "<tr>" << std::endl
      << "<td class=\"ee1\" ><img src=\"MUST_DeadlockCallStack.png\" alt=\"stack\"></td>" << std::endl
      << "</tr>" << std::endl
#endif
/*
      << "<!-- RELEVANT P2P: Overview -->" << std::endl
      << "<tr>" << std::endl
      << "<td align=\"center\" bgcolor=\"#7777BB\" colspan=\"2\">" << std::endl
      << "<b>Active and Relevant Point-to-Point Messages: Overview</b>" << std::endl
      << "</td>" << std::endl
      << "</tr>" << std::endl
      << "<tr>" << std::endl
      << "<td class=\"ee2\" colspan=\"2\" ><img src=\"MUST_DeadlockMessageQueue.png\" alt=\"Message queue\"></td>" << std::endl
      << "</tr>" << std::endl
*/
#ifdef  USE_CALLPATH
/*      << "<tr>" << std::endl
      << "<td align=\"center\" bgcolor=\"#9999DD\"><b>Active and Relevant Point-to-Point Messages: Callstack-view</b></td>" << std::endl
      << "</tr>" << std::endl
      << "<tr>" << std::endl
      << "<td class=\"ee1\" ><img src=\"MUST_DeadlockMessageQueueStacked.png\" alt=\"stack\"></td>" << std::endl
      << "</tr>" << std::endl*/
#endif

      << "</table>" << std::endl
      << "</body>" << std::endl
      << "</html>" << std::endl;
      out.flush();
      out.close();
#endif /*DOT*/

    //==2) Generate error report
    std::stringstream stream;
    std::list <std::pair <MustParallelId, MustLocationId> > refs;

    for (nodeIter = nodes.begin(); nodeIter != nodes.end() && refs.size() < 5; nodeIter++)
    {
        int from = *nodeIter;

        //Remove sub nodes, they are not meaningful here
        if (from >= myWorldSize)
            continue;

        nodeInfo *info = &(myNodeInfos[from]);

        if ((info->pId != 0) || (info->lId != 0))
            refs.push_back(std::make_pair(info->pId, info->lId));
    }

#ifdef DOT
    stream
    << "The application issued a set of MPI calls that can cause a deadlock!"
    << " A graphical representation of this situation is available in a"
    << " <a href=\"" << MUST_OUTPUT_DIR << "MUST_Deadlock.html\" title=\"detailed deadlock view\"> detailed deadlock view ("<< MUST_OUTPUT_DIR << "MUST_Deadlock.html)</a>."
    << " References 1-" << refs.size() << " list the involved calls (limited to the first 5 calls, further calls may be involved)."
    << " The application still runs, if the deadlock manifested (e.g. caused a hang on this MPI implementation) you can attach to the involved ranks with a debugger"
    << " or abort the application (if necessary).";
#else
    stream
    << "The application issued a set of MPI calls that can cause a deadlock!"
    << " A graphical representation of this situation is available in the file named \""<<MUST_OUTPUT_DIR<<"MUST_Deadlock.dot\"."
    << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<<MUST_OUTPUT_DIR<<"MUST_Deadlock.dot -o deadlock.ps\"."
    << " The graph shows the nodes that form the root cause of the deadlock, any other active MPI calls have been removed."
    << " A legend is available in the dot format in the file named \""<<MUST_OUTPUT_DIR<<"MUST_DeadlockLegend.dot\", further information on these graphs is available in the MUST manual."
    << " References 1-" << refs.size() << " list the involved calls (limited to the first 5 calls, further calls may be involved)."
    << " The application still runs, if the deadlock manifested (e.g. caused a hang on this MPI implementation) you can attach to the involved ranks with a debugger"
    << " or abort the application (if necessary).";
#endif
    myLogger->createMessage(MUST_ERROR_DEADLOCK, MustErrorMessage, stream.str(), refs);


    //==3) Print to std::cerr
    std::cerr
        << "============MUST===============" << std::endl
        << "ERROR: MUST detected a deadlock, detailed information is available in the MUST output file."
        << " You should either investigate details with a debugger or abort, the operation of MUST will stop from now." << std::endl
        << "===============================" << std::endl;

    //Timing for experiments
    myTOutputFin = getUsecTime ();

    //------------------------------------
    //3c) draw graphs from generated DOT files
#ifdef DOT
    std::string command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng ") +((std::string)MUST_OUTPUT_DIR) + ((std::string)"MUST_Deadlock.dot -o ") +((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_Deadlock.png");
    system (command.c_str());
    ////command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueue.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueue.png");
    ////system (command.c_str());
#ifdef  USE_CALLPATH
    //command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockCallStack.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockCallStack.png");
    //system (command.c_str());
    ////command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueueStacked.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueueStacked.png");
    ////system (command.c_str());
#endif /*USE_CALLPATH*/
    command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) "  -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockLegend.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockLegend.png");
    system (command.c_str());
#endif /* DOT */

    //Timing for experiments
    myTDotFin = getUsecTime ();

    //Print experimentatition timing
    std::cerr
        << "----Deadlock detection timing ----" << std::endl
        << "syncTime=" << myTSyncEnd - myTSyncStart << std::endl
        << "wfgGatherTme=" << myTWfgInfoArrived - myTSyncEnd << std::endl
        << "preparationTime=" << myTPrepFin - myTWfgInfoArrived << std::endl
        << "wfgCheckTime=" << myTWfgCheckFin - myTPrepFin << std::endl
        << "outputTime=" << myTOutputFin - myTWfgCheckFin << std::endl
        << "dotTime=" << myTDotFin - myTOutputFin << std::endl;

    /**
     * @todo currently we hard code an abort, this is not the production method we had in the
     * centralized case, but since I want to do some measurements of this, it makes my life
     * simpler for the time being.
     */
    PMPI_Abort (MPI_COMM_WORLD,666);
}

//=============================
// getCommInfo
//=============================
DWaitStateWfgMgr::commInfo* DWaitStateWfgMgr::getCommInfo (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize)
{
    //Search for the right comm info
    std::list<commInfo>::iterator iter;
    for (iter = myCommInfos.begin(); iter != myCommInfos.end(); iter++)
    {
        if (iter->comm->isIntercomm() != (bool) isIntercomm)
            continue;

        int firstRankOfW = 0;
        if (iter->comm->getGroup() && !iter->comm->getRemoteGroup())
            iter->comm->getGroup()->translate(0, &firstRankOfW);

        if (iter->comm->getContextId()+firstRankOfW != contextId)
            continue;

        if (isIntercomm)
        {
            if ((iter->comm->getGroup()->getSize() == localGroupSize && iter->comm->getRemoteGroup()->getSize() == remoteGroupSize) ||
                    (iter->comm->getGroup()->getSize() == remoteGroupSize && iter->comm->getRemoteGroup()->getSize() == localGroupSize) )
                break;
        }
        else
        {
            if (iter->comm->getGroup()->getSize() == localGroupSize)
                break;
        }
    }

    if (iter != myCommInfos.end())
        return &(*iter);
    return NULL;
}

/*EOF*/
