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
 * @file I_P2PMatch.h
 *       @see I_P2PMatch.
 *
 *  @date 26.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_ChannelId.h"
#include "MustEnums.h"
#include "I_P2PMatchListener.h"
#include "I_Comm.h"

#ifndef I_P2PMATCH_H
#define I_P2PMATCH_H

namespace must
{
    /**
     * Information on a P2P op provided to outside.
     */
    class P2PInfo
    {
    public:
        bool isSend;
        MustParallelId pId;
        MustLocationId lId;
        int target; /**< Source for receives, dest for sends, both translated into MPI_COMM_WORLD.*/
        bool isWc; /**< True if a wildcard was specified for this op (if so target is either the constant for MPI_ANY_SOURCE or it some rank which the MPI determined as a matching partner).*/
        I_Comm* comm; /**< Comm used by the op, only valid until the op is matched!*/
        MustSendMode mode; /**< If a send operation this is its mode.*/
        int tag;
    };
}

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
 * - BaseConstants
 * - CreateMessage  ##for printing lost messages
 * - CommTrack
 * - RequestTrack
 * - DatatypeTrack
 * - OperationReordering
 * - FloodControl
 *
 */
class I_P2PMatch : public gti::I_Module
{
public:

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
        int mode
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
    		MustRequestType request
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
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN startPersistent (
    		MustParallelId pId,
    		MustLocationId lId,
    		MustRequestType request) = 0;

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
     * Registers the given callback with P2PMatch.
     * The callback is triggered whenever a new match was found and its
     * processing completed. It should be used to notify any module that may
     * be interested in matches. Multiple callbacks may be registered and are
     * executed in the order in which they where registered.
     * @param listener to register.
     * @return GTI_SUCCESS if successful.
     */
    virtual gti::GTI_RETURN registerListener (must::I_P2PMatchListener *listener) = 0;

    /**
     * Retrieves information on an unmatched p2p op.
     * The op is a non-blocking op with the given request.
     *
     * Note: the first matching op is returned, there will only be
     *          a unique operation if I_OperationReordering is being
     *          correctly driven by a BlockingState, otherwise multiple
     *          operations may exist and any of these is returned.
     *
     * @param rank of the op.
     * @param request of the op.
     * @param outInfo pointer to storage to store the information in,
     *               only used if an op is found.
     * @return true if found, false otherwise.
     */
    virtual bool getP2PInfo (int rank, MustRequestType request, must::P2PInfo* outInfo) = 0;

    /**
     * Retrieves information on an unmatched p2p op.
     * The op is a blocking op of the given type (send/receive).
     *
     * Important: This ignores MPI_Bsends, which is exactly what
     *                  BlockingState whishs for. (also no unique op for
     *                  MPI_Bsends anyways)
     *
     * Note: the first matching op is returned, there will only be
     *          a unique operation if I_OperationReordering is being
     *          correctly driven by a BlockingState, otherwise multiple
     *          operations may exist and any of these is returned.
     *
     * @param rank of the op.
     * @param isSend true iff a send.
     * @param outInfo pointer to storage to store the information in,
     *               only used if an op is found.
     * @return true if found, false otherwise.
     */
    virtual bool getP2PInfo (int rank, bool isSend, must::P2PInfo* outInfo) = 0;

    /**
     * Provides a list of all active P2P operations between the two given ranks (both
     * as ranks in MPI_COMM_WORLD).
     *
     * This does not ignores MPI_Bsends!
     *
     * @param fromRank world rank of P2P issuer.
     * @param toRank world rank of P2P source/dest.
     * @param sends true if sends shall be included in the returned list.
     * @param receives true if receives shall be included in the returned list.
     * @return list of P2P infos that are valid until the individual operations are matched.
     */
    virtual std::list<must::P2PInfo> getP2PInfos (int fromRank, int toRank, bool sends, bool receives) = 0;

    /**
     * Notifies the P2P matching of a deadlock, this is used to disable printing
     * of any lost messages (as they are very likely in a deadlock case, but are
     * of no importance then).
     */
    virtual void notifyDeadlock (void) = 0;

    /**
     * Creates a checkpoint of the current matching status.
     * The I_P2PMatch::rollback function rewinds the matching state to
     * this checkpoint.
     * There may always only be one checkpoint, when this function is called
     * it overwrites any preceding checkpoint. Checkpoints are used to execute
     * matches that can be undone, e.g. if a wildcard source is missing.
     *
     * This mechanism will only work if I_OperationReordering,
     * I_CollectiveMatch and I_BlockingState are checkpointed at the same
     * time, otherwise inconsistent state results.
     */
    virtual void checkpoint (void) = 0;

    /**
     * Rolls the matching information back to the last checkpoint.
     *
     * This mechanism will only work if I_OperationReordering,
     * I_CollectiveMatch and I_BlockingState are rolled back at the same
     * time, otherwise inconsistent state results.
     */
    virtual void rollback (void) = 0;

    /**
     * Used in cases where a wildcard source update appears to not arrive,
     * e.g. if a deadlock appears without a completion call being issued.
     * @param decissionIndex index of the choice to take, starts with 0, there is always at least one.
     * @param outNumAlternatives is set to the number of available alternatives for this choice.
     * @return true if this was a valid decission, i.e. the decided receive
     *              could match something with the decission and false otherwise.
     *              If this returns false, this decission was invalid (as there was a
     *              match available, thus the suspension) so no deadlock detection must be
     *              issued, instead an immediate rollback should be done.
     */
    virtual bool decideSuspensionReason (
            int decissionIndex,
            int *outNumAlternatives) = 0;

    /**
     * Returns true if this operation can be processed
     * on this tool place, i.e. the destination of a send
     * or the source of a receive are reachable by this
     * place. Otherwise it returns false
     *
     * @param pId context of calling process.
     * @param comm used for the send/recv.
     * @param sourceDest source for receives, destination for sends.
     */
    virtual bool canOpBeProcessed (
            MustParallelId pId,
            MustCommType comm,
            int sourceDest) = 0;

    /**
     * Returns true if this operation can be processed
     * on this tool place, i.e. the destination of a send
     * or the source of a receive are reachable by this
     * place. Otherwise it returns false
     *
     * @param pId context of calling process.
     * @param comm used for the send/recv.
     * @param sourceDest source for receives, destination for sends.
     */
    virtual bool canOpBeProcessed (
            MustParallelId pId,
            must::I_Comm* comm,
            int sourceDest) = 0;

};/*class I_P2PMatch*/

#endif /*I_P2PMATCH_H*/
