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
 * @file I_DWaitState.h
 *       @see I_DWaitState.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_DWAITSTATE_H
#define I_DWAITSTATE_H

/**
 * Distributed wait-state tracking that determines which processes are currently
 * blocked in which MPI calls (Assuming a pessimistic interpretation of MPI wait-for
 * semantics).
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - LocationAnalysis
 * - BaseConstants
 * - DP2PMatch
 * - DCollectiveMatchReduction
 * - CommTrack
 * - RequestTrack
 * - FloodControl
 * - Profiler
 *
 */
class I_DWaitState : public gti::I_Module
{
public:

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
     * Notified DWaitState of an acknowledge for a collective.
     * The communicator associated with the acknowledged
     * collective is described with the arguments.
     * @see PgenerateCollectiveActiveAcknowledge
     *
     * Note that if communicators do not include all ranks,
     * a node that runs DWaitState may receive acknowledges
     * for which it does not cares, i.e., which it must discard.
     *
     * @see PgenerateCollectiveActiveAcknowledge
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN collectiveAcknowledge (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize) = 0;

    /**
     * Notifies DWaitState of a ReceiveActiveRequest,
     *
     * @see PgenerateReceiveActiveRequest
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN receiveActiveRequest (
            int sendRank,
            MustLTimeStamp sendLTS,
            MustLTimeStamp receiveLTS) = 0;

    /**
     * Notifies DWaitState of a ReceiveActiveAcknowledge,
     *
     * @see PgenerateReceiveActiveAcknowledge
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN receiveActiveAcknowledge (
            int receiveRank,
            MustLTimeStamp receiveLTS) = 0;

    /**
     * Is issued when DWaitStateWfgMgr requests WFG information.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN requestWaitForInfos (void) = 0;

    /**
     * Is issued when DWaitStateWfgMgr requests that DWaitState
     * computes a consistent state.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN requestConsistentState (void) = 0;

    /**
     * Handler for a syncronization ping.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN handlePing (int fromNode, int pingsRemaining) = 0;

    /**
     * Handler for a syncronization pong.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN handlePong (int fromNode, int pingsRemaining) = 0;

};/*class I_DWaitState*/

#endif /*I_DWaitState_H*/
