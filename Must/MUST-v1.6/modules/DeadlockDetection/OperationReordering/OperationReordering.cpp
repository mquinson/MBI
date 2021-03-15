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
 * @file OperationReordering.cpp
 *       @see MUST::OperationReordering.
 *
 *  @date 25.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "GtiMacros.h"

#include "OperationReordering.h"

using namespace must;

mGET_INSTANCE_FUNCTION(OperationReordering)
mFREE_INSTANCE_FUNCTION(OperationReordering)
mPNMPI_REGISTRATIONPOINT_FUNCTION(OperationReordering)

//=============================
// Constructor
//=============================
OperationReordering::OperationReordering (const char* instanceName)
    : gti::ModuleBase<OperationReordering, I_OperationReordering> (instanceName),
      myRankBlocked (),
      myCheckpointRankBlocked(),
      myIsSuspended (false),
      myCheckpointIsSuspended (false),
      myQueues (),
      myCheckpointQueues (),
      myIsInProcessing (false),
      myCheckpointIsInProcessing (false),
      myNumOps (0),
      myCheckpointNumOps (0),
      myOpenNonEmptyQueueIndices (),
      myRankinNonEmptyQueueList (),
      myIndexInOpenNonEmptyIndices ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 1
    if (subModInstances.size() < NUM_MODS_REQUIRED)
    {
        std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
        assert (0);
    }
    if (subModInstances.size() > NUM_MODS_REQUIRED)
    {
        for (std::vector<I_Module*>::size_type i = NUM_MODS_REQUIRED; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    myFloodControl = (I_FloodControl*) subModInstances[0];
}

//=============================
// ~OperationReordering
//=============================
OperationReordering::~OperationReordering (void)
{
#ifdef MUST_DEBUG
    for (int i = 0; i < myQueues.size(); i++)
    {
        if (myQueues[i].size())
        {
            std::cout << "Warning: OperationReordering has " << myQueues[i].size() << " operation queued for rank " << i << " during its destruction:" << std::endl;

            std::list<I_Operation*>::iterator iter;
            int j = 1;
            while (!myQueues[i].empty())
            {
                I_Operation* op = myQueues[i].front();
                myQueues[i].pop_front();
                j++;

                std::cout << "- (" << j << "):";
                //We do not call print here as the operation may have inconsistent data due to the ongoing destruction
                //if (*iter) (*iter)->print (std::cout);
                std::cout << op << std::endl;
            }
        }
    }
#endif

    myQueues.clear ();
    myRankBlocked.clear();
    myRankinNonEmptyQueueList.clear ();
    myOpenNonEmptyQueueIndices.clear ();
    myIndexInOpenNonEmptyIndices.clear ();

    if (myFloodControl)
        destroySubModuleInstance ((I_Module*) myFloodControl);
    myFloodControl = NULL;
}

//=============================
// init
//=============================
GTI_ANALYSIS_RETURN OperationReordering::init (
        int worldSize)
{
    if (!myQueues.size())
    {
        myQueues.resize(worldSize);
        myRankBlocked.resize(worldSize, false);
        myRankinNonEmptyQueueList.resize(worldSize, false);
        myIndexInOpenNonEmptyIndices.resize(worldSize);
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isRankOpen
//=============================
bool OperationReordering::isRankOpen (int rank)
{
    if (rank >= myRankBlocked.size())
    {
        std::cerr << "ERROR: initialization failed, OperationReordering did not receive the init event." << std::endl;
        assert (0);
        return false;
    }

    if (myIsSuspended)
        return false;

    if (myRankBlocked[rank])
        return false;

    /*
     * Rank is also not open if there is at least one op
     * that is still queued for it!
     */
    return myQueues[rank].empty();
}

//=============================
// blockRank
//=============================
GTI_RETURN OperationReordering::blockRank (int rank)
{
    if (rank >= myRankBlocked.size())
    {
        std::cerr << "ERROR: initialization failed, OperationReordering did not receive the init event." << std::endl;
        assert (0);
        return GTI_ERROR;
    }

    //Remove from list of open ranks if it was there
    if (myRankinNonEmptyQueueList[rank])
    {
        std::list<int>::iterator iter = myIndexInOpenNonEmptyIndices[rank];
        myOpenNonEmptyQueueIndices.erase(iter);
        myRankinNonEmptyQueueList[rank] = false;
    }

    myRankBlocked[rank] = true;
    return GTI_SUCCESS;
}

//=============================
// resumeRank
//=============================
GTI_RETURN OperationReordering::resumeRank (int rank)
{
    if (rank >= myRankBlocked.size())
    {
        std::cerr << "ERROR: initialization failed, OperationReordering did not receive the init event." << std::endl;
        assert (0);
        return GTI_ERROR;
    }

    myRankBlocked[rank] = false;

    //Add to open ranks if it has queued ops and is not already there
    if (!myQueues[rank].empty() && !myRankinNonEmptyQueueList[rank])
    {
        myRankinNonEmptyQueueList[rank] = true;
        myOpenNonEmptyQueueIndices.push_front(rank);
        myIndexInOpenNonEmptyIndices[rank] = myOpenNonEmptyQueueIndices.begin();
    }

    return processQueues ();
}

//=============================
// enqueueOp
//=============================
GTI_RETURN OperationReordering::enqueueOp (int rank, I_Operation* op)
{
    //Check rank
    if (rank >= myRankBlocked.size())
    {
        std::cerr << "ERROR: initialization failed, OperationReordering did not receive the init event." << std::endl;
        assert (0);
        return GTI_ERROR;
    }

    //Mark as bad op
    myFloodControl->markCurrentRecordBad();

    //Enqueue
    myQueues[rank].push_back(op);
    myNumOps++;

    //Do we need to add the rank into the non-empty queue list ?
    if (!myRankBlocked[rank] && !myRankinNonEmptyQueueList[rank])
    {
        myRankinNonEmptyQueueList[rank] = true;
        myOpenNonEmptyQueueIndices.push_front(rank);
        myIndexInOpenNonEmptyIndices[rank] = myOpenNonEmptyQueueIndices.begin();
    }

    //TODO: find out why we need this!
    if (!myOpenNonEmptyQueueIndices.empty())
        return processQueues();

    return GTI_SUCCESS;
}

//=============================
// suspend
//=============================
GTI_RETURN OperationReordering::suspend (void)
{
    if (!myRankBlocked.size())
    {
        std::cerr << "ERROR: initialization failed, OperationReordering did not receive the init event." << std::endl;
        assert (0);
        return GTI_ERROR;
    }

    myIsSuspended = true;
    return GTI_SUCCESS;
}

//=============================
// isSuspended
//=============================
bool OperationReordering::isSuspended (void)
{
    return myIsSuspended;
}

//=============================
// removeSuspension
//=============================
GTI_RETURN OperationReordering::removeSuspension (void)
{
    if (!myRankBlocked.size())
    {
        std::cerr << "ERROR: initialization failed, OperationReordering did not receive the init event." << std::endl;
        assert (0);
        return GTI_ERROR;
    }

    myIsSuspended = false;
    return processQueues();
}

//=============================
// processQueues
//=============================
GTI_RETURN OperationReordering::processQueues (void)
{
    //Protection against recursive calling of processQueues, not thread safe!
    if (myIsInProcessing)
        return GTI_SUCCESS;

    myIsInProcessing = true;

    while (!myIsSuspended && !myOpenNonEmptyQueueIndices.empty())
    {
        int rank = myOpenNonEmptyQueueIndices.front();

        //Is there still something to process ?
        if (myRankBlocked[rank] || myQueues[rank].empty())
        {
            std::list<int>::iterator iter = myIndexInOpenNonEmptyIndices[rank];
            myOpenNonEmptyQueueIndices.erase(iter);
            myRankinNonEmptyQueueList[rank] = false;
            continue;
        }

        //Do the processing
        I_Operation* op = myQueues[rank].front();

        PROCESSING_RETURN ret = PROCESSING_SUCCESS;
        if (op)
            ret = op->process(rank);

        //Evaluate the processing return value
        if (ret == PROCESSING_ERROR) return GTI_ERROR;

        if (ret != PROCESSING_REEXECUTE)
        {
            myQueues[rank].pop_front();
            myNumOps--;

            if (myQueues[rank].empty() && myRankinNonEmptyQueueList[rank])
            {
                std::list<int>::iterator iter = myIndexInOpenNonEmptyIndices[rank];
                myOpenNonEmptyQueueIndices.erase(iter);
                myRankinNonEmptyQueueList[rank] = false;
            }
        }
    }

    myIsInProcessing = false;

    return GTI_SUCCESS;
}

//=============================
// getTotalQueueSize
//=============================
int OperationReordering::getTotalQueueSize (void)
{
    return myNumOps;
}

//=============================
// getTotalQueueSize
//=============================
void OperationReordering::clearQ (RankQueues *queue)
{
    for (RankQueues::size_type i = 0; i < queue->size(); i++)
    {
        std::deque<I_Operation*>::iterator iter;
        for (iter = (*queue)[i].begin(); iter != (*queue)[i].end(); iter++)
        {
            if (*iter) delete (*iter);
        }
        (*queue)[i].clear();
    }
    queue->clear();
}

//=============================
// checkpoint
//=============================
void OperationReordering::checkpoint (void)
{
    //==1) myRankBlocked
    if (myCheckpointRankBlocked.size() != myRankBlocked.size())
        myCheckpointRankBlocked.resize (myRankBlocked.size());

    myCheckpointRankBlocked = myRankBlocked;

    //==2) myIsSuspended
    myCheckpointIsSuspended = myIsSuspended;

    //==3) myQueues
    clearQ (&myCheckpointQueues);
    if (myQueues.size() != myCheckpointQueues.size())
        myCheckpointQueues.resize(myQueues.size());
    for (RankQueues::size_type i = 0; i < myQueues.size(); i++)
    {
        std::deque<I_Operation*>::iterator opIter;
        for (opIter = myQueues[i].begin(); opIter != myQueues[i].end(); opIter++)
        {
            myCheckpointQueues[i].push_back ((*opIter)->copyQueuedOp());
        }
    }

    //==4) myIsInProcessing
    myCheckpointIsInProcessing = myIsInProcessing;

    //==5) myNumOps
    myCheckpointNumOps = myNumOps;
}

//=============================
// rollback
//=============================
void OperationReordering::rollback (void)
{
    //==1) myRankBlocked
    myRankBlocked = myCheckpointRankBlocked;

    //==2) myIsSuspended
    myIsSuspended = myCheckpointIsSuspended;

    //==3) myQueues
    clearQ (&myQueues); //This must be a delete and clear
    if (myQueues.size() != myCheckpointQueues.size())
        myQueues.resize(myCheckpointQueues.size());

    myOpenNonEmptyQueueIndices.clear();

    for (RankQueues::size_type i = 0; i < myQueues.size(); i++)
    {
        myQueues[i] = myCheckpointQueues[i];

        //Take care of performance optimization vars
        if (!myQueues[i].empty() && !myRankBlocked[i])
        {
            myRankinNonEmptyQueueList[i] = true;
            myOpenNonEmptyQueueIndices.push_front(i);
            myIndexInOpenNonEmptyIndices[i] = myOpenNonEmptyQueueIndices.begin();
        }
        else
        {
            myRankinNonEmptyQueueList[i] = false;
        }
    }
    myCheckpointQueues.clear(); //This must be a clear

    //==4) myIsInProcessing
    myIsInProcessing = myCheckpointIsInProcessing;

    //==5) myNumOps
    myNumOps = myCheckpointNumOps;
}

/*EOF*/
