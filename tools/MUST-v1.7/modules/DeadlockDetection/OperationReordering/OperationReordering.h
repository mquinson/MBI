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

#include "ModuleBase.h"
#include "I_FloodControl.h"

#include "I_OperationReordering.h"

#include <vector>
#include <deque>
#include <list>

#ifndef OPERATIONREORDERING_H
#define OPERATIONREORDERING_H

using namespace gti;

namespace must
{
    /**
     * Implementation of I_OperationReordering.
     */
    class OperationReordering : public gti::ModuleBase<OperationReordering, I_OperationReordering>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        OperationReordering (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~OperationReordering (void);

        /**
         * @see I_OperationReordering::init
         */
        GTI_ANALYSIS_RETURN init (
                int worldSize);

        /**
         * @see I_OperationReordering::isRankOpen
         */
        bool isRankOpen (int rank);

        /**
         * @see I_OperationReordering::blockRank
         */
        GTI_RETURN blockRank (int rank);

        /**
         * @see I_OperationReordering::resumeRank
         */
        GTI_RETURN resumeRank (int rank);

        /**
         * @see I_OperationReordering::enqueueOp
         */
        GTI_RETURN enqueueOp (int rank, I_Operation *op);

        /**
         * @see I_OperationReordering::suspend
         */
        GTI_RETURN suspend (void);

        /**
         * @see I_OperationReordering::isSuspended
         */
        bool isSuspended (void);

        /**
         * @see I_OperationReordering::removeSuspension
         */
        GTI_RETURN removeSuspension (void);

        /**
         * @see I_OperationReordering::getTotalQueueSize
         */
        int getTotalQueueSize (void);

        /**
         * @see I_OperationReordering::checkpoint
         */
        void checkpoint (void);

        /**
         * @see I_OperationReordering::rollback
         */
        void rollback (void);

    protected:
        std::vector<bool> myRankBlocked;
        std::vector<bool> myCheckpointRankBlocked;
        bool myIsSuspended;
        bool myCheckpointIsSuspended;
        typedef std::vector<std::deque<I_Operation*> > RankQueues;
        RankQueues myQueues;
        RankQueues myCheckpointQueues;
        bool myIsInProcessing;
        bool myCheckpointIsInProcessing;
        int myNumOps;
        int myCheckpointNumOps;

        I_FloodControl* myFloodControl;

        /*
         * The following three variables are used to count which ranks
         * have operations that can be processed (if no suspension is
         * ongoing).
         * This increases performance as otherwise it would be necessary
         * to loop over all ranks in order to determine whether any op
         * can be processes.
         */
        std::list<int> myOpenNonEmptyQueueIndices; /**< Indices of ranks that have ops to process, allocated in size of world, myRankinNonEmptyQueueList is used to guarantee that no duplicates ocurr and thus this space is sufficient.*/
        std::vector<bool> myRankinNonEmptyQueueList; /**< Stores whether some rank is in myOpenNonEmptyQueueIndices or not.*/
        std::vector<std::list<int>::iterator> myIndexInOpenNonEmptyIndices; /**< Stores for each non-blocked rank what index it has in myOpenNonEmptyQueueIndices.*/

        /**
         * Processes all queues that can currently be processed.
         */
        GTI_RETURN processQueues (void);

        /**
         * Empties the given set of rank queues.
         * @param queue to empty.
         */
        void clearQ (RankQueues *queue);
    };
}

#endif /*OPERATIONREORDERING_H*/
