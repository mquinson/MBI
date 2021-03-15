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
 * @file BlockingRequestCompletion.h
 *       @see must::BlockingRequestCompletion.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#ifndef BLOCKINGREQUESTCOMPLETION_H
#define BLOCKINGREQUESTCOMPLETION_H

#include "BlockingOp.h"

#include <vector>

using namespace gti;

namespace must
{
    /**
     * An update on requests completed in any completion.
     */
    class BlockingRequestCompletion : public BlockingOp
    {
    public:

        /**
         * Constructor.
         */
        BlockingRequestCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request);

        /**
         * Constructor.
         */
        BlockingRequestCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                int count,
                MustRequestType* requests);

        /**
         * Destructor.
         */
        virtual ~BlockingRequestCompletion (void);

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
         * True if this is an array of requests, false otherwise.
         * @return true iff array.
         */
        bool isArray (void);

        /**
         * Returns the request of this update.
         * Only valid if this is not an array.
         * @return request.
         */
        MustRequestType getRequest (void);

        /**
         * Returns a pointer to the request array.
         * Only valid if this is a request array.
         */
        std::vector<MustRequestType>* getRequests (void);

        /**
         * Returns true if this update caries no meaningful information.
         */
        bool isInvalid (void);

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
         * Creates a copy of this op.
         */
        BlockingOp* copy (void);

        /**
         * @see BlockingOp::getUsedComms
         */
        std::list<I_Comm*> getUsedComms (void);

    protected:
        MustRequestType myRequest;
        std::vector<MustRequestType> myRequests;
        bool myIsInvalid; /**< true if this update caries no valid information*/

        /**
         * Creates from existing op.
         */
        BlockingRequestCompletion (BlockingRequestCompletion* other);
    };

} /*namespace must*/

#endif /*BLOCKINGREQUESTCOMPLETION_H*/
