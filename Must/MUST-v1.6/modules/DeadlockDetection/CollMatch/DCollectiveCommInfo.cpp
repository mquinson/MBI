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
 * @file DCollectiveCommInfo.cpp
 *       @see must::CollectiveCommInfo.
 *
 *  @date 26.04.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#include "DCollectiveCommInfo.h"

#include <assert.h>

using namespace must;

//=============================
// Constructor
//=============================
DCollectiveCommInfo::DCollectiveCommInfo (I_CommPersistent* comm)
: myComm (comm),
  myNumReachableRanks (0),
  myNextWaveNumber (0),
  myAggregate (true),
  myStride (1),
  myOffset (),
  myActiveWaves (),
  myTimedOutWaves (),
  myWaitingForIntraWaves (),
  myUnexpectedTypeMatchInfos ()
{
    /**
     * @todo stride detection should use services from the I_Group interface, which should
     *           implement a stride representation beforehand.
     */
    std::set<int> reachableWRanks;

    //Determine total number of tasks reachable by this node
    if (!myComm->isNull() && myComm->getGroup())
    {
        for (int i = 0; i < myComm->getGroup()->getSize(); i++)
        {
            if (myComm->isRankReachable(i))
            {
                myNumReachableRanks++;

                int wRank;
                myComm->getGroup()->translate(i, &wRank);
                reachableWRanks.insert(wRank);
            }
        }
    }

    //For intercomms we also need to add the remote group ranks!
    if (!myComm->isNull() && myComm->isIntercomm() && myComm->getRemoteGroup())
    {
        for (int i = 0; i < myComm->getRemoteGroup()->getSize(); i++)
        {
            if (myComm->isRankReachable(i))
            {
                myNumReachableRanks++;

                int wRank;
                myComm->getRemoteGroup()->translate(i, &wRank);
                reachableWRanks.insert(wRank);
            }
        }
    }

    //Detect stride if present! (See inital to-do comment above)
    if (reachableWRanks.size() == 1)
    {
        myStride = 0;
        myOffset = *(reachableWRanks.begin());
    }
    else if (reachableWRanks.size() > 1)
    {
        std::set<int>::iterator iter = reachableWRanks.begin();

        int a = *iter;
        iter ++;
        int b = *iter;
        iter++;
        int stride = b - a;

        if (stride != 1)
        {
            for (int cur = b+stride; iter != reachableWRanks.end(); iter++, cur += stride)
            {
                if (cur != *iter)
                    break;
            }

            if (iter == reachableWRanks.end())
            {
                myOffset = a;
                myStride = stride;
            }
        }
    }
}

//=============================
// Destructor
//=============================
DCollectiveCommInfo::~DCollectiveCommInfo ()
{
    std::list<DCollectiveWave*>::iterator i;
    std::map<int, DCollectiveWave*>::iterator j;

    for (i = myActiveWaves.begin(); i != myActiveWaves.end(); i++)
    {
        if (*i)
            delete (*i);
    }

    for (i = myTimedOutWaves.begin(); i != myTimedOutWaves.end(); i++)
    {
        if (*i)
            delete (*i);
    }

    for (j = myWaitingForIntraWaves.begin(); j != myWaitingForIntraWaves.end(); j++)
    {
        if (j->second)
            delete (j->second);
    }

    myActiveWaves.clear();
    myTimedOutWaves.clear();
    myWaitingForIntraWaves.clear();

    if (myComm)
        myComm->erase();
    myComm = NULL;
}

//=============================
// getComm
//=============================
I_Comm* DCollectiveCommInfo::getComm (void)
{
    return myComm;
}

//=============================
// addNewOp
//=============================
GTI_ANALYSIS_RETURN DCollectiveCommInfo::addNewOp (
    I_DCollectiveListener* listener,
    I_ChannelId* cId,
    std::list<I_ChannelId*> *outFinishedChannels,
    DCollectiveOp* newOp,
    bool runIntraChecks,
    bool ancestorRunsIntraChecks,
    bool forceTimeout)
{
    DCollectiveWave* wave = NULL;
    GTI_ANALYSIS_RETURN ret;

    //1) Search in the timed out waves
    std::list<DCollectiveWave*>::iterator iter;
    std::list<DCollectiveWave*> *fromList = NULL;
    for (iter = myTimedOutWaves.begin(); iter != myTimedOutWaves.end(); iter++)
    {
        if ((*iter)->belongsToWave (cId, newOp))
        {
            wave = *iter;
            fromList = &myTimedOutWaves;
            break;
        }
    }

    //2) Search in the active waves
    if (!wave)
    {
        for (iter = myActiveWaves.begin(); iter != myActiveWaves.end(); iter++)
        {
            if ((*iter)->belongsToWave (cId, newOp))
            {
                wave = *iter;
                fromList = &myActiveWaves;
                break;
            }
        }
    }

    //3) Create a new active wave, if necessary
    if (!wave)
    {
        wave = new DCollectiveWave (newOp->getCollId(), myNumReachableRanks, myNextWaveNumber++);
        if (forceTimeout || !myAggregate)
        {
            wave->timeout();
            myTimedOutWaves.push_back(wave);
            iter = myTimedOutWaves.end();
            iter--;
            fromList = &myTimedOutWaves;
        }
        else
        {
            myActiveWaves.push_back(wave);
            iter = myActiveWaves.end();
            iter--;
            fromList = &myActiveWaves;
        }

        //Do we have type match information for this wave?
        std::map<int, std::list<DCollectiveTypeMatchInfo*> >::iterator unexpectedPos = myUnexpectedTypeMatchInfos.find (wave->getWaveNumber());

        if (unexpectedPos != myUnexpectedTypeMatchInfos.end())
        {
            //Add all unexpected type match infos to the wave
            wave->addNewTypeMatchInfo(unexpectedPos->second);

            //Remove the unexpected type match infos
            myUnexpectedTypeMatchInfos.erase(unexpectedPos);
        }
    }

    //4alpha) Let our listener know of the new op, update its timestamp accordingly
    if (    listener &&
            newOp->isFirstOpOfWave())
    {
        MustLTimeStamp ts = listener->newCollectiveOp (
                newOp->getPId(),
                newOp->getLId(),
                newOp->getCommCopy(),
                newOp->getCollId(),
                wave->getWaveNumber(),
                newOp->hasRequest(),
                newOp->hasRequest() ? newOp->getRequest() : 0);

        newOp->setLTimeStamp (ts);
    }

    //4) Apply to the wave
    ret = wave->addNewOp(listener, cId, outFinishedChannels, newOp, runIntraChecks, ancestorRunsIntraChecks, myStride, myOffset);

    if (listener && ret == GTI_ANALYSIS_FAILURE)
    {
        /**
         * @todo we should tell the listener if we had detected an error, such that he can switch his analysis of
         * (What about parallelism, how do we forward this decission)
         */
    }

    //5) Did we have an error, if so abort all active waves
    if (ret == GTI_ANALYSIS_FAILURE)
    {
        for (iter = myActiveWaves.begin(); iter != myActiveWaves.end(); iter++)
            (*iter)->abort (outFinishedChannels);
    }

    //6) Was the wave completed?
    if (ret != GTI_ANALYSIS_FAILURE && wave->isCompleted())
    {
        //Remove from current list
        fromList->erase(iter);

        if (wave->waitsForIntraTypeMatchInfos())
        {
            myWaitingForIntraWaves[wave->getWaveNumber()] = wave;
        }
        else
        {
            delete (wave);
        }
    }

    return ret;
}

//=============================
// printAsDot
//=============================
std::ostream& DCollectiveCommInfo::printAsDot (std::ostream& out, std::string nodeNamePrefix, I_LocationAnalysis *locations)
{
    int i =0;
    std::list<DCollectiveWave*>::iterator iter;
    std::map<int, DCollectiveWave*>::iterator iter2;

    out
        << "subgraph cluster" << nodeNamePrefix << "_" << ++i << std::endl
        << "{" << std::endl
        << "color=black;" << std::endl
        << "style=rounded;" << std::endl;

    int firstWorldInComm = -1;
    if (myComm && myComm->getGroup())
        myComm->getGroup()->translate(0, &firstWorldInComm);

    if (myComm->isPredefined())
        out << "label=\"" << myComm->getPredefinedName() << "\";" << std::endl;
    else
        out << "label=\"" << locations->getInfoForId(myComm->getCreationPId(),myComm->getCreationLId()).callName << " " << firstWorldInComm << "\";" << std::endl;

    //ACTIVE
    out
        << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
        << "{" << std::endl
        << "color=black;" << std::endl
        << "style=rounded;" << std::endl
        << "label=\"Active\";" << std::endl;

    for (iter = myActiveWaves.begin(); iter != myActiveWaves.end(); iter++)
    {
        if (!*iter)
            continue;
        std::stringstream stream;
        stream << nodeNamePrefix << "_" << ++i;
        (*iter)->printAsDot (out, stream.str(), locations);
    }
    if (myActiveWaves.empty())
        out << nodeNamePrefix << "_" <<  ++i << "_empty [label=\"EMPTY\", shape=box];" << std::endl;

    out << "}" << std::endl;

    //TIMEDOUT
    out
    << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
    << "{" << std::endl
    << "color=black;" << std::endl
    << "style=rounded;" << std::endl
    << "label=\"Timedout\";" << std::endl;

    for (iter = myTimedOutWaves.begin(); iter != myTimedOutWaves.end(); iter++)
    {
        if (!*iter)
            continue;
        std::stringstream stream;
        stream << nodeNamePrefix << "_" << ++i;
        (*iter)->printAsDot (out, stream.str(), locations);
    }
    if (myTimedOutWaves.empty())
        out << nodeNamePrefix << "_" <<  ++i << "_empty [label=\"EMPTY\", shape=box];" << std::endl;

    out << "}" << std::endl;

    //INTRA
    out
    << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
    << "{" << std::endl
    << "color=black;" << std::endl
    << "style=rounded;" << std::endl
    << "label=\"WaitingForIntra\";" << std::endl;

    for (iter2 = myWaitingForIntraWaves.begin(); iter2 != myWaitingForIntraWaves.end(); iter2++)
    {
        if (!iter2->second)
            continue;
        std::stringstream stream;
        stream << nodeNamePrefix << "_" << ++i;
        iter2->second->printAsDot (out, stream.str(), locations);
    }
    if (myWaitingForIntraWaves.empty())
        out << nodeNamePrefix << "_" <<  ++i << "_empty [label=\"EMPTY\", shape=box];" << std::endl;

    out << "}" << std::endl;

    //Type-Match infos
    out
    << "subgraph cluster" << nodeNamePrefix << "_"  << ++i << std::endl
    << "{" << std::endl
    << "color=black;" << std::endl
    << "style=rounded;" << std::endl
    << "label=\"TypeMatchInfos\";" << std::endl;

    std::map<int, std::list<DCollectiveTypeMatchInfo*> >::iterator matchInfoIter;
    int lastWaveNumber = -1;
    for (matchInfoIter = myUnexpectedTypeMatchInfos.begin(); matchInfoIter != myUnexpectedTypeMatchInfos.end(); matchInfoIter++)
    {
        int waveNumber = matchInfoIter->first;
        std::stringstream stream;

        stream << nodeNamePrefix << "_" << i << "_TypeMatchInfo_" << waveNumber << "[label=\"{" << waveNumber << ":";

        std::list<DCollectiveTypeMatchInfo*>::iterator infoIter;
        for (infoIter = matchInfoIter->second.begin(); infoIter != matchInfoIter->second.end(); infoIter++)
        {
            DCollectiveTypeMatchInfo *info = *infoIter;

            if (!info) continue;

            stream << "|{";
            for (int i = 0; i < info->getNumCounts(); i++)
            {
                if (i != 0)
                    stream << "|";

                stream << info->getFirstRank()+i << ":" << info->getCounts()[i];
            }
            stream << "}";
        }

        stream << "}\", shape=record]";

        out << stream.str() << std::endl;

        if (lastWaveNumber >= 0)
            out << nodeNamePrefix << "_" << i << "_TypeMatchInfo_" << lastWaveNumber << "->"
                  << nodeNamePrefix << "_" << i << "_TypeMatchInfo_" << waveNumber << ";" << std::endl;
    }
    if (myUnexpectedTypeMatchInfos.empty())
        out << nodeNamePrefix << "_" <<  ++i << "_empty [label=\"EMPTY\", shape=box];" << std::endl;

    out << "}" << std::endl;

    out
        << "}" << std::endl;

    return out;
}

//=============================
// timeout
//=============================
void DCollectiveCommInfo::timeout (void)
{
    std::list<DCollectiveWave*>::iterator iter;

    for (iter = myActiveWaves.begin(); iter != myActiveWaves.end(); iter++)
    {
        if (!*iter)
            continue;

        (*iter)->timeout ();
    }

    myTimedOutWaves.splice(myTimedOutWaves.end(), myActiveWaves);
}

//=============================
// hasUncompletedWaves
//=============================
bool DCollectiveCommInfo::hasUncompletedWaves (void)
{
    if (!myActiveWaves.empty() || !myTimedOutWaves.empty() || !myWaitingForIntraWaves.empty())
        return true;

    return false;
}

//=============================
// addNewTypeMatchInfo
//=============================
void DCollectiveCommInfo::addNewTypeMatchInfo (DCollectiveTypeMatchInfo *matchInfo)
{
    std::list<DCollectiveWave*>::iterator iter;

    //== 1) Search in Waiting for intra waves
    std::map<int, DCollectiveWave*>::iterator pos;
    pos = myWaitingForIntraWaves.find (matchInfo->getWaveNumber());

    if (pos != myWaitingForIntraWaves.end())
    {
        pos->second->addNewTypeMatchInfo (matchInfo);

        //For waves in myWaitingForIntraWaves we might finally complete the wave
        if (!pos->second->waitsForIntraTypeMatchInfos())
        {
            delete (pos->second);
            myWaitingForIntraWaves.erase (pos);
        }

        return;
    }

    //== 2) Search in Timedout waves
    for (iter = myTimedOutWaves.begin(); iter != myTimedOutWaves.end(); iter++)
    {
        if ((*iter)->getWaveNumber() == matchInfo->getWaveNumber())
        {
            (*iter)->addNewTypeMatchInfo (matchInfo);
            return;
        }
    }

    //== 3) Search in active waves
    for (iter = myActiveWaves.begin(); iter != myActiveWaves.end(); iter++)
    {
        if ((*iter)->getWaveNumber() == matchInfo->getWaveNumber())
        {
            (*iter)->addNewTypeMatchInfo (matchInfo);
            return;
        }
    }

    //== 4) Store in unexpected type match infos
    myUnexpectedTypeMatchInfos[matchInfo->getWaveNumber()].push_back (matchInfo);
}

//=============================
// hasActiveWave
//=============================
bool DCollectiveCommInfo::hasActiveWave (void)
{
    return !myActiveWaves.empty();
}

//=============================
// disableAggregation
//=============================
void DCollectiveCommInfo::disableAggregation (void)
{
    myAggregate = false;
}

/*EOF*/
