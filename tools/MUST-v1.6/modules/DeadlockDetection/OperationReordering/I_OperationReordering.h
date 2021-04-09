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
 * @file I_OperationReordering.h
 *       @see MUST::I_OperationReordering.
 *
 *  @date 25.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "I_Operation.h"

#ifndef I_OPERATIONREORDERING_H
#define I_OPERATIONREORDERING_H

/**
 * Interface for modules that subscribe to a common operation
 * ordering. The ordering is influenced by blocked operations that
 * are currently not completed and by suspensions that arise if
 * some module waits for information that is guaranteed to arrive.
 *
 * Each module using this ordering querries whether an operation
 * of a certain rank can currently be processed, if so it can immediately
 * process this operation. Otherwise it must create a representation
 * for this operation and pass it to this module for queuing.
 *
 * Dependencies (order as listed):
 * - FloodControl
 */
class I_OperationReordering : public gti::I_Module
{
public:

    /**
     * Initializes the reordering with the size of MPI_COMM_WORLD
     * @param worldSize size of MPI_COMM_WORLD
     * @return @see GTI_ANALYSIS_RETURN
     */
    virtual gti::GTI_ANALYSIS_RETURN init (
            int worldSize) = 0;

    /**
     * Returns true if the given rank is not blocked
     * while no suspension is going on. I.e. ops from
     * this rank may be processed.
     * @param rank to query for.
     * @return true if open, false otherwise.
     */
    virtual bool isRankOpen (int rank) = 0;

    /**
     * Marks the given rank as blocked, meaning a blocking and
     * uncompleted operation is currently not finished on it.
     * @param rank to block.
     * @return GTI_SUCCESS if successful.
     */
    virtual gti::GTI_RETURN blockRank (int rank) = 0;

    /**
     * Marks the given rank as resumed (assumes it was blocked
     * before).
     *
     * IMPORTANT: After opening the rank this will immediately
     * cause an inspection of queued opertions. Any queued
     * operation that can be processed as a result will now be
     * processed. Thus, when calling this make sure you finished
     * processing of the current operation in order to avoid
     * working on inconsistent state.
     *
     * @param rank to resume.
     * @return GTI_SUCCESS if successful.
     */
    virtual gti::GTI_RETURN resumeRank (int rank) = 0;

    /**
     * If any module that is subject to this reordering module
     * can't process an operation to to its rank being blocked
     * or an ongoing suspension, it must enqueue its operation
     * for automatic processing once the rank is available for
     * processing.
     * @param rank of the operation.
     * @param callBack function to call when the operation
     *              can be processed.
     * @param queueData data that represents the operation which
     *               is passed to the given callback function.
     * @return GTI_SUCCESS if successful.
     */
    virtual gti::GTI_RETURN enqueueOp (int rank, must::I_Operation* operation) = 0;

    /**
     * Suspends processing of all operations of any module
     * that is subject to this reordering module. This is done
     * if some module waits for an important piece of information
     * that is guaranteed to arrive.
     * @return GTI_SUCCESS if successful.
     */
    virtual gti::GTI_RETURN suspend (void) = 0;

    /**
     * Returns true if processing is currently suspended.
     * @return true iff suspended.
     */
    virtual bool isSuspended (void) = 0;

    /**
     * Removes suspension after waited for data arrived.
     *
     * Will automatically start processing of any queued ops
     * that can now be processed. @ref resumeRank.
     *
     * @return GTI_SUCCESS if successful.
     */
    virtual gti::GTI_RETURN removeSuspension (void) = 0;

    /**
     * Returns the total number of operations in all queues.
     */
    virtual int getTotalQueueSize (void) = 0;

    /**
     * Creates a checkpoint of the current op queues.
     * The I_OperationReordering::rollback function rewinds the matching state to
     * this checkpoint.
     * There may always only be one checkpoint, when this function is called
     * it overwrites any preceding checkpoint. Checkpoints are used to execute
     * matches that can be undone, e.g. if a wildcard source is missing.
     *
     * This mechanism will only work if I_P2PMatch,
     * I_CollectiveMatch and I_BlockingState are checkpointed at the same
     * time, otherwise inconsistent state results.
     */
    virtual void checkpoint (void) = 0;

    /**
     * Rolls the op queues back to the last checkpoint.
     *
     * This mechanism will only work if I_P2PMatch,
     * I_CollectiveMatch and I_BlockingState are rolled back at the same
     * time, otherwise inconsistent state results.
     */
    virtual void rollback (void) = 0;
};

#endif /*I_OPERATIONREORDERING_H*/
