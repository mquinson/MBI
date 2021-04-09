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
 * @file BlockingState.cpp
 *       @see MUST::BlockingState.
 *
 *  @date 08.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "MustDefines.h"
#include "Wfg.h"
#include "MatchExplorer.h"
#include "GtiApi.h"
#include "mustConfig.h"

#include "BlockingState.h"

#include <assert.h>
#include <sstream>
#include <fstream>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef GTI_VT
#include <vt_user.h>
#include <dlfcn.h>

void* theHandle = 0;

typedef void (*startP) (const char* name, const char* file, int lno);
typedef void (*endP) (const char* name);
typedef void (*closeP) (void);

void VT_User_start__(const char* name, const char* file, int lno)
{
    if (!theHandle)
        theHandle = dlopen ("libvt.so", RTLD_LAZY);

    startP fp = (startP) dlsym (theHandle, "VT_User_start__");
    fp (name, file, lno);
}

void VT_User_end__(const char* name)
{
    endP fp = (endP) dlsym (theHandle, "VT_User_end__");
    fp (name);
}
#endif /*GTI_VT*/


using namespace must;

mGET_INSTANCE_FUNCTION(BlockingState)
mFREE_INSTANCE_FUNCTION(BlockingState)
mPNMPI_REGISTRATIONPOINT_FUNCTION(BlockingState)

//=============================
// HeadInfo -- Constructor
//=============================
HeadInfo::HeadInfo ()
: matchedReqs (),
  unexpectedCompletions (),
  hasCollCompletion (false),
  hasSendCompletion (false),
  hasReceiveCompletion (false),
  primary (NULL),
  secondary (NULL)
#ifdef MUST_DEBUG
  ,specialTime (-1)
#endif
{
    //Nothing to do
}

//=============================
// HeadInfo -- Destructor
//=============================
HeadInfo::~HeadInfo ()
{
    if (primary)
        delete primary;
    primary = NULL;

    if (secondary)
        delete secondary;
    secondary = NULL;
}

//=============================
// Constructor
//=============================
BlockingState::BlockingState (const char* instanceName)
: gti::ModuleBase<BlockingState, I_BlockingState> (instanceName),
  myHeads (),
  myCheckpointHeads (),
  myFinCompletion (NULL),
  myCheckpointFinCompletion (NULL)
#ifdef MUST_DEBUG
  ,myHistory (),
  myCheckpointHistory (),
  myTimeStep (0),
  myCheckpointTimeStep (0)
#endif
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 9
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

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myConsts = (I_BaseConstants*) subModInstances[1];
    myLogger = (I_CreateMessage*) subModInstances[2];
    myCTrack = (I_CommTrack*) subModInstances[3];
    myOrder = (I_OperationReordering*) subModInstances[4];
    myP2PMatch = (I_P2PMatch*) subModInstances[5];
    myCollMatch = (I_CollectiveMatch*) subModInstances[6];
    myLIdMod = (I_LocationAnalysis*) subModInstances[7];
    myRTrack = (I_RequestTrack*) subModInstances[8];

    //Make us listen for P2P/Coll matches
    myP2PMatch->registerListener(this);
    myCollMatch->registerListener(this);
}

//=============================
// Destructor
//=============================
BlockingState::~BlockingState ()
{
#ifdef GTI_VT
   if (!theHandle)
      theHandle = dlopen ("libvt.so", RTLD_LAZY);

   closeP fp = (closeP) dlsym (theHandle, "vt_close");
   fp ();
#endif

#ifdef MUST_DEBUG
    //Print history
    printHistoryAsDot ();
    myHistory.clear ();
    myCheckpointHistory.clear();
#endif

    //Debug warnings if remaining items
    for (HeadStates::size_type i = 0; i < myHeads.size(); i++)
    {
        HeadInfo* head = &(myHeads[i]);

        if (head->primary)
        {
#ifdef MUST_DEBUG
            std::cout << "Warning: head " << i << " had an outstanding primary!" << std::endl;
#endif
        }

        if (head->secondary)
        {
#ifdef MUST_DEBUG
            std::cout << "Warning: head " << i << " had an outstanding secondary!" << std::endl;
#endif
        }

        if (head->hasCollCompletion || head->hasReceiveCompletion || head->hasSendCompletion || head->matchedReqs.size() || head->unexpectedCompletions.size())
        {
#ifdef MUST_DEBUG
            std::cout
                << "Warning: head " << i << " had available completions, requests or unexpected requests!" << std::endl
                << "Info: hasCollCompletion=" << head->hasCollCompletion << " hasReceiveCompletion=" << head->hasReceiveCompletion << " hasSendCompletion=" << head->hasSendCompletion << " matchedReqs={";
            std::list<MustRequestType>::iterator rIter;
            for (rIter = head->matchedReqs.begin(); rIter != head->matchedReqs.end(); rIter++)
            {
                std::cout << *rIter << ", ";
            }
            std::cout << "} unexpectedCompletions={";
            for (rIter = head->unexpectedCompletions.begin(); rIter != head->unexpectedCompletions.end(); rIter++)
            {
                std::cout << *rIter << ", ";
            }
            std::cout << "}" << std::endl;
#endif
        }
    }

    //Clear the heads
    clearHeads (&myHeads);
    clearHeads (&myCheckpointHeads);

    //Delete the completion
    if (myFinCompletion) delete (myFinCompletion);
    if (myCheckpointFinCompletion) delete (myCheckpointFinCompletion);
    myFinCompletion = myCheckpointFinCompletion = NULL;

    //Free sub modules
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myConsts)
        destroySubModuleInstance ((I_Module*) myConsts);
    myConsts = NULL;

    if (myLogger)
        destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myCTrack)
        destroySubModuleInstance ((I_Module*) myCTrack);
    myCTrack = NULL;

    if (myOrder)
        destroySubModuleInstance ((I_Module*) myOrder);
    myOrder = NULL;

    if (myP2PMatch)
        destroySubModuleInstance ((I_Module*) myP2PMatch);
    myP2PMatch = NULL;

    if (myCollMatch)
        destroySubModuleInstance ((I_Module*) myCollMatch);
    myCollMatch = NULL;

    if (myLIdMod)
        destroySubModuleInstance ((I_Module*) myLIdMod);
    myLIdMod = NULL;

    if (myRTrack)
        destroySubModuleInstance ((I_Module*) myRTrack);
    myRTrack = NULL;
}

//=============================
// CollAll
//=============================
GTI_ANALYSIS_RETURN BlockingState::CollAll (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustCommType comm,
        int isSend,
        int numTasks // counter for event aggregation
)
{
    //Did we initialize the heads ?
    initHeads (pId);

    //We discard some operations here (if some coll spawns a recv and a send part we only care about one
    //of the two parts), the important rule is:
    // WE ONLY HANLD THE SECOND PART, WHICH IS THE RECEIVE, IF IT HAS TWO PARTS!
    if (isSend)
    {
        /*All other transfering ops as the above and and besides MPI_Bcast have both a send and a receive, we discard the send there*/
        if (coll == MUST_COLL_SCATTER ||
            coll == MUST_COLL_SCATTERV ||
            coll == MUST_COLL_ALLGATHER ||
            coll == MUST_COLL_ALLGATHERV ||
            coll == MUST_COLL_ALLTOALL ||
            coll == MUST_COLL_ALLTOALLV ||
            coll == MUST_COLL_ALLTOALLW ||
            coll == MUST_COLL_ALLREDUCE ||
            coll == MUST_COLL_REDUCE_SCATTER ||
            coll == MUST_COLL_REDUCE_SCATTER_BLOCK ||
            coll == MUST_COLL_SCAN ||
            coll == MUST_COLL_EXSCAN)
            return GTI_ANALYSIS_SUCCESS;
    }

    //Get a persistent info for the comm
    I_CommPersistent *cInfo = myCTrack->getPersistentComm(pId, comm);
    if (!cInfo)
        return GTI_ANALYSIS_SUCCESS;
    if (cInfo->isNull ())
    {
        cInfo->erase ();
        return GTI_ANALYSIS_SUCCESS;
    }

    //For what remains we create a op
    BlockingColl *op = new BlockingColl (this, pId, lId, (MustCollCommType) coll, cInfo);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// CollRoot
//=============================
GTI_ANALYSIS_RETURN BlockingState::CollRoot (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustCommType comm,
        int isSend,
        int root,
        int numTasks // counter for event aggregation
)
{
    //Did we initialize the heads ?
    initHeads (pId);

    //Get a persistent info for the comm
    I_CommPersistent *cInfo= myCTrack->getPersistentComm(pId, comm);
    if (!cInfo)
        return GTI_ANALYSIS_SUCCESS;
    if (cInfo->isNull ())
    {
        cInfo->erase ();
        return GTI_ANALYSIS_SUCCESS;
    }

    //We need to discard the send parts of the root task for MPI_Gather, MPI_Gatherv, MPI_Reduce
    //(Such that we only block the rank when we receive the receive part on the root)
    int worldroot = root; /**< We originally translated the given root with the process group of the given communicator, but the root that arrives here is already translated to MPI_COMM_WORLD!*/
    
    if (isSend && worldroot == myPIdMod->getInfoForId(pId).rank &&
        (coll == MUST_COLL_GATHER || coll == MUST_COLL_GATHERV || coll == MUST_COLL_REDUCE))
    {
        cInfo->erase();
        return GTI_ANALYSIS_SUCCESS;
    }

    //Create and handle the operation
    BlockingColl *op = new BlockingColl (this, pId, lId, (MustCollCommType) coll, cInfo);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// send
//=============================
GTI_ANALYSIS_RETURN BlockingState::send (
        MustParallelId pId,
        MustLocationId lId,
        int dest)
{
    //Skip if MPI_PROC_NULL
    if (dest == myConsts->getProcNull())
        return GTI_ANALYSIS_SUCCESS;

    //Did we initialize the heads ?
    initHeads (pId);

    BlockingP2P *op = new BlockingP2P (this, pId, lId, true);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// send
//=============================
GTI_ANALYSIS_RETURN BlockingState::srsend (
        MustParallelId pId,
        MustLocationId lId,
        int dest)
{
    //Skip if MPI_PROC_NULL
    if (dest == myConsts->getProcNull())
        return GTI_ANALYSIS_SUCCESS;

    //Did we initialize the heads ?
    initHeads (pId);

    BlockingP2P *op = new BlockingP2P (this, pId, lId, true, true);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// receive
//=============================
GTI_ANALYSIS_RETURN BlockingState::receive (
        MustParallelId pId,
        MustLocationId lId,
        int source)
{
    //Did we initialize the heads ?
    initHeads (pId);

    //Skip if MPI_PROC_NULL
    if (source == myConsts->getProcNull())
    {
        //Was this the recv part of an ongoing sendrecv ?
        /*
         * This is somewhat ugly ...
         * We create an already matched recv op in that case!
         */
        HeadInfo *head = &(myHeads[myPIdMod->getInfoForId(pId).rank]);
        if (head->primary && head->primary->needsSecondary() && !head->secondary)
        {
            //Create an already completed P2P Op
            BlockingP2P *op = new BlockingP2P (this, pId, lId, false);
            op->offerMatchedReceive(false,0);
            handleNewOp (op->getIssuerRank(), op);
        }

        return GTI_ANALYSIS_SUCCESS;
    }

    //Regular handling
    BlockingP2P *op = new BlockingP2P (this, pId, lId, false);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// wait
//=============================
GTI_ANALYSIS_RETURN BlockingState::wait (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    //Did we initialize the heads ?
    initHeads (pId);

    BlockingCompletion *op = new BlockingCompletion (this, pId, lId, request);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitAny
//=============================
GTI_ANALYSIS_RETURN BlockingState::waitAny (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType* requests,
        int count,
        int numProcNull)
{
#ifdef GTI_VT
    VT_TRACER("BlockingCompletion::waitAny");
#endif /*GTI_VT*/

    //Did we initialize the heads ?
    initHeads (pId);

    BlockingCompletion *op = new BlockingCompletion (this, pId, lId, count, requests, false, numProcNull > 0);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitAll
//=============================
GTI_ANALYSIS_RETURN BlockingState::waitAll (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull)
{
    //Did we initialize the heads ?
    initHeads (pId);

    BlockingCompletion *op = new BlockingCompletion (this, pId, lId, count, requests, true, numProcNull > 0);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitSome
//=============================
GTI_ANALYSIS_RETURN BlockingState::waitSome (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull)
{
    //Did we initialize the heads ?
    initHeads (pId);

    BlockingCompletion *op = new BlockingCompletion (this, pId, lId, count, requests, false, numProcNull > 0);

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completedRequest
//=============================
GTI_ANALYSIS_RETURN BlockingState::completedRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    //Did we initialize the heads ?
    initHeads (pId);

    BlockingRequestCompletion *op = new BlockingRequestCompletion (this, pId, lId, request);

    if (op->isInvalid())
    {
        delete op;
        return GTI_ANALYSIS_SUCCESS;
    }

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completedRequests
//=============================
GTI_ANALYSIS_RETURN BlockingState::completedRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count)
{
    //Did we initialize the heads ?
    initHeads (pId);

    BlockingRequestCompletion *op = new BlockingRequestCompletion (this, pId, lId, count, requests);

    if (op->isInvalid())
    {
        delete op;
        return GTI_ANALYSIS_SUCCESS;
    }

    handleNewOp (op->getIssuerRank(), op);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// initHeads
//=============================
void BlockingState::initHeads (MustParallelId pId)
{
    initHeads (myPIdMod->getInfoForId(pId).rank);
}

//=============================
// initHeads
//=============================
void BlockingState::initHeads (int rank)
{
    if (myHeads.size () == 0)
        myHeads.resize(myCTrack->getComm(rank, myCTrack->getWorldHandle())->getGroup()->getSize());
}

//=============================
// handleNewOp
//=============================
bool BlockingState::handleNewOp (int rank, BlockingOp* newOp)
{
#ifdef MUST_EXHAUSTIVE_CHECKPOINT_TEST
    static bool printedTeaser = false;
    if (!printedTeaser)
    {
        std::cout << "INFORMATION: the exhaustive checkpoint verification test is enabled for this build, you should not use this build for production tests!" << std::endl;
        printedTeaser = true;
    }

    static bool didTheHandle =false;
    BlockingOp* opCopy;

    if (!didTheHandle)
    {
        myP2PMatch->checkpoint();
        myCollMatch->checkpoint();
        this->checkpoint();
        myOrder->checkpoint();
        opCopy = newOp->copy();
    }
    didTheHandle = !didTheHandle;
#endif

    if (myOrder->isRankOpen(rank))
    {
        //Process
        PROCESSING_RETURN ret = newOp->process(rank);

        //reprocessing should never be necessary ....
        if (ret == PROCESSING_REEXECUTE)
        {
            std::cerr << "Internal error in BlockingState, an operation returned PROCESSING_REEXECUTE, which should not happen!" << std::endl;
            assert (0);
        }
    }
    else
    {
        //Heuristic for deadlock detection
        /*
         * For potential deadlocks (e.g. send-send deadlock) the application can continue
         * while BlockingState keeps some ranks blocked, we won't get a timeout in that
         * case while at the same time the queues might explode before we get the finalize.
         * For that we must do a deadlock detection before the queues explode ....
         */
        static bool inQueueHandling = false;
        static int numSteps = 0;
        int queueSize = myOrder->getTotalQueueSize();

        if (!inQueueHandling && queueSize > BLOCKING_STATE_MAX_QUEUE_SIZE + numSteps*BLOCKING_STATE_STEP_INCREASE)
        {
            inQueueHandling = true;
            numSteps++;

            //Call a deadlock detection and resolve suspension!
            bool wasSuspended = myOrder->isSuspended();
            bool hadDeadlock = handleDeadlockDetection(true);

            if (wasSuspended && !hadDeadlock)
            {
                /*
                 * Give a warning, we just decided some suspension reasons at random
                 * this is necessary to keep queues from swelling too large (i.e. if we
                 * lack an early wc receive status that is leaked). However, this can cause
                 * serious harm later on if the following happens:
                 * - We used a different decision as MPI did
                 * - The code uses the knowledge of MPIs decission to influence its control flow
                 */
                myLogger->createMessage(
                        MUST_INFO_ENFORCED_WC_SOURCE_DECISION,
                        MustInformationMessage,
                        "MUST had a high number of queued operations while a wildcard receive with at least one possible match was not completed. MUST usually waits until the completion of this receive occurs before it continues its analysis, however it seams that MUSTS queues might become too large if this strategy was continued. Thus, MUST decided a match for this receive to allow continued analysis. Note that this may have been a different match than the MPI implementation decided, if so false positives may occur after this message.");
            }

            inQueueHandling = false;
        }

        //Queue
        myOrder->enqueueOp(rank, newOp);
    }

#ifdef MUST_EXHAUSTIVE_CHECKPOINT_TEST
    if (didTheHandle)
    {
        myP2PMatch->rollback();
        myCollMatch->rollback();
        this->rollback();
        myOrder->rollback();
        return handleNewOp (rank, opCopy);
    }
#endif

    return true;
}

//=============================
// applyNewP2POp
//=============================
bool BlockingState::applyNewP2POp (BlockingP2P* op)
{
    HeadInfo *head = &(myHeads[op->getIssuerRank()]);

    //==1) Check integrity
    if (op->isSend())
    {
        //For a send it must be the first op
        assert (!head->primary && !head->secondary);
    }
    else
    {
        //For a receive there may also already be a send of a sendrecv
        assert (!head->secondary);
    }

    //==2)Apply
    if (head->primary)
    {
        head->secondary = op;
        head->primary->registerSecondaryOp(op);
    }
    else
    {
        head->primary = op;
    }

    //==3) Is it already completed ?
    //=> Offer completed blocking P2P ops
    /*
     * Complication: If a recv part of a sendrecv uses MPI_PROC_NULL
     *                       it arrives here as an already completed recv!
     */
    if (op->isSend())
    {
        if (head->hasSendCompletion || op->canComplete())
        {
            if (!op->canComplete())
            {
                if (op->offerMatchedSend(false,0))
                    head->hasSendCompletion = false;
            }

            if (op->canComplete())
            {
                if (!op->isSrsend())
                {
#ifdef MUST_DEBUG
                    myTimeStep++;
#endif
                    return completeHead (op->getIssuerRank(), head);
                }
            }
        }
    }
    else
    {
        if (head->hasReceiveCompletion || op->canComplete())
        {
            if (!op->canComplete())
            {
                if (op->offerMatchedReceive(false,0))
                    head->hasReceiveCompletion = false;
            }

            if (op->canComplete())
            {
#ifdef MUST_DEBUG
                myTimeStep++;
#endif
                if (head->primary->canComplete() && (!head->secondary || head->secondary->canComplete()))
                    return completeHead (op->getIssuerRank(), head);
            }
        }
    }

    //==4) If not completed, do we need to block the head ?
    if (!op->isSrsend())
        myOrder->blockRank(op->getIssuerRank());

    return true;
}

//=============================
// applyNewCollectiveOp
//=============================
bool BlockingState::applyNewCollectiveOp (BlockingColl* op)
{
    HeadInfo *head = &(myHeads[op->getIssuerRank()]);

    //==1) Check integrity
    //For a collective there must only be a single op
    assert (!head->primary && !head->secondary);

    //==2)Apply
    head->primary = op;

    //==3) Is it already completed ?
    //=> Offer completed collective
    if (head->hasCollCompletion)
    {
        if (op->offerMatchedCollective())
        {
            head->hasCollCompletion = false;
#ifdef MUST_DEBUG
            myTimeStep++;
#endif
            return completeHead (op->getIssuerRank(), head);
        }
    }

    //==4) If not completed, block the head
    myOrder->blockRank(op->getIssuerRank());

    return true;
}

//=============================
// applyNewCompletionOp
//=============================
bool BlockingState::applyNewCompletionOp (BlockingCompletion* op)
{
    HeadInfo *head = &(myHeads[op->getIssuerRank()]);

    //==1) Check integrity
    //For a completion there must only be a single op
    assert (!head->primary && !head->secondary);

    //==2)Apply
    head->primary = op;

    //==3) How complete are we ?
    //=> Offer completed requests
    std::list<MustRequestType>::iterator rIter;
    for (rIter = head->matchedReqs.begin(); rIter != head->matchedReqs.end(); rIter++)
    {
        //Do we have to offer any more at all?
        if (op->canComplete())
            break;

        //This is kinda ambigious its of no interest whether we call receive or send, its implementation doesn't cares about that ... so do we ...
        op->offerMatchedReceive(true, *rIter);
        /*
         * IMPORTANT: we do not remove used request from head->matchedReqs yet, this is done in the
         *                     completion update, as we can't tell what MPI is going to decide for.
         */
    }

    //Was that enough stuff to complete ?
    if (op->canComplete())
    {
#ifdef MUST_DEBUG
        myTimeStep++;
#endif
        return completeHead (op->getIssuerRank(), head);
    }

    //==4) If not completed, block the head
    myOrder->blockRank(op->getIssuerRank());

    return true;
}

//=============================
// applyNewCompletionUpdateOp
//=============================
bool BlockingState::applyNewCompletionUpdateOp (BlockingRequestCompletion* op)
{
    HeadInfo *head = &(myHeads[op->getIssuerRank()]);

    //==1) Check integrity
    //An update is only processed if unblocked, and as it can't arrive in between a send-recv, both ops must be NULL
    assert (!head->primary && !head->secondary);

    //==2) Sort it in
    int rCount = 1;
    std::vector<MustRequestType> *rs =NULL;
    if (op->isArray()) rs = op->getRequests();
    if (rs) rCount = rs->size();

    for (int i = 0; i < rCount; i++)
    {
        //Determine the request to update
        MustRequestType r;

        if (!rs)
            r = op->getRequest();
        else
            r = (*rs)[i];

        //Is this request in the matched reqs ?
        /**
         * @todo this can cause O(N^2) time complexity where N is number of requests used in a completion.
         *            Usually N should be low, even at scale. However if it is large it would be more clever to
         *            sort both lists leading to O(2*N*logN + N) -> O(N*logN) as time complexity.
         */
        std::list<MustRequestType>::iterator rIter;
        bool found = false;
        for (rIter = head->matchedReqs.begin(); rIter != head->matchedReqs.end(); rIter++)
        {
            if (r == *rIter)
            {
                head->matchedReqs.erase(rIter);
                found = true;
                break;
            }
        }

        //If found and removed all is done
        if (found) continue;

        //If not found we need to add this to the list of unexpected completions
        head->unexpectedCompletions.push_back(r);
    }

    //delete the operation
    delete (op);

    return true;
}

//=============================
// completeHead
//=============================
bool BlockingState::completeHead (int rank, HeadInfo* head)
{
#ifdef MUST_DEBUG
    MustParallelId pId;
    MustLocationId lId;

    if (head->primary) //There should always be a primary ....
    {
        pId = head->primary->getPId ();
        lId = head->primary->getLId ();
    }

    int time = myTimeStep;
    if (head->specialTime >= 0)
    {
        time = head->specialTime;
        head->specialTime = -1;
    }

     //Add the completed op to the timeline
    myHistory[time][rank] = std::make_pair(pId, lId);
#endif

    //Delete the ops
    if (head->primary)
        delete head->primary;
    head->primary = NULL;

    if (head->secondary)
        delete head->secondary;
    head->secondary = NULL;

    //Resume the ranks
    myOrder->resumeRank(rank);

    return true;
}

//=============================
// newMatch (P2P)
//=============================
void BlockingState::newMatch (
                        int sendRankWorld,
                        int receiveRankWorld,
                        bool sendHasRequest,
                        MustRequestType sendRequest,
                        bool receiveHasRequest,
                        MustRequestType receiveRequest,
                        must::MustSendMode sendMode)
{
    //Did we initialize the heads ?
    initHeads (sendRankWorld);

    //We ignore MPI_Bsend!
    if (sendHasRequest || sendMode != MUST_BUFFERED_SEND)
        newMatchedP2P (true, sendRankWorld, sendHasRequest, sendRequest);

    newMatchedP2P (false, receiveRankWorld, receiveHasRequest, receiveRequest);

#ifdef MUST_DEBUG
     myTimeStep++;
#endif
}

//=============================
// newMatchedP2P
//=============================
void BlockingState::newMatchedP2P (bool isSend, int rank, bool hasRequest, MustRequestType request)
{
    HeadInfo *head = &(myHeads[rank]);
    bool consumed = false;

    //==1) Has the head any op ? If so offer the send/receive
    if (head->primary)
    {
        if (isSend)
            consumed = head->primary->offerMatchedSend(hasRequest, request);
        else
            consumed = head->primary->offerMatchedReceive(hasRequest, request);
    }

    if (!consumed && head->secondary)
    {
        if (isSend)
            consumed = head->secondary->offerMatchedSend(hasRequest, request);
        else
            consumed = head->secondary->offerMatchedReceive(hasRequest, request);
    }

    //==2) Remember this match (if necessary)
    if (hasRequest)
    {
        //If we have a request we must remember it irrespective of whether it was consumed!
        bool wasMissing = false;

        //a) Is it in the unexpected request completions ?
        std::list<MustRequestType>::iterator rIter;
        for (rIter = head->unexpectedCompletions.begin(); rIter != head->unexpectedCompletions.end(); rIter++)
        {
            if (*rIter == request)
            {
                //It was missing, remove it from the unexpected completions
                head->unexpectedCompletions.erase(rIter);
                wasMissing = true;
                break;
            }
        }

        //b)If it was not a missing one, add it to the available ones
        if (!wasMissing)
        {
            head->matchedReqs.push_back(request);
#ifdef MUST_DEBUG
            head->specialTime = myTimeStep;
#endif
        }
    }
    else
    {
        //If we do not have a request we only have to remember if it was not consumed
        if (!consumed)
        {
            if (isSend)
                head->hasSendCompletion = true;
            else
                head->hasReceiveCompletion = true;
#ifdef MUST_DEBUG
            head->specialTime = myTimeStep;
#endif
        }
    }

    //==3) Did we complete the wait state
    //This can come within a send and a recv of a sendrecv, so we must be a bit careful!
    bool resumeRank = false;

    if (head->primary && head->primary->canComplete())
    {
        if (!head->primary->needsSecondary())
        {
            resumeRank = true;
        }
        else
        {
            if (head->secondary && head->secondary->canComplete())
                resumeRank = true;
        }
    }

    if (resumeRank)
    {
        completeHead (rank, head);
    }
}

//=============================
// newMatch (Collective)
//=============================
void BlockingState::newMatch (
                        MustCollCommType collId,
                        I_Comm* comm)
{
    //Did we initialize the heads ?
    initHeads (0);

#ifdef MUST_DEBUG
     myTimeStep++;
#endif

     //For intra comms total size is group size, for intercoms its the sum of both groups
     int commTotalSize = comm->getGroup()->getSize();
     int localSize=commTotalSize;
     if (comm->isIntercomm())
         commTotalSize+=comm->getRemoteGroup()->getSize();

    // Loop over the comm and update all heads it contains
    for (int i = 0; i < commTotalSize; i++)
    {
        //Translate into MPI_COMM_WORLD
        int rank;

        if (i < localSize)
            comm->getGroup()->translate(i, &rank);
        else
            comm->getRemoteGroup()->translate(i-localSize, &rank);

        //Get the head
        HeadInfo *head = &(myHeads[rank]);
        bool consumed = false;

        //Offer the completion if possible (we only need to offer to primary, there is never a collective op in secondary
        if (head->primary)
        {
            consumed = head->primary->offerMatchedCollective();
        }

        //If not consumed store the collective
        if (!consumed)
        {
            head->hasCollCompletion = true;
#ifdef MUST_DEBUG
            head->specialTime = myTimeStep;
#endif
        }

        //Did we complete the head ?
        //We can only complete collectives, so it has to be a primary which needs no secondary if we completed something
        if (head->primary && head->primary->canComplete() && !head->primary->needsSecondary())
        {
            completeHead (rank, head);
        }
    }

#ifdef MUST_DEBUG
     myTimeStep++;
#endif
}

//=============================
// printHistoryAsDot
//=============================
#ifdef MUST_DEBUG
void BlockingState::printHistoryAsDot (void)
{
    std::ofstream out ((((std::string)MUST_OUTPUT_DIR)+((std::string)"must_block_history.dot")).c_str());

    out
        << "digraph BlockHistory {" << std::endl
        << "nodesep=0.0;" << std::endl
        << "ranksep=0.0;" << std::endl;

    for (int i = 0; i < myHeads.size(); i++)
    {
        out << i << ";" << std::endl;
        out << i << "->node0x" << i << ";" << std::endl;
    }

    BlockHistory::iterator tIter;
    int time = 0;
    for (tIter = myHistory.begin(); tIter != myHistory.end(); tIter++, time++)
    {
        TimeSlice *slice = &(tIter->second);

        for (int i = 0; i < myHeads.size(); i++)
        {
            out << "node" << time << "x" << i << "[shape=box, width=3, label=\"";

            if ( slice->find (i) != slice->end())
            {
                out << myLIdMod->getInfoForId ((*slice)[i].first, (*slice)[i].second).callName;
            }

            out << "\"];" << std::endl;

            if (time > 0)
            {
                out << "node" << time-1 << "x" << i << "->"<< "node" << time << "x" << i << ";" << std::endl;
            }
        }

        out << "{ rank = same ; ";
        for (int i = 0; i < myHeads.size(); i++)
        {
            out<< "node" << time << "x" << i << " ; ";
        }
        out << "}" << std::endl;
    }

    out << "}" << std::endl;

    out.close ();

    std::cout << "BlockingState: Printed block history into a file named \"" << MUST_OUTPUT_DIR << "must_block_history.dot\" use DOT to visualize it." << std::endl;
}
#endif

//=============================
// detectDeadlock
//=============================
bool BlockingState::detectDeadlock (void)
{
    //If we already had a deadlock, then we don't do any further detection
    static bool hadDeadlock = false;

    if (hadDeadlock)
        return true;

    //------------------------------------
    //1) Build WFG
    Wfg wfg;
    std::map<I_Comm*, std::string> emptyLabels;
    for (HeadStates::size_type i = 0; i < myHeads.size(); i++)
    {
        HeadInfo* head = &(myHeads[i]);

        /*
         * We only ask the primary for the wait for stuff,
         * if it has a secondary, the secondary should
         * register at the primary. With that the primary
         * can evaluate and return the information of
         * the secondary in one go.
         */
        if (!head->primary)
            continue;

       BlockingOp *op = head->primary;

       bool isMixed = op->isMixedOp();
       int numSubs = 0;
       if (isMixed) numSubs = op->mixedOpGetNumSubNodes ();
       ArcType type = op->getWaitType();

       std::list<int> toList = op->getWaitedForRanks(NULL, NULL, emptyLabels);
       std::list<int>::iterator toIter;

       //Add the regular arcs
       for (toIter = toList.begin(); toIter != toList.end(); toIter++)
       {
           int to = *toIter;
           wfg.addArc(i, to, type);
       }

       //Handle sub nodes
       for (int sId = 1; sId <= numSubs; sId++)
       {
           int subNode = sId*myHeads.size()+i;

           //Add arcs to sub node
           wfg.addArc(i, subNode, type);

           //Add arcs from sub node
           toList = op->getSubNodeWaitedForRanks(sId-1, NULL, NULL, NULL, NULL, emptyLabels);
           for (toIter = toList.begin(); toIter != toList.end(); toIter++)
           {
               int to = *toIter;
               wfg.addArc(subNode, to, ARC_OR);
           }
       }
    }

    //------------------------------------
    //2) Detect
    bool hasDeadlock = false;
    std::list<int> deadlockedNodes;
    wfg.detectDeadlock(&hasDeadlock, &deadlockedNodes);

    if (!hasDeadlock)
        return false;

    hadDeadlock = true;

/* DEBUGGIN (disable the return above to make it work)
    deadlockedNodes.clear();
    for (int i = 0; i < myHeads.size(); i++)
    {
        deadlockedNodes.push_back(i);

        HeadInfo* head = &(myHeads[i]);
        if (!head->primary)
            continue;

        int numSubs = head->primary->mixedOpGetNumSubNodes ();
        for (int sId = 1; sId <= numSubs; sId++)
            deadlockedNodes.push_back (sId*myHeads.size()+i);
    }
*/
    MUST_OUTPUT_DIR_CHECK

    //------------------------------------
    //3-alpha) Let the Lost-Message module know that we had a deadlock
    //              (We should not print lost messages in that case)
    myP2PMatch->notifyDeadlock();

    //------------------------------------
    //3) Print Deadlock (If necessary)
    std::vector<bool> isNodeDl (myHeads.size(), false);
    std::list<int>::iterator nodeIter;

    //alpha) determine active communicators
    std::map<I_Comm*, std::string> commLabels = generateActiveCommLabels (&deadlockedNodes);

    //beta) create message queue graph, update comm labels, and generate a call stack for the msg queue operations
    std::list<std::pair<MustParallelId, MustLocationId> > msgQueueLocations;
    generateReducedMessageQueueGraph (&deadlockedNodes, &commLabels, &msgQueueLocations);

#ifdef USE_CALLPATH
    generateParallelCallStackGraph (
            msgQueueLocations,
            (((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_DeadlockMessageQueueStacked.dot")).c_str(),
            true,
            &deadlockedNodes,
            &commLabels);
#endif /*USE_CALLPATH*/

    //gamma) locations to include in a parallel call stack (if we have stack tracing)
    std::list<std::pair<MustParallelId, MustLocationId> > callStackLocations;

    //a) Mark all deadlocked nodes
    for (nodeIter = deadlockedNodes.begin(); nodeIter != deadlockedNodes.end(); nodeIter++)
    {
        int node = *nodeIter;
        if (node >= myHeads.size()) continue; //get rid of sub nodes

        isNodeDl[node] = true;
    }

    //b) Prepare the DOT output
    static int printCount = 0;
    std::ofstream out;
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
		out	<< "graph [bgcolor=transparent]" << std::endl;
#endif

    //c) Iterate over all deadlocked nodes
    for (nodeIter = deadlockedNodes.begin(); nodeIter != deadlockedNodes.end(); nodeIter++)
    {
        int from = *nodeIter;
        bool isSubNode = false;
        int subId;
        int realId = from;

        if (from >= myHeads.size())
        {
            subId = from / myHeads.size();
            from = from % myHeads.size();
            isSubNode = true;
        }

        HeadInfo* head = &(myHeads[from]);
        BlockingOp *op = head->primary;
        if (!op)
        {
            out << from << ";" << std::endl;
            continue;//THIS may happen in debugging cases
        }

        if (!isSubNode)
        {
            //add to call stack locations
            callStackLocations.push_back(std::make_pair(op->getPId(), op->getLId()));

            //Print this node
            out << from << " [label=\"{" << myPIdMod->getInfoForId(op->getPId()).rank << ": " << myLIdMod->getInfoForId(op->getPId(), op->getLId()).callName;

            if (op->isCollective())
            {
                I_Comm* collComm = op->getUsedComms().front();
                if (collComm)
                {
                    std::map<I_Comm*, std::string>::iterator labelIter;
                    for (labelIter = commLabels.begin(); labelIter != commLabels.end(); labelIter++)
                    {
                        if (collComm->compareComms(labelIter->first))
                        {
                            out << "|comm=" << labelIter->second;
                            break;
                        }
                    }
                }
            }

            out << "}\", shape=record];" << std::endl;

            //Prepare labels and strings
            std::list<std::string> labels;
            std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > refs;
            ArcType type = op->getWaitType();
            std::list<int> toList = op->getWaitedForRanks( &labels, &refs, commLabels);

            std::string style = "solid";
            if (type == ARC_OR) style = "dashed";

            //Iterate over all to arcs
            std::list<int>::iterator toIter;
            std::list<std::string>::iterator labelIter;
            std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > >::iterator refIter;
            for (toIter = toList.begin(), labelIter = labels.begin(), refIter = refs.begin();
                   toIter != toList.end();
                   toIter++, labelIter++, refIter++)
            {
                int to = *toIter;

                //Is an arc to a deadlocked node ?
                if (!isNodeDl[to]) continue;

                //Labels and ref
                std::string label = "";
                bool hasRef = false;
                MustParallelId pId;
                MustLocationId lId;

                if (labelIter != labels.end()) label = *labelIter;
                if (refIter != refs.end())
                {
                    hasRef = refIter->first;
                    pId = refIter->second.first;
                    lId = refIter->second.second;

                    //Add to parallel call stack!
                    if (hasRef)
                        callStackLocations.push_back(std::make_pair(pId, lId));
                }

                //Add to graph
                std::stringstream extra;
                if (hasRef) extra << ":" << myLIdMod->getInfoForId(pId, lId).callName << "@" << myPIdMod->getInfoForId(pId).rank;
                out
                    << from << "->" << to << "[label=\"" << label << extra.str()<< "\", style=" << style << "];" << std::endl;
            }
        }
        else
        {
            //Prepare label, to arcs and strings
            std::string label;
            std::stringstream fullLabel;
            bool hasRef;
            MustParallelId pId;
            MustLocationId lId;
            std::list<int> toList = op->getSubNodeWaitedForRanks(subId-1, &label, &hasRef, &pId, &lId, commLabels);

            //Add to parallel call stack!
            if (hasRef)
                callStackLocations.push_back(std::make_pair(pId, lId));

            //print
            std::list<int>::iterator toIter;

            fullLabel << label;
            if (hasRef)
                fullLabel <<":" << myLIdMod->getInfoForId(pId, lId).callName << "@" << myPIdMod->getInfoForId(pId).rank;

            //Print this sub node
            out << realId << " [label=\"" << fullLabel.str() << "\", shape=diamond];" << std::endl;

            //Add arc to this sub node
            out << from << "->" << realId << " [weight=5, style=solid];" << std::endl;

            //Add arcs from this sub node
            for (toIter = toList.begin(); toIter != toList.end(); toIter++)
            {
                int to = *toIter;

                //Is an arc to a deadlocked node ?
                if (!isNodeDl[to]) continue;

                //Add to graph
                out << realId << "->" << to << "[style=dashed];" << std::endl;
            }
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
		out	<< "graph [bgcolor=transparent]" << std::endl;
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
        << "    dia [label=\"Sub Operation\", shape=diamond];" << std::endl
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
		out	<< "  }" << std::endl;
#endif /*DOT*/
    out << "}" << std::endl;
    out.close ();

    //------------------------------------
    //3c) create a parallel call stack graph
#ifdef USE_CALLPATH
    generateParallelCallStackGraph (
            callStackLocations,
            (((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_DeadlockCallStack.dot")).c_str(),
            false,
            NULL,
            NULL);
#endif /*USE_CALLPATH*/

    //------------------------------------
    //3c) draw graphs from generated DOT files
#ifdef DOT
      std::string command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng ") +((std::string)MUST_OUTPUT_DIR) + ((std::string)"MUST_Deadlock.dot -o ") +((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_Deadlock.png");
      system (command.c_str());
      command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueue.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueue.png");
      system (command.c_str());
#ifdef  USE_CALLPATH
      command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockCallStack.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockCallStack.png");
      system (command.c_str());
      command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueueStacked.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockMessageQueueStacked.png");
      system (command.c_str());
#endif /*USE_CALLPATH*/
      command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) "  -Tpng "+MUST_OUTPUT_DIR+"MUST_DeadlockLegend.dot -o "+MUST_OUTPUT_DIR+"MUST_DeadlockLegend.png");
      system (command.c_str());

      std::stringstream commOverviewStream;
      generateCommunicatorOverview (&commLabels, &commOverviewStream);
      generateDeadlockHtml(&commOverviewStream);
#endif /*DOT*/

    //------------------------------------
    //4) Create the log event
    std::stringstream stream;
    std::list <std::pair <MustParallelId, MustLocationId> > refs;

    for (nodeIter = deadlockedNodes.begin(); nodeIter != deadlockedNodes.end() && refs.size() < 5; nodeIter++)
    {
        int from = *nodeIter;

        if (from >= myHeads.size())
            continue;

        HeadInfo* head = &(myHeads[from]);
        BlockingOp *op = head->primary;
        if (!op) continue; //Should only happen in debugging ...

        refs.push_back(std::make_pair(op->getPId(),op->getLId()));
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

    //------------------------------------
    //5) Print something to std::err
    std::cerr
        << "============MUST===============" << std::endl
        << "ERROR: MUST detected a deadlock, detailed information is available in the MUST output file."
        << " You should either investigate details with a debugger or abort, the operation of MUST will stop from now." << std::endl
        << "===============================" << std::endl;

    return true;
}

//=============================
// notifyFinalize
//=============================
GTI_ANALYSIS_RETURN BlockingState::notifyFinalize (
                    gti::I_ChannelId *thisChannel)
{
    //Is completely reduced without a channel id ? (Reduced on this place)
    if (thisChannel)
    {
        //Initialize completion tree
        if (!myFinCompletion)
            myFinCompletion = new CompletionTree (
                    thisChannel->getNumUsedSubIds()-1, thisChannel->getSubIdNumChannels(thisChannel->getNumUsedSubIds()-1));

        myFinCompletion->addCompletion (thisChannel);
    }

    if (!thisChannel || myFinCompletion->isCompleted())
    {
        //Do a final deadlock detection
        handleDeadlockDetection(true);
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// timeout
//=============================
void BlockingState::timeout (void)
{
    //1) Check whether we see a deadlock with the current informaiton
    handleDeadlockDetection(false);

    //2) Request that all ancestor TBON nodes provide us with all information they have
    gtiNotifyFlushP f;
    if (getBroadcastFunction("gtiNotifyFlush", (GTI_Fct_t*) &f) == GTI_SUCCESS)
        (*f) ();

#ifdef MUST_DEBUG
    printHistoryAsDot ();
#endif
}

#ifdef DOT
//=============================
// generateDeadlockHtml
//=============================
void BlockingState::generateDeadlockHtml (std::stringstream *commOverview)
{
	//Print the two maps as dot
	std::ofstream out;
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
			<< " , while a parallel call stack view summarizes the locations of the MPI calls that cause the deadlock" << std::endl
#endif /*USE_CALLPATH*/
			<< ". Below these graphs, a message queue graph shows active and unmatched point-to-point communications." << std::endl
			<< " This graph only includes operations that could have been intended to match a point-to-point operation that is relevant to the deadlock situation."
#ifdef USE_CALLPATH
            << "  Finally, a parallel call stack shows the locations of any operation in the parallel call stack." << std::endl
            << " The leafs of this call stack graph show the components of the message queue graph that they span." << std::endl
#endif /*USE_CALLPATH*/
			<< " The application still runs, if the deadlock manifested" << std::endl
			<< " (e.g. caused a hang on this MPI implementation) you can attach to the involved ranks" << std::endl
			<< " with a debugger or abort the application (if necessary)." << std::endl
			<< "</td>" << std::endl
			<< "</tr>"  << std::endl

			<< "<!-- ACTIVE COMMS -->" << std::endl
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

			<< "<!-- RELEVANT P2P: Overview -->" << std::endl
			<< "<tr>" << std::endl
			<< "<td align=\"center\" bgcolor=\"#7777BB\" colspan=\"2\">" << std::endl
			<< "<b>Active and Relevant Point-to-Point Messages: Overview</b>" << std::endl
			<< "</td>" << std::endl
			<< "</tr>" << std::endl
			<< "<tr>" << std::endl
			<< "<td class=\"ee2\" colspan=\"2\" ><img src=\"MUST_DeadlockMessageQueue.png\" alt=\"Message queue\"></td>" << std::endl
			<< "</tr>" << std::endl

#ifdef  USE_CALLPATH
            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\"><b>Active and Relevant Point-to-Point Messages: Callstack-view</b></td>" << std::endl
            << "</tr>" << std::endl
            << "<tr>" << std::endl
            << "<td class=\"ee1\" ><img src=\"MUST_DeadlockMessageQueueStacked.png\" alt=\"stack\"></td>" << std::endl
            << "</tr>" << std::endl
#endif

			<< "</table>" << std::endl
			<< "</body>" << std::endl
			<< "</html>" << std::endl;
	out.flush();
	out.close();
}
#endif

//=============================
// generateActiveCommLabels
//=============================
std::map<I_Comm*, std::string> BlockingState::generateActiveCommLabels (std::list<int> *deadlockedTasks)
{
    std::map<I_Comm*, std::string> ret;

    if (!deadlockedTasks) return ret;

    char cur = 'A';
    std::list<I_Comm*> comms, temp;
    std::list<I_Comm*>::iterator commIter, tempIter;
    std::list<int>::iterator taskIter;

    for (taskIter = deadlockedTasks->begin(); taskIter != deadlockedTasks->end(); taskIter++)
    {
        if (*taskIter >= myHeads.size()) continue; //get rid of sub nodes

        for (int i = 0; i < 2; i++)
        {
            BlockingOp *op;
            if (i == 0)
                op = myHeads[*taskIter].primary;
            else
                op = myHeads[*taskIter].secondary;

            if (!op) continue;

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
    }

    return ret;
}

//=============================
// generateCommunicatorOverview
//=============================
void BlockingState::generateCommunicatorOverview (std::map<I_Comm*, std::string> *labels, std::stringstream *stream)
{
    std::map<I_Comm*, std::string>::iterator iter;

    (*stream)
        << "<table class=\"em\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">" << std::endl
        << "<tr class=\"em\" bgcolor=\"#9999DD\">" << std::endl;

    for (iter = labels->begin(); iter != labels->end(); iter++)
    {
        (*stream)
            << "<td class=\"em\">" << std::endl
            << "Comm:" << std::endl
            << "</td>" << std::endl
            << "<td class=\"em\">" << std::endl
            << iter->second << std::endl
            << "</td>" << std::endl;
    }

    (*stream)
            << "</tr>" << std::endl
            << "<tr class=\"ee1\">" << std::endl;

    for (iter = labels->begin(); iter != labels->end(); iter++)
    {
        //Create a short summary about the communicator
        std::stringstream infoStream;
        if (iter->first->isPredefined())
        {
            infoStream << iter->first->getPredefinedName();
        }
        else
        {
            //A user defined communicator
            infoStream << "User defined communicator with a group of " << iter->first->getGroup()->getSize() << " processes, representative constructor (rank  " << myPIdMod->getInfoForId(iter->first->getCreationPId()).rank << "):<br>";
            printLocation (iter->first->getCreationPId(),iter->first->getCreationLId(), infoStream);
        }

        (*stream)
            << "<td class=\"em\" colspan=\"2\">" << std::endl
            << infoStream.str() << std::endl
            << "</td>" << std::endl;
    }

    (*stream)
            << "</tr>" << std::endl
            << "</table>" << std::endl;

    return;
}

//=============================
// generateReducedMessageQueueGraph
//=============================
void BlockingState::generateReducedMessageQueueGraph (
        std::list<int> *deadlockedTasks,
        std::map<I_Comm*, std::string> *commLabels,
        std::list<std::pair<MustParallelId, MustLocationId> > *refs)
{
    std::ofstream out ((((std::string)MUST_OUTPUT_DIR)+((std::string)"MUST_DeadlockMessageQueue.dot")).c_str());

    //==Print header
    out
        << "digraph ReducedMessageQueue {" << std::endl
        << "graph [bgcolor=transparent]" << std::endl;

    //==Print the actual graph
    generatePartialReducedMessageQueueGraph (
            deadlockedTasks,
            commLabels,
            "",
            std::list<int> (),
            NULL,
            refs,
            out);

    //==Print trailer
    out
        << "}" << std::endl;
    out.flush();
    out.close();
}

//=============================
// generateParallelCallStackGraph
//=============================
void BlockingState::generateParallelCallStackGraph (
        std::list<std::pair<MustParallelId, MustLocationId> > locations,
        std::string outFileName,
        bool includeMsgQSubGraphs,
        std::list<int> *deadlockedTasks,
        std::map<I_Comm*, std::string> *commLabels)
{
#ifdef USE_CALLPATH
    std::list<std::pair<MustParallelId, MustLocationId> >::iterator locationIter;
    std::map <std::pair<int, std::string>, int>  symAndLevelToNodeId;
    std::map <int, std::pair<std::pair<MustParallelId, MustLocationId>, int> > leafInfos; //maps nodeId to (pId,lId) pair and parent node id
    std::map <std::pair<int, int>, std::set<int> > nodeConnectionToRanks;
    int nextNodeId = 0;

    //First fill the above two maps
    for (locationIter = locations.begin(); locationIter != locations.end(); locationIter++)
    {
        MustParallelId pId = locationIter->first;
        MustLocationId lId = locationIter->second;

        int from = myPIdMod->getInfoForId(pId).rank;

        LocationInfo &location = myLIdMod->getInfoForId(pId, lId);
        std::list<MustStackLevelInfo>::reverse_iterator stackIter;

        //Walk over the stack
        int lastNodeId = -1;
        int i = 0;
        for (stackIter = location.stack.rbegin(); ; stackIter++, i++)
        {
            std::pair<int, std::string> symAndLevel;

            if (stackIter != location.stack.rend())
            {
                MustStackLevelInfo info = *stackIter;

                //Get node id for this
                symAndLevel = std::make_pair (i, info.symName + "@" + info.fileModule + ":" + info.lineOffset);
            }
            else
            {
                symAndLevel = std::make_pair (lastNodeId, location.callName);
            }

            std::map <std::pair<int, std::string>, int>::iterator pos = symAndLevelToNodeId.find (symAndLevel);
            int nodeId;
            if (pos == symAndLevelToNodeId.end())
            {
                symAndLevelToNodeId.insert (std::make_pair (symAndLevel, nextNodeId));
                nodeId = nextNodeId;

                //Store that this was a leaf (if necessary)
                if (stackIter == location.stack.rend())
                {
                    leafInfos[nextNodeId] = std::make_pair(std::make_pair (pId,lId), lastNodeId);
                }

                nextNodeId++;
            }
            else
            {
                nodeId = pos->second;
            }

            //Nothing to do if this is the first node id
            if (lastNodeId < 0)
            {
                lastNodeId = nodeId;
                continue;
            }

            //Add to arcs
            std::pair<int, int> fromTo = std::make_pair(lastNodeId, nodeId);
            std::map <std::pair<int, int>, std::set<int> >::iterator arcPos = nodeConnectionToRanks.find (fromTo);

            if (arcPos == nodeConnectionToRanks.end())
            {
                std::set<int> temp;
                temp.insert(from);
                nodeConnectionToRanks.insert (std::make_pair (fromTo, temp));
            }
            else
            {
                arcPos->second.insert(from);
            }

            //Update last node id
            lastNodeId = nodeId;

            if (stackIter == location.stack.rend()) break;
        }
    }

    //Print the two maps as dot
    std::ofstream out (outFileName.c_str());

    out
        << "digraph DeadlockCallStack {" << std::endl
        << "graph [bgcolor=transparent]" << std::endl
        << "compound=true;" << std::endl;

    std::map <std::pair<int, std::string>, int>::iterator  stackNodeIter;
    for (stackNodeIter = symAndLevelToNodeId.begin(); stackNodeIter != symAndLevelToNodeId.end(); stackNodeIter++)
    {
        int level = stackNodeIter->first.first;
        std::string label = stackNodeIter->first.second;
        int nodeId = stackNodeIter->second;

        if (!includeMsgQSubGraphs || leafInfos.find(nodeId) == leafInfos.end())
        {
            //Regular node
            out << "Node" << nodeId << " [label=\"" << label << "\", shape=box];" << std::endl;
        }
        else
        {
            std::stringstream prefixStream;
            prefixStream << "subgraph" << nodeId;
            out
                << "subgraph cluster" << nodeId << " {" << std::endl
                << "color=black;" << std::endl
                << "style=rounded;" << std::endl
                << "label=\"" << label << "\";" << std::endl
                << "Node" << nodeId << "[label=\"\", shape=box, style=invis];" << std::endl;

            int parent = leafInfos[nodeId].second;
            MustParallelId pId = leafInfos[nodeId].first.first;
            MustLocationId lId = leafInfos[nodeId].first.second;

            std::list<int> fromTasks;
            std::set<int> *arcs = &(nodeConnectionToRanks[std::make_pair(parent, nodeId)]);
            std::set<int>::iterator arcIter;
            for (arcIter = arcs->begin(); arcIter != arcs->end(); arcIter++)
                fromTasks.push_back (*arcIter);

            std::list<MustStackLevelInfo> stack = myLIdMod->getInfoForId (pId,lId).stack;

            //Leaf node
            generatePartialReducedMessageQueueGraph (
                            deadlockedTasks,
                            commLabels,
                            prefixStream.str(),
                            fromTasks,
                            &stack,
                            NULL,
                            out);

            out << "}" << std::endl;
        }
    }

    std::map <std::pair<int, int>, std::set<int> >::iterator arcIter;
    for (arcIter = nodeConnectionToRanks.begin(); arcIter != nodeConnectionToRanks.end(); arcIter++)
    {
        int from = arcIter->first.first;
        int to = arcIter->first.second;

        std::set<int>::iterator nodePos;

        out << "Node" << from << "->Node" << to << " [label=\"Ranks: ";
        printIntegerList (out, arcIter->second, false);
        out << "\"";

        if (includeMsgQSubGraphs && leafInfos.find(to) != leafInfos.end())
        {
            out << ", lhead=cluster" << to;
        }

        out <<  "];" << std::endl;
    }

    out << "}" << std::endl;
    out.flush();
    out.close();
#endif
}

//=============================
// clearHeads
//=============================
void BlockingState::clearHeads (HeadStates *heads)
{
    //Clean up module data
    for (HeadStates::size_type i = 0; i < heads->size(); i++)
    {
        HeadInfo* head = &((*heads)[i]);

        if (head->primary)
            delete head->primary;

        if (head->secondary)
            delete head->secondary;

        head->primary = head->secondary = NULL;
    }
    heads->clear();
}

//=============================
// checkpoint
//=============================
void BlockingState::checkpoint (void)
{
    //==0) Module handles stay as they are

    //==1) myHeads
    clearHeads (&myCheckpointHeads);
    if (myCheckpointHeads.size() != myHeads.size())
        myCheckpointHeads.resize(myHeads.size());

    for (HeadStates::size_type i = 0; i < myHeads.size(); i++)
    {
        HeadInfo *currHead = &(myHeads[i]);
        HeadInfo *newHead = &(myCheckpointHeads[i]);

        //Copy
        newHead->matchedReqs = currHead->matchedReqs;
        newHead->unexpectedCompletions = currHead->unexpectedCompletions;
        newHead->hasCollCompletion = currHead->hasCollCompletion;
        newHead->hasSendCompletion = currHead->hasSendCompletion;
        newHead->hasReceiveCompletion = currHead->hasReceiveCompletion;

        newHead->primary = currHead->primary;
        if (newHead->primary) newHead->primary = newHead->primary->copy();

        newHead->secondary = currHead->secondary;
        if (newHead->secondary) newHead->secondary = newHead->secondary->copy();

        //Fix reference of primary to a secondary operation
        if (newHead->secondary && newHead->primary)
            newHead->primary->registerSecondaryOp(newHead->secondary);

#ifdef MUST_DEBUG
        newHead->specialTime = currHead->specialTime;
#endif
    }

    //==2) myFinCompletion
    if (myCheckpointFinCompletion) delete (myCheckpointFinCompletion);
    if (myFinCompletion)
        myCheckpointFinCompletion = myFinCompletion->copy();
    else
        myCheckpointFinCompletion = NULL;

    //==3) myTimeStep & myHistory (DEBUG)
#ifdef MUST_DEBUG
    myCheckpointHistory.clear();
    myCheckpointHistory = myHistory;
    myCheckpointTimeStep = myTimeStep;
#endif /*MUST_DEBUG*/
}

//=============================
// rollback
//=============================
void BlockingState::rollback (void)
{
    //==0) Module handles stay as they are

    //==1) myHeads
    clearHeads (&myHeads);
    if (myCheckpointHeads.size() != myHeads.size())
        myHeads.resize(myCheckpointHeads.size());
    myHeads = myCheckpointHeads;

    //IMPORTANT: we need to invalidate the ops in the checkpoint heads
    //                    otherwise we will lose them due to the HeadInfo destructor.
    for (HeadStates::size_type i = 0; i < myCheckpointHeads.size(); i++)
    {
        myCheckpointHeads[i].primary = myCheckpointHeads[i].secondary = NULL;
    }
    myCheckpointHeads.clear();

    //==2) myFinCompletion
    if (myFinCompletion) delete (myFinCompletion);
    myFinCompletion = myCheckpointFinCompletion;
    myCheckpointFinCompletion = NULL;

    //==3) myTimeStep & myHistory (DEBUG)
#ifdef MUST_DEBUG
    myHistory.clear();
    myHistory = myCheckpointHistory;
    myCheckpointHistory.clear();
    myTimeStep = myCheckpointTimeStep;
#endif /*MUST_DEBUG*/
}

//=============================
// handleDeadlockDetection
//=============================
bool BlockingState::handleDeadlockDetection (bool enforceAValidSuspensionDecision)
{
    //We may need to issue an assumed matching in case we are lacking a wildcard receive status
    if (myOrder->isSuspended())
    {
        //Create a MUST information message (as this may be time consuming and expensive)
        static bool wcExplorationWarned = false;
        if (!wcExplorationWarned)
        {
            myLogger->createMessage(
                    MUST_INFO_MISSING_WC_SOURCE_EXPLORATION,
                    MustInformationMessage,
                    "MUST issued a deadlock detection while a wildcard receive call (MPI_Recv/MPI_Irecv with MPI_ANY_SOURCE) was not yet completed. However, MUST found at least one potential match for this receive. MUST has to evaluate all different matches that could have been taken by any of the receives to perform a thorough deadlock detection now, this may have a severe performance impact.");
            /**
             * @todo we should give a location for such a wildcard receive to let users fix this!
             */
            wcExplorationWarned = true;
        }

        //Init our explorer for matches
        MatchExplorer explorer, validExploration;
        bool hadAValidExploration = false;
        int numExplorations = 0;

        //Look until we did all explorations
        static int usecToInvest = 10000000; //10s TODO should become something configurable (We abort after 10s of search)
        struct timeval searchStart, currentTime;
        gettimeofday(&searchStart, NULL);
        long usecSpent;
        do
        {
            bool wasValid = true; //Used to detect invalid suspension decisions

            //Checkpoint
            myP2PMatch->checkpoint();
            myCollMatch->checkpoint();
            this->checkpoint();
            myOrder->checkpoint();

            //Decide wc-matches until we aren't suspended anymore
            while (myOrder->isSuspended() && wasValid)
            {
                int indexToDecide = 0;
                int numAlternatives = 0;

                if (explorer.isKnownLevel ())
                {
                    indexToDecide = explorer.getCurrAlternativeIndex ();
                }

                wasValid = myP2PMatch->decideSuspensionReason (indexToDecide, &numAlternatives);

                if (explorer.isKnownLevel ())
                {
                    explorer.push ();
                }
                else
                {
                    explorer.addLevel (numAlternatives);
                    explorer.push ();
                }
            }

            //Store a valid exploration in case we want to enforce such an exploration
            if (wasValid)
            {
                hadAValidExploration = true;
                validExploration = explorer;
                validExploration.rewindExploration();
            }

            //Do an actual deadlock detection
            //We only issue it if we decided for a correct match
            if (wasValid && detectDeadlock ())
            {
//#ifdef MUST_DEBUG
                /**
                 * I decided to keep this as it helps to pinpoint why detection could become so expensive.
                 */
                std::stringstream stream;
                stream << "MUST used " << numExplorations + 1  << " explorations to discover this deadlock.";

                myLogger->createMessage(
                        MUST_INFO_MISSING_WC_SOURCE_EXPLORATION_STATISTICS,
                        MustInformationMessage,
                        stream.str());
//#endif
                return true;//We abort the search at the first deadlock we found
            }
            
            //Restart
            myP2PMatch->rollback();
            myCollMatch->rollback();
            this->rollback();
            myOrder->rollback();

            numExplorations++;

            gettimeofday(&currentTime, NULL);
            usecSpent = ((currentTime.tv_sec * 1000000 + currentTime.tv_usec) - (searchStart.tv_sec * 1000000 + searchStart.tv_usec));
        }
        while (usecSpent < usecToInvest && explorer.nextExploration() );

        if (usecSpent > usecToInvest)
            usecToInvest = usecToInvest * 2;

//#ifdef MUST_DEBUG
        /**
         * I decided to keep this as it helps to pinpoint why detection could become so expensive.
         */
        std::stringstream stream;
        stream << "MUST used " << numExplorations << " explorations to check for deadlock.";
        if (explorer.nextExploration())
        {
            stream << " Search was aborted due to excessive search time, if necessary a longer search will be performed in a following deadlock detection.";
        }

        myLogger->createMessage(
                MUST_INFO_MISSING_WC_SOURCE_EXPLORATION_STATISTICS,
                MustInformationMessage,
                stream.str());
//#endif

        //Does the caller wants us to enforce some valid exploration?
        if (enforceAValidSuspensionDecision && hadAValidExploration)
        {
            validExploration.rewindExploration();

            while (validExploration.isKnownLevel())
            {
                int decision = validExploration.getCurrAlternativeIndex();
                int temp;
                myP2PMatch->decideSuspensionReason (decision, &temp);
                validExploration.push();
            }

            if (myOrder->isSuspended())
            {
                std::cerr << "ERROR: Internal MUST error, tried to enforce a valid suspension decision but apparently this was not a valid decission series in the first place." << std::endl;
                assert (0);
            }
        }
    }
    else
    {
        //A simple and direct deadlock detection
        return detectDeadlock ();
    }

    return false;
}

//=============================
// printLocation
//=============================
void BlockingState::printLocation (MustParallelId pId, MustLocationId lId, std::stringstream &stream)
{
#ifndef USE_CALLPATH
        stream << myLIdMod->getInfoForId(pId, lId).callName;
#else
        LocationInfo &ref = myLIdMod->getInfoForId (pId, lId);

        stream << "<b>" << ref.callName << "</b> called from: <br>" << std::endl;

        std::list<MustStackLevelInfo>::iterator stackIter;
        int i = 0;
        for (stackIter = ref.stack.begin(); stackIter != ref.stack.end(); stackIter++, i++)
        {
            if (i != 0)
                stream << "<br>";
            stream << "#" << i << "  " << stackIter->symName << "@" << stackIter->fileModule << ":" << stackIter->lineOffset << std::endl;
        }
#endif
}

//=============================
// printIntegerList
//=============================
void BlockingState::printIntegerList (std::ostream &out, std::set<int> &ints, bool isTagList)
{
    //Iterate over the individual tags
    std::set<int>::iterator intIter;
    bool inInterval = false;
    int lastValue = -2;
    for (intIter = ints.begin(); intIter !=ints.end(); intIter++)
    {
        int value = *intIter;

        if (isTagList)
        {
            if (value == myConsts->getAnyTag())
            {
                if (inInterval)
                {
                    out << lastValue << ", ";
                }
                inInterval = false;

                out << "MPI_ANY_TAG";
                value = -2; //to enforce reset of lastTag
            }
        }

        if (!inInterval)
        {
            if (value == lastValue +1)
            {
                inInterval = true;
                out << "-";
            }
            else
            {
                if (intIter != ints.begin())
                    out << ", ";
                out << value;
            }
        }
        else// !inInterval
        {
            if (value != lastValue + 1)
            {
                out << lastValue << ", " << value;
                inInterval = false;
            }
        }

        lastValue=value;
    }

    if (inInterval)
    {
        out << lastValue;
    }
}

//=============================
// generateReducedMessageQueueGraph
//=============================
void BlockingState::generatePartialReducedMessageQueueGraph (
        std::list<int> *deadlockedTasks,
        std::map<I_Comm*, std::string> *commLabels,
        std::string prefix,
        std::list<int> fromTasks,
        std::list<MustStackLevelInfo> *fromStack,
        std::list<std::pair<MustParallelId, MustLocationId> > *refs,
        std::ostream &out)
{
    std::set<int> printedNodes;

    //select list of from tasks
    std::list<int> *fromList = deadlockedTasks;
    if (!fromTasks.empty()) fromList = &fromTasks;

    //==Print the sub-graph
    std::list<int>::iterator toIter, fromIter;
    for (toIter = deadlockedTasks->begin(); toIter != deadlockedTasks->end(); toIter++)
    {
        int to = *toIter;
        if (to >= myHeads.size()) continue; //get rid of sub ops!

        //Search for all meaningful operations and add them to the graph
        for (fromIter = fromList->begin(); fromIter != fromList->end(); fromIter++)
        {
            int from = *fromIter;
            if (from >= myHeads.size()) continue; //get rid of sub ops!

            //Do we need sends,receives, anything?
            bool sends = false, recvs = false;

            if (myHeads[to].primary)
            {
                if (myHeads[to].primary->waitsForASend (from))
                    sends=true;
                if (myHeads[to].primary->waitsForAReceive (from))
                    recvs=true;
            }
            if (myHeads[to].secondary)
            {
                if (myHeads[to].secondary->waitsForASend (from))
                    sends=true;
                if (myHeads[to].secondary->waitsForAReceive (from))
                    recvs=true;
            }

            if (!sends && !recvs) continue;

            std::list<must::P2PInfo> ops = myP2PMatch->getP2PInfos(from, to, sends, recvs);

            if (ops.empty())
                continue;

            //Arrange the ops nicely
            std::list<must::P2PInfo>::iterator opIter;
            std::map<bool, std::map<I_Comm*, std::set<int> > > sortedOps; //maps isSend to a map of all active comms that map to the individual and sorted tags (isSend->(comm->(tags))

            for (opIter = ops.begin(); opIter != ops.end(); opIter++)
            {
                //Does the op matches the given stack?
                if (fromStack)
                {
#ifdef USE_CALLPATH
                    LocationInfo info = myLIdMod->getInfoForId(opIter->pId, opIter->lId);

                    if (info.stack.size() != fromStack->size ())
                        continue;

                    std::list<MustStackLevelInfo>::iterator a,b;
                    for (a = fromStack->begin(), b = info.stack.begin(); a != fromStack->end(); a++, b++)
                    {
                        if (a->symName.compare(b->symName) != 0) break;
                        if (a->fileModule.compare(b->fileModule) != 0) break;
                        if (a->lineOffset.compare(b->lineOffset) != 0) break;
                    }

                    if (a != fromStack->end()) continue;
#endif /*USE_CALLPATH*/
                }

                //add the op to sorting
                std::map<I_Comm*, std::set<int> > *ref = &(sortedOps[opIter->isSend]);
                std::map<I_Comm*, std::set<int> >::iterator commIter;

                if (refs) refs->push_back (std::make_pair(opIter->pId, opIter->lId));

                for (commIter = ref->begin(); commIter != ref->end(); commIter++)
                {
                    if (commIter->first->compareComms(opIter->comm))
                        break;
                }

                if (commIter == ref->end())
                {
                    commIter = ref->insert(std::make_pair(opIter->comm, std::set<int>())).first;
                }

                commIter->second.insert(opIter->tag);
            }

            if (sortedOps.size() == 0)
                continue;

            //Do we need to print nodes?
            if (printedNodes.find(from) == printedNodes.end())
            {
                out << prefix << from << " [label=\"" << from <<  "\"  ];" << std::endl;
                printedNodes.insert (from);
            }
            if (printedNodes.find(to) == printedNodes.end())
            {
                out << prefix << to << " [label=\"" << to <<  "\"  ];" << std::endl;
                printedNodes.insert (to);
            }


            //Open the node label
            out << prefix << from << "->" << prefix << to << "[label=\" ";

            //Print all individual ops
            std::map<bool, std::map<I_Comm*, std::set<int> > >::iterator srIter;
            for (srIter = sortedOps.begin(); srIter != sortedOps.end(); srIter++)
            {
                bool send = srIter->first;
                std::map<I_Comm*, std::set<int> >::iterator commIter;

                if (send)
                    out << "sends={comms={";
                else
                    out << "receives={comms={";

                for (commIter = srIter->second.begin(); commIter != srIter->second.end(); commIter++)
                {
                    I_Comm* comm = commIter->first;
                    std::set<int>::iterator tagIter;
                    std::string commLabel = "";

                    //find the right comm label, add a new one if necessary
                    std::map<I_Comm*, std::string>::iterator labelIter;
                    char maxLabel = 'A';
                    for (labelIter = commLabels->begin(); labelIter != commLabels->end(); labelIter++)
                    {
                        if (labelIter->first->compareComms(comm))
                        {
                            commLabel = labelIter->second;
                            break;
                        }

                        if (labelIter->second[0] > maxLabel) maxLabel = labelIter->second[0];
                    }

                    if (labelIter == commLabels->end())
                    {
                        commLabel = std::string (1, maxLabel+1);
                        commLabels->insert(std::make_pair(comm, commLabel));
                    }

                    //print comm
                    if (commIter != srIter->second.begin()) out << ", ";
                    out << commLabel << "={tags={";

                    //Print tags
                    printIntegerList (out, commIter->second, true);

                    out << "}}";
                }

                out << "}} ";
            }

            //Close the node label
            out << "\"]; " << std::endl;

        } //end iterate over "FROM" tasks
    }//end iterate over "TO" tasks
}


/*EOF*/
