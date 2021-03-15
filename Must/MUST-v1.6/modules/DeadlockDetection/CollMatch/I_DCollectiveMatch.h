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
 * @file I_DCollectiveMatch.h
 *       @see I_DCollectiveMatch.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_ChannelId.h"

#include "BaseIds.h"
#include "MustEnums.h"
#include "MustTypes.h"

#include <list>

#ifndef I_DCOLLECTIVEMATCH_H
#define I_DCOLLECTIVEMATCH_H

/**
 * Interface for distributed collective matching.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 * - BaseConstants
 * - CreateMessage
 * - CommTrack
 * - DatatypeTrack
 * - OpTrack
 *
 */
class I_DCollectiveMatch : public gti::I_Module
{
public:
    /*
     * Collective for implicit collectives like
     * MPI_Finalize and communicator constructors.
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param comm used for collective.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollNoTransfer (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            MustCommType comm,
            int numTasks, // counter for event aggregation
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
        )  = 0;

    /*
     * A send operation to a single process. Usually performed by the slaves
     * (and possibly also the root) of a collective with a root process.
     *
     * The following preconditioned collective transfers are mapped to this:
     * Must_Coll_Send
     * Must_Coll_Op_Send
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param comm used for collective.
     * @param count for send type repetition.
     * @param type used for the data transfer.
     * @param dest root to send to (as a rank in MPI_COMM_WORLD).
     * @param hasOp true iff this transfer uses a reduction operation.
     * @param op operation to use for reduction, only significant if hasOp == true.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollSend (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            int count,
            MustDatatypeType type,
            int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
            MustCommType comm,
            int hasOp,
            MustOpType op,
            int numTasks, // counter for event aggregation
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
        )  = 0;

    /*
     * A send operation to all tasks in the comm.
     * For MPI_Bcast this excludes the root.
     *
     * The following preconditioned collective transfers are mapped to this:
     * Must_Coll_Send_n
     * Must_Coll_Op_Send_n
     * Must_Coll_Send_buffers
     * Must_Coll_Op_Send_buffers
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param count for send type repetition.
     * @param type used for the data transfer.
     * @param commsize number of tasks to send to (size of comm).
     * @param comm used for collective.
     * @param hasOp true iff this transfer uses a reduction operation.
     * @param op operation to use for reduction, only significant if hasOp == true.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollSendN (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            int count,
            MustDatatypeType type,
            int commsize,
            MustCommType comm,
            int hasOp,
            MustOpType op,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;

    /*
     * A send operation to all tasks in the comm. Each send can use
     * a different sendcount.
     *
     * The following preconditioned collective transfers are mapped to this:
     * Must_Coll_Send_counts
     * Must_Coll_Op_Send_counts
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param counts for send type repetition.
     * @param type used for the data transfer.
     * @param commsize number of tasks to send to (size of comm).
     * @param comm used for collective.
     * @param hasOp true iff this transfer uses a reduction operation.
     * @param op operation to use for reduction, only significant if hasOp == true.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollSendCounts (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            const int counts[],
            MustDatatypeType type,
            int commsize,
            MustCommType comm,
            int hasOp,
            MustOpType op,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;

    /*
     * A send operation to all tasks in the comm. Each send can use
     * a different sendcount and type.
     *
     * The following preconditioned collective transfers are mapped to this:
     * Must_Coll_Send_types
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param counts for send type repetition.
     * @param types used for the data transfer.
     * @param commsize number of tasks to send to (size of comm).
     * @param comm used for collective.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollSendTypes (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            const int counts[],
            const MustDatatypeType types[],
            int commsize,
            MustCommType comm,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;

    /*
     * A recv from the root of the collective, only used for collectives
     * that actually have a root.
     *
     * The following preconditioned collective transfers are mapped to this:
     * PMust_Coll_Recv
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param count for recv type repetition.
     * @param type used for the data transfer.
     * @param src root to receive from (as a rank in MPI_COMM_WORLD).
     * @param comm used for collective.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollRecv (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            int count,
            MustDatatypeType type,
            int src, /*Root process to receive from as a rank in MPI_COMM_WORLD*/
            MustCommType comm,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;

    /*
     * A receive from each task in the communicator.
     *
     * The following preconditioned collective transfers are mapped to this:
     * Must_Coll_Recv_n
     * Must_Coll_Op_Recv_n
     * PMust_Coll_Recv_buffers
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param count for recv type repetition.
     * @param type used for the data transfer.
     * @param commsize number of tasks to receive from (size of comm).
     * @param comm used for collective.
     * @param hasOp true iff this transfer uses a reduction operation.
     * @param op operation to use for reduction, only significant if hasOp == true.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollRecvN (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            int count,
            MustDatatypeType type,
            int commsize,
            MustCommType comm,
            int hasOp,
            MustOpType op,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;


    /**
     * A receive from each task in the communicator. Each receive can
     * have a distinct count.
     *
     * The following preconditioned collective transfers are mapped to this:
     * PMust_Coll_Recv_counts
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param counts for recv type repetition.
     * @param type used for the data transfer.
     * @param commsize number of tasks to receive from (size of comm).
     * @param comm used for collective.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollRecvCounts (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            const int counts[],
            MustDatatypeType type,
            int commsize,
            MustCommType comm,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;

    /*
     * A receive from each task in the communicator. Each receive can
     * have a distinct count and type.
     *
     * The following preconditioned collective transfers are mapped to this:
     * PMust_Coll_Recv_types
     *
     * @param pId parallel id of calling context.
     * @param lId location id of calling context.
     * @param coll id of the collective, @see must::MustCollCommType.
     * @param counts for recv type repetition.
     * @param types used for the data transfer.
     * @param commsize number of tasks to receive from (size of comm).
     * @param comm used for collective.
     * @param numTasks used for reduction counts progress of reduction.
     * @param cId channel Id of the event.
     * @param outFinishedChannels for reductions.
     * @return @see gti::GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN CollRecvTypes (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            const int counts[],
            const MustDatatypeType types[],
            int commsize,
            MustCommType comm,
            int numTasks,
            int hasRequest,
            MustRequestType request,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
            )  = 0;

};/*class I_DCollectiveMatch*/

#endif /*I_DCOLLECTIVEMATCH_H*/
