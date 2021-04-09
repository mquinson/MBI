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
 * @file Wfg.cpp
 *       @see must::Wfg.
 *
 *  @date 12.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "Wfg.h"
#include "wfglib.h"

#include <stack>

using namespace must;

//=============================
// NodeInfo
//=============================
NodeInfo::NodeInfo ()
: type (ARC_AND),
  inArcs (),
  outCount (0)
{
    //Nothing to do
}

//=============================
// Wfg
//=============================
Wfg::Wfg (void)
: myWfg (),
  myNextInternalId (0),
  myNodeIdToInternalId (),
  myInternalIdToNodeId ()
{
    //Nothing to do
}

//=============================
// ~Wfg
//=============================
Wfg::~Wfg (void)
{
    myWfg.clear();
}

//=============================
// addArc
//=============================
bool Wfg::addArc (int from, int to, ArcType type)
{
    if (myWfg.find (from) == myWfg.end())
    {
        myNodeIdToInternalId[from] = myNextInternalId;
        myInternalIdToNodeId[myNextInternalId] = from;
        myNextInternalId++;
    }
    myWfg[from].type = type;

    myWfg[from].outCount = myWfg[from].outCount + 1;
    myWfg[to].inArcs.push_back (from);

    if (myNodeIdToInternalId.find(to) == myNodeIdToInternalId.end())
    {
        myNodeIdToInternalId[to] = myNextInternalId;
        myInternalIdToNodeId[myNextInternalId] = to;
        myNextInternalId++;
    }
    return true;
}

//=============================
// detectDeadlock
//=============================
bool Wfg::detectDeadlock (bool *outHasDeadlock, std::list<int> *outDeadlockedNodes)
{
    //0)== Preprocess WFG
    /*
     * We apply signal reduction here already to only enter the more costy WFG lib
     * when necessary.
     * @todo the wfglib should be replaced at some point, we now have the signal
     *            reduction already here, all that remains is to add the OR-Knot detection.
     */
    //a) find sinks
    int numSinks = 0;
    std::stack<int> sinks;
    std::map<int, NodeInfo>::iterator nodeIter;
    for (nodeIter = myWfg.begin(); nodeIter != myWfg.end(); nodeIter++)
    {
        if (nodeIter->second.outCount == 0)
        {
            if (!nodeIter->second.inArcs.empty())
                sinks.push(nodeIter->first);
            numSinks++;
        }
    }

    //b) signal reduce
    while (!sinks.empty())
    {
        //Current sink
        int cur = sinks.top();
        NodeInfo &info = myWfg[cur];
        sinks.pop();

        //Iterate over the incoming arcs of the sink
        std::list<int>::iterator inIter;
        for (inIter = info.inArcs.begin(); inIter != info.inArcs.end(); inIter++)
        {
            int from = *inIter;
            NodeInfo &fromInfo = myWfg[from];
            if (fromInfo.outCount > 0)
            {
                if (fromInfo.type == ARC_AND)
                {
                    fromInfo.outCount = fromInfo.outCount - 1;
                }
                else
                {
                    fromInfo.outCount = 0;
                }

                if (fromInfo.outCount == 0)
                {
                    sinks.push(from);
                    numSinks++;
                }
            }
        }//for in-arcs of current sink
    }//while sinks

    //IMPORTANT: Early exit if signal reduction yields that no deadlock exists
    if (numSinks == myWfg.size())
    {
        if (outHasDeadlock)
            *outHasDeadlock = false;
        return true;
    }

    //1)== Init
    if (wfg_initialize (myInternalIdToNodeId.size()) != WFG_SUCCESS)
        return false;

    //2)== Add arcs
    std::map<int, NodeInfo>::iterator toIter;
    for (toIter = myWfg.begin();toIter != myWfg.end(); toIter++)
    {
        int to = toIter->first;

        std::list<int>::iterator fromIter;
        for (fromIter = toIter->second.inArcs.begin(); fromIter != toIter->second.inArcs.end(); fromIter++)
        {
            int from = *fromIter;

            WFG_ARC_TYPE type = WFG_ARC_AND;
            if (myWfg[from].type == ARC_OR)
                type = WFG_ARC_OR;

            if (wfg_add_arc (myNodeIdToInternalId[from], myNodeIdToInternalId[to], type) != WFG_SUCCESS)
                return false;
        }
    }

    //3)== Detect
    WFG_RETURN ret = wfg_deadlock_check ();
    if (ret == WFG_ERROR) return false;

    //4)== Prepare outputs
    if (ret == WFG_DEADLOCK)
    {
        int *pNodes = new int [myInternalIdToNodeId.size()];
        int count;
        if (wfg_get_deadlocked_nodes (&count, pNodes) != WFG_SUCCESS)
            return false;

        if (outDeadlockedNodes)
        {
            for (int i = 0; i < count; i++)
                outDeadlockedNodes->push_back(myInternalIdToNodeId[pNodes[i]]);
        }

        if (outHasDeadlock)
            *outHasDeadlock = true;

        if (pNodes) delete [] pNodes;
    }
    else
    {
        if (outHasDeadlock)
            *outHasDeadlock = false;
    }

    //5)== Clean up
    if (wfg_finalize () != WFG_SUCCESS)
        return false;

    return true;
}

/*EOF*/
