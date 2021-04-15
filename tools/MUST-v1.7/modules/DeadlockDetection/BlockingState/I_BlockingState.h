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
 * @file I_BlockingState.h
 *       @see I_BlockingState.
 *
 *  @date 08.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_ChannelId.h"

#include "BaseIds.h"
#include "MustEnums.h"
#include "MustTypes.h"

#include "I_P2PMatchListener.h"
#include "I_CollMatchListener.h"

#ifndef I_BLOCKINGSTATE_H
#define I_BLOCKINGSTATE_H

/**
 * Interface for tracking wait states resulting from (potentially) blocking operations.
 *
 * Suspension and operation queuing is implemented
 * with I_OperationReordering.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - BaseConstants
 * - CreateMessage
 * - CommTrack
 * - OperationReordering
 * - P2PMatch
 * - CollectiveMatch
 * - LocationAnalysis (for Debugging)
 * - RequestTrack (to distinguish MPI_REQUEST_NULL from other requests)
 *
 */
class I_BlockingState : public gti::I_Module, public must::I_P2PMatchListener, public must::I_CollMatchListener
{
public:
    /**
     * Notification of blocking collective operation that is directed
     * to all ranks.
     * (Must be mapped to the pre event of the call)
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param comm used for collective.
     * @param isSend true if send or no transfer, false if receive.
     * @param numTasks used for reduction counts progress of reduction.
     */
    virtual gti::GTI_ANALYSIS_RETURN CollAll (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            MustCommType comm,
            int isSend,
            int numTasks // counter for event aggregation
        )  = 0;

    /**
     * Notification of blocking collective operation that is directed
     * to a root rank.
     * (Must be mapped to the pre event of the call)
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param comm used for collective.
     * @param isSend true if send or no transfer, false if receive.
     * @param root rank for the collective (in MPI_COMM_WORLD)
     * @param numTasks used for reduction counts progress of reduction.
     */
    virtual gti::GTI_ANALYSIS_RETURN CollRoot (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            MustCommType comm,
            int isSend,
            int root,
            int numTasks // counter for event aggregation
    )  = 0;

    /**
     * Notification of a blocking send.
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     */
    virtual gti::GTI_ANALYSIS_RETURN send (
            MustParallelId pId,
            MustLocationId lId,
            int dest)  = 0;

    /**
     * Notification of the send part of a MPI_Sendrecv.
     * This is mapped to the split send part of sendrecv.
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     */
    virtual gti::GTI_ANALYSIS_RETURN srsend (
            MustParallelId pId,
            MustLocationId lId,
            int dest)  = 0;

    /**
     * Notification of a blocking receive.
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     */
    virtual gti::GTI_ANALYSIS_RETURN receive (
            MustParallelId pId,
            MustLocationId lId,
            int source)  = 0;

    /**
     * Notification of an MPI_Wait.
     * (Must be mapped to the pre event of the call)
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param request that is used in the completion.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN wait (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType request) = 0;

    /**
     * Notification of a MPI_Waitany.
     * (Must be mapped to the pre event of the call)
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param requests to complete one of.
     * @param count number of requests in array.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitAny (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType* requests,
            int count,
            int numProcNull) = 0;

    /**
     * Wait for completion of all requests from an array of requests.
     * (Mapped to pre of MPI_Waitall)
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param requests to complete.
     * @param count number of requests to complete.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitAll (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType *requests,
            int count,
            int numProcNull) = 0;

    /**
     * Wait for some requests out of an array.
     * (Mapped to pre of MPI_Waitsome)
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param requests to complete.
     * @param count number of requests.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitSome (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType *requests,
            int count,
            int numProcNull) = 0;

    /**
     * Notification of a successful MPI_Test[any] or MPI_Waitany that completed a request.
     * (Must be mapped to the post event of the call, with preconditioner)
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param request that completed.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN completedRequest (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType request) = 0;

    /**
     * Notification of completion of possibly multiple requests.
     * (Mapped to post of MPI_Waitsome, MPI_Testall, MPI_Testsome,
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param requests to complete.
     * @param count number of requests.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN completedRequests (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType *requests,
            int count) = 0;

    /**
     * Notification of finalize, this should trigger a deadlock detection to
     * determine whether there are any deadlocks.
     *
     * @param thisChannel channel id of process or set of completed processes
     *               triggering this action.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN notifyFinalize (
            gti::I_ChannelId *thisChannel) = 0;

    /**
     * We listen to timeouts to trigger deadlock detection when they occur.
     */
    virtual void timeout (void) = 0;

    /**
     * Creates a checkpoint of the current blocking state status.
     * The I_BlockingState::rollback function rewinds the state to
     * this checkpoint.
     * There may always only be one checkpoint, when this function is called
     * it overwrites any preceding checkpoint. Checkpoints are used to execute
     * matches that can be undone, e.g. if a wildcard source is missing.
     *
     * This mechanism will only work if I_OperationReordering,
     * I_CollectiveMatch and I_P2PMatch are checkpointed at the same
     * time, otherwise inconsistent state results.
     */
    virtual void checkpoint (void) = 0;

    /**
     * Rolls blocking state information back to the last checkpoint.
     *
     * This mechanism will only work if I_OperationReordering,
     * I_CollectiveMatch and I_P2PMatch are rolled back at the same
     * time, otherwise inconsistent state results.
     */
    virtual void rollback (void) = 0;

};/*class I_BlockingState*/

#endif /*I_BLOCKINGSTATE_H*/
