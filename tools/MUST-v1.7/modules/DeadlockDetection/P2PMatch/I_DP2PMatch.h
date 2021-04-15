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
 * @file I_DP2PMatch.h
 *       @see I_DP2PMatch.
 *
 *  @date 20.01.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_ChannelId.h"
#include "MustEnums.h"
#include "I_Comm.h"
#include "I_DP2PListener.h"

#ifndef I_DP2PMATCH_H
#define I_DP2PMATCH_H

/**
 * Interface for a point to point message matching.
 *
 * Suspension and operation queuing is implemented
 * with I_OperationReordering and is also used to impose
 * orderings relating from analyzing only feasible operations
 * for deadlock detection.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 * - BaseConstants
 * - CreateMessage  ##for printing lost messages
 * - CommTrack
 * - RequestTrack
 * - DatatypeTrack
 * - FloodControl
 * - Profiler
 *
 */
class I_DP2PMatch : public gti::I_Module
{
public:

    /**
     * Notification of MPI_Init.
     *
     * @param pId parallel Id of the call site.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN init (
            MustParallelId pId
    ) = 0;

    /**
     * Notification of a started send.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param dest to send to.
     * @param tag of the send.
     * @param comm for the send.
     * @param type for the send.
     * @param count of the send.
     * @param ts if this is an op from remote, this is
     *              the associated timestamp from the remote
     *              side, otherwise it is some value.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN send (
            MustParallelId pId,
            MustLocationId lId,
            int dest,
            int tag,
            MustCommType comm,
            MustDatatypeType type,
            int count,
            int mode,
            MustLTimeStamp ts
    ) = 0;

    /**
     * Notification of a started non-blocking send.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param dest to send to.
     * @param tag of the send.
     * @param comm for the send.
     * @param type for the send.
     * @param count of the send.
     * @param request of the isend.
     * @param ts if this is an op from remote, this is
     *              the associated timestamp from the remote
     *              side, otherwise it is some value.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN isend (
            MustParallelId pId,
            MustLocationId lId,
            int dest,
            int tag,
            MustCommType comm,
            MustDatatypeType type,
            int count,
            int mode,
            MustRequestType request,
            MustLTimeStamp ts
    ) = 0;

    /**
     * Motification of a started recv.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param source to receive from.
     * @param tag of the receive.
     * @param comm for the receive.
     * @param type for the recv.
     * @param count of the recv.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN recv (
            MustParallelId pId,
            MustLocationId lId,
            int source,
            int tag,
            MustCommType comm,
            MustDatatypeType type,
            int count) = 0;

    /**
     * Notification of a non-blocking receive.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param source to receive from.
     * @param tag of the receive.
     * @param comm for the receive.
     * @param type for the recv.
     * @param count of the recv.
     * @param request for the non-blocking receive.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN irecv (
            MustParallelId pId,
            MustLocationId lId,
            int source,
            int tag,
            MustCommType comm,
            MustDatatypeType type,
            int count,
            MustRequestType request) = 0;

    /**
     * Provides the actual source for a completed blocking wildcard receive.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param source for the receive.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN recvUpdate (
            MustParallelId pId,
            MustLocationId lId,
            int source) = 0;

    /**
     * Provides the actual source for a completed non-blocking
     * wildcard receive.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param source that actualy was used in the wildcard receive.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN irecvUpdate (
            MustParallelId pId,
            MustLocationId lId,
            int source,
            MustRequestType request) = 0;

    /**
     * Notification of a started persistent send/recv operaiton.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param request that was started.
     * @param ts if this is an op from remote, this is
     *              the associated timestamp from the remote
     *              side, otherwise it is some value.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN startPersistent (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType request,
            MustLTimeStamp ts) = 0;

    /**
     * Notification of a cancel operation.
     *
     * May require to undo matches, very hard to support.
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param request to cancel.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN cancel (
            MustParallelId pId,
            MustLocationId lId,
            MustRequestType request) = 0;

    /**
     * Prints all unmatched send/recv operations if this
     * action was issued by all connected places.
     *
     * @param thisChannel channel id of process or set of completed processes
     *               triggering this action.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN printLostMessages (
            gti::I_ChannelId *thisChannel) = 0;

    /**
     * Registers a point to point operation listener for this module.
     */
    virtual void registerListener (must::I_DP2PListener *listener) = 0;

    /**
     * Retuns true if the given world rank is currently suspended.
     * I.e. DP2PMatch waits for a wc update for this rank.
     */
    virtual bool isWorldRankSuspended (int worldRank) = 0;

    /**
     * Allows a WaitState system to tell us when a send becomes active,
     * DP2P can use this information to time a passSend to a sister node.
     * @param rank issuer rank of the send.
     * @param ts timestamp of the send.
     */
    virtual void notifySendActivated (int rank, MustLTimeStamp ts) = 0;

};/*class I_DP2PMatch*/

#endif /*I_DP2PMATCH_H*/
