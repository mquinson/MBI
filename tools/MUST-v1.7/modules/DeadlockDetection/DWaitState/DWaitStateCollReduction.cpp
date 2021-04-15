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
 * @file DWaitStateCollReduction.cpp
 *       @see DWaitStateCollReduction.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "DWaitStateCollReduction.h"

#include <assert.h>

using namespace must;

mGET_INSTANCE_FUNCTION(DWaitStateCollReduction)
mFREE_INSTANCE_FUNCTION(DWaitStateCollReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DWaitStateCollReduction)

//=============================
// CommInfo -- Constructor
//=============================
DWaitStateCollReduction::CommInfo::CommInfo (void)
 : isIntercomm (false),
   contextId (0),
   localSize (0),
   remoteSize (0),
   numConnected (0),
   activeRequests (),
   tempCId (NULL)
{

}

//=============================
// CommInfo -- Copy Constructor
//=============================
DWaitStateCollReduction::CommInfo::CommInfo (const CommInfo& other)
 : isIntercomm (other.isIntercomm),
   contextId (other.contextId),
   localSize (other.localSize),
   remoteSize (other.remoteSize),
   numConnected (other.numConnected),
   activeRequests (),
   tempCId (NULL)
{
    if (other.tempCId)
        tempCId = other.tempCId->copy();

    //Note that we don't copy activeRequests, that shouldn't be necessary
}

//=============================
// CommInfo -- Destructor
//=============================
DWaitStateCollReduction::CommInfo::~CommInfo (void)
{
    std::list<std::pair<int, CompletionTree*> >::iterator iter;
    for (iter = activeRequests.begin(); iter != activeRequests.end(); iter++)
    {
        if (iter->second)
            delete iter->second;
    }
    activeRequests.clear();

    if (tempCId)
        delete tempCId;
}

//=============================
// Constructor
//=============================
DWaitStateCollReduction::DWaitStateCollReduction (const char* instanceName)
    : gti::ModuleBase<DWaitStateCollReduction, I_DWaitStateCollReduction> (instanceName),
      myInfos (),
      myUnexpectedRequests (),
      myInUnexepctedTest (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBS 3
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
    myCommTrack = (I_CommTrack*) subModInstances[1];
    myCollMatch = (I_DCollectiveMatchReduction*) subModInstances[2];

    //Initialize module data
    ModuleBase<DWaitStateCollReduction, I_DWaitStateCollReduction>::getWrapperFunction("generateCollectiveActiveRequest", (GTI_Fct_t*)&myFForward);
    assert (myFForward); //Must be there, otherwise we have a mapping error for this tool configuration

    myCollMatch->registerCommListener(this);
}

//=============================
// Destructor
//=============================
DWaitStateCollReduction::~DWaitStateCollReduction ()
{
    //Free data
    myInfos.clear();

    /*Free module data*/
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myCommTrack)
        destroySubModuleInstance ((I_Module*) myCommTrack);
    myCommTrack = NULL;

    if (myCollMatch)
        destroySubModuleInstance ((I_Module*) myCollMatch);
    myCollMatch = NULL;
}

//=============================
// newCommInColl
//=============================
void DWaitStateCollReduction::newCommInColl (
        MustParallelId pId,
        I_CommPersistent* comm)
{
    int isIntercomm = comm->isIntercomm();
    int localSize = 0, remoteSize = 0;
    int firstOfW;
    unsigned long long contextId = comm->getContextId();

    if (comm->getGroup())
        localSize = comm->getGroup()->getSize();

    if (comm->getRemoteGroup())
        remoteSize = comm->getRemoteGroup()->getSize();

    if (comm->getGroup() && !comm->getRemoteGroup())
    {
        comm->getGroup()->translate(0, &firstOfW);
        contextId += firstOfW;
    }

    //Do we already have this comm?
    std::list<CommInfo>::iterator iter;
    for (iter = myInfos.begin(); iter != myInfos.end(); iter++)
    {
        if (compare (isIntercomm, contextId, localSize, remoteSize, iter->isIntercomm, iter->contextId, iter->localSize, iter->remoteSize))
            return;
    }

    //Add as new comm
    if (iter == myInfos.end())
    {
        CommInfo requestInfo;
        requestInfo.isIntercomm = isIntercomm;
        requestInfo.contextId = contextId;
        requestInfo.localSize = localSize;
        requestInfo.remoteSize = remoteSize;
        requestInfo.numConnected = 0;

        //Determine how many ranks are we expecting for this particular comm
        int rank = myPIdMod->getInfoForId(pId).rank,
                begin, end;

        ModuleBase<DWaitStateCollReduction, I_DWaitStateCollReduction>::getReachableRanks (&begin, &end, rank);

        /*
         * @todo Think about inter-communicators here!
         */
        for (int i = begin; i <= end; i++)
        {
            if (comm->getGroup()->containsWorldRank(i,NULL))
                requestInfo.numConnected++;
        }

        //Add comm to info list
        myInfos.push_back(requestInfo);
    }

    //Check whether the new comm info allows us to process a bit more
    myInUnexepctedTest = true;
    GTI_ANALYSIS_RETURN ret = GTI_ANALYSIS_SUCCESS;
    while (ret == GTI_ANALYSIS_SUCCESS && !myUnexpectedRequests.empty())
    {
        CommInfo& info = myUnexpectedRequests.front();
                                                        // TODO: temp fix for trying out. Needs to be fixed in CommInfo!
        ret = request (info.isIntercomm, info.contextId, MUST_COLL_UNKNOWN, info.localSize, info.remoteSize, info.numConnected, info.tempCId, NULL);

        if (ret != GTI_ANALYSIS_FAILURE)
        {
            myUnexpectedRequests.pop_front();
        }
    }

    myInUnexepctedTest = false;
}

//=============================
// request
//=============================
GTI_ANALYSIS_RETURN DWaitStateCollReduction::request (
        int isIntercomm,
        unsigned long long contextId,
        int collCommType,
        int localGroupSize,
        int remoteGroupSize,
        int numTasks,
        I_ChannelId *cId,
        std::list<I_ChannelId*> *outFinishedChannels)
{
    //Is it already a complete request (central case should lead to this where DWaitState, DWaitStateCollReduction and DWaitStateCollMgr all run on the same TBON node)
    if (cId == NULL)
        return GTI_ANALYSIS_IRREDUCIBLE;

    //Search for a fitting info in our infos
    std::list<CommInfo>::iterator iter;
    for (iter = myInfos.begin(); iter != myInfos.end(); iter++)
    {
        if (compare (isIntercomm, contextId, localGroupSize, remoteGroupSize, iter->isIntercomm, iter->contextId, iter->localSize, iter->remoteSize))
        {
            break;
        }
    }

    //Sometimes comm infos arrive late, so lets keep that for later
    if (iter == myInfos.end())
    {
        //If we explore whether we can process, tell the loop that we can't!
        if (myInUnexepctedTest)
            return GTI_ANALYSIS_FAILURE;

        //Otherwise add to unexpected
        myUnexpectedRequests.push_back(CommInfo ());
        std::list<CommInfo>::reverse_iterator pos = myUnexpectedRequests.rbegin();

        pos->isIntercomm = isIntercomm;
        pos->contextId = contextId;
        pos->localSize = localGroupSize;
        pos->remoteSize = remoteGroupSize;
        pos->numConnected = numTasks;
        pos->tempCId = cId->copy();

        return GTI_ANALYSIS_SUCCESS;
    }

    //Apply to the request info, we search for the right completion tree in our list
    std::list<std::pair<int, CompletionTree*> >::iterator waveIter;
    bool found = false;
    for (waveIter = iter->activeRequests.begin(); waveIter != iter->activeRequests.end(); waveIter++)
    {
        assert (waveIter->second);

        //Skip if we don't belong to that wave
        if (waveIter->second->wasCompleted (cId))
            continue;

        //We belong into this existing wave
        found = true;
        waveIter->second->addCompletion(cId);
        waveIter->first = waveIter->first + numTasks;
        assert (iter->numConnected >= waveIter->first); //Must never exceed numConnected

        //Completed request (on this TBON node)?
        if (iter->numConnected == waveIter->first)
        {
            delete waveIter->second;
            waveIter->second = NULL;
            iter->activeRequests.erase(waveIter);

            if (myFForward)
            {
                (*myFForward) (
                        isIntercomm,
                        contextId,
                        collCommType,
                        localGroupSize,
                        remoteGroupSize,
                        iter->numConnected );
            }
        }

        break;
    }

    //Did we find a fitting wave? If not we add a new one!
    if (!found)
    {
        if (numTasks != iter->numConnected)
        {
            //This needs multiple events to complete, so we create a tree
            CompletionTree *newTree = new CompletionTree (cId->getNumUsedSubIds()-1, cId->getSubIdNumChannels(cId->getNumUsedSubIds()-1));
            newTree->addCompletion(cId);
            iter->activeRequests.push_back(std::make_pair(numTasks, newTree));
        }
        else
        {
            //This is already completed, so don't bother to create a tree
            /**
             * @note this handling here (and also above) lets acknowledges overtake
             *       one another, I think this should only happen across distinct communicators,
             *       which is fine, but it might be worth to think a bit more about this.
             */
            if (myFForward)
            {
                (*myFForward) (
                        isIntercomm,
                        contextId,
                        collCommType,
                        localGroupSize,
                        remoteGroupSize,
                        iter->numConnected );
            }
        }
    }

    //Always we return success (i.e. we remove the incoming event!)
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// timeout
//=============================
void DWaitStateCollReduction::timeout (void)
{
    //Nothing to do we do not care about timeouts
}

//=============================
// compare
//=============================
bool DWaitStateCollReduction::compare (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize,
                int isIntercomm2,
                unsigned long long contextId2,
                int localGroupSize2,
                int remoteGroupSize2)
{
    if (isIntercomm != isIntercomm2)
        return false;

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

/*EOF*/
