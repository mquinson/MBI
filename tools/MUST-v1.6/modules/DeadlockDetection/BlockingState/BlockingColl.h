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
 * @file BlockingColl.h
 *       @see must::BlockingColl.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#ifndef BLOCKINGCOLL_H
#define BLOCKINGCOLL_H

#include "I_Comm.h"
#include "BlockingOp.h"

using namespace gti;

namespace must
{
    /**
     * A blocking collective operation.
     */
    class BlockingColl : public BlockingOp
    {
    public:

        /**
         * Constructor for any Blocking state.
         */
        BlockingColl (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm);

        /**
         * Destructor.
         */
        virtual ~BlockingColl (void);

        /**
         * @see I_Operation::process
         */
        PROCESSING_RETURN process (int rank);

        /**
         * @see I_Operation::print
         */
        GTI_RETURN print (std::ostream &out);

        /**
         * @see BlockingOp::offerMatchedSend.
         */
        bool offerMatchedSend (bool hasRequest, MustRequestType request);

        /**
         * @see BlockingOp::offerMatchedReceive.
         */
        bool offerMatchedReceive (bool hasRequest, MustRequestType request);

        /**
         * @see BlockingOp::offerMatchedCollective.
         */
        bool offerMatchedCollective (void);

        /**
         * @see BlockingOp::canComplete.
         */
        bool canComplete (void);

        /**
         * @see BlockingOp::isMixedOp.
         */
        bool isMixedOp (void);

        /**
         * @see BlockingOp::getWaitType.
         */
        ArcType getWaitType (void);

        /**
         * @see BlockingOp::mixedOpGetNumSubNodes.
         */
        int mixedOpGetNumSubNodes (void);

        /**
         * @see BlockingOp::getWaitedForRanks.
         */
        std::list<int> getWaitedForRanks (
                std::list<std::string> *outLabels,
                std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *pReferences,
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * @see BlockingOp::getSubNodeWaitedForRanks.
         */
        std::list<int> getSubNodeWaitedForRanks (
                int subId,
                std::string *outLabel,
                bool *outHasReference,
                MustParallelId *outPId,
                MustLocationId *outLId,
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * @see BlockingOp::isMatchingColl
         */
        bool isMatchingColl (MustCollCommType collId, I_Comm *comm);

        /**
         * Creates a copy of this op.
         */
        BlockingOp* copy (void);

        /**
         * @see BlockingOp::getUsedComms
         */
        std::list<I_Comm*> getUsedComms (void);

        /**
         * @see BlockingOp::getUsedComms
         */
        bool isCollective (void);

    protected:
        MustCollCommType myCollId;
        bool myIsCompleted;
        I_CommPersistent *myComm;

        /**
         * Constructor from existing operation.
         */
        BlockingColl (BlockingColl *other);
    };

} /*namespace must*/

#endif /*BLOCKINGCOLL_H*/
