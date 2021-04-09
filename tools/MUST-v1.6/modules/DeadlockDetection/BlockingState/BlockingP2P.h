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
 * @file BlockingP2P.h
 *       @see must::BlockingP2P.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "BlockingOp.h"
#include "I_P2PMatch.h"

#ifndef BLOCKINGP2P_H
#define BLOCKINGP2P_H

using namespace gti;

namespace must
{
    /**
     * A blocking P2P operation.
     */
    class BlockingP2P : public BlockingOp
    {
    public:

        /**
         * Constructor for any Blocking state.
         */
        BlockingP2P (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                bool isSend,
                bool isSendRecvSend = false);

        /**
         * Destructor.
         */
        virtual ~BlockingP2P (void);

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
         * @see BlockingOp::needsSecondary.
         */
        bool needsSecondary (void);

        /**
         * Returns true if this is a send.
         * @return true if send.
         */
        bool isSend (void);

        /**
         * Returns true if this is the send part of a sendrecv.
         * @return true if send part of sendrecv.
         */
        bool isSrsend (void);

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
         * @see BlockingOp::registerSecondaryOp.
         */
        void registerSecondaryOp (BlockingOp* secondary);

        /**
         * Creates a copy of this op.
         */
        BlockingOp* copy (void);

        /**
         * @see BlockingOp::getUsedComms
         */
        std::list<I_Comm*> getUsedComms (void);

        /**
         * @see BlockingOp::waitsForASend
         */
        bool waitsForASend (int fromRank);

        /**
         * @see BlockingOp::waitsForAReceive
         */
        bool waitsForAReceive (int fromRank);

    protected:
        bool myIsSend;
        bool myIsMatched;
        bool myIsSendRecvSend;
        BlockingP2P* mySecondary;
        P2PInfo* myP2PInfo;

        /**
         * Initializes the wfg info field.
         * @return true iff successful.
         */
        bool initWfgInfo (void);

        /**
         * Drops the wfg info, necessary if an update is applied.
         */
        bool dropWfgInfo (void);

        /**
         * Creates from existing op.
         */
        BlockingP2P (BlockingP2P* other);
    };

} /*namespace must*/

#endif /*BLOCKINGOP_H*/
