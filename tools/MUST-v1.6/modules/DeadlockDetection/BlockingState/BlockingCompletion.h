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
 * @file BlockingCompletion.h
 *       @see must::BlockingCompletion.
 *
 *  @date 09.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#ifndef BLOCKINGCOMPLETION_H
#define BLOCKINGCOMPLETION_H

#include "I_P2PMatch.h"

#include "BlockingOp.h"

#include <vector>
#include <map>

using namespace gti;

namespace must
{
    /**
     * Information for each waited for request.
     */
    class RequestWaitInfo
    {
    public:
        RequestWaitInfo ();
        ~RequestWaitInfo ();
        bool isCompleted;
        MustRequestType request;
        P2PInfo *info; /**< Null if not requested yet, otherwise pointer to info.*/

        RequestWaitInfo copy ();
    };

    class WfgInfo
    {
    public:
        WfgInfo ();
        ArcType type; //Is primary type (AND) for mixed case
        bool isMixed;
        std::map<int, int> subNodeToReq; /**< . Maps id of sub node to request index for sub node.*/
    };

    /**
     * A blocking completion operation (e.g. MPI_Wait).
     */
    class BlockingCompletion : public BlockingOp
    {
    public:

        /**
         * Constructor.
         * For a completion with a single request.
         */
        BlockingCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request);

        /**
         * Constructor.
         * For a completion with multiple requests.
         *
         * If isForAll is true this waits for all of the communications
         * associated with the requests to complete. Otherwise, it
         * waits dor at least one of them to complete.
         */
        BlockingCompletion (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId,
                int count,
                MustRequestType *requests,
                bool isForAll,
                bool hadProcNullReqs);

        /**
         * Destructor.
         */
        virtual ~BlockingCompletion (void);

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

        /**
         * Creates a copy of this op.
         */
        BlockingOp* copy (void);

    protected:

        RequestWaitInfo myRequest;

        std::vector<RequestWaitInfo> myRequests;
        MustRequestType minReq, maxReq;
        bool myIsForAll;
        int myNumCompleted;
        int myMatchIndex; /**< Index of first completed request.*/
        bool hadAnActualCompletion; /**< True if any of the requests have actually been completed, used for determing blocking status of any/some waits.*/

        WfgInfo *myWfgInfo;

        /**
         * Compares the request to this ops requests and marks any match as
         * completed.
         * @param request to compare to this op.
         * @return true if the request ocured, false otherwise.
         */
        bool addMatchedRequest (MustRequestType request);

        /**
         * Initializes the given P2PInfo pointer to storage for a pointer to
         * the filled info.
         * @param rank of the request.
         * @param request to get info for.
         * @param outInfo to store the information pointer in.
         * @return true iff successful.
         */
        bool initRequestInfo (MustRequestType request, P2PInfo **outInfo);

        /**
         * Initializes the wfg info field for this blocking completion if not already done.
         * @return true iff successful.
         */
        bool initWfgInfo (void);

        /**
         * Drops the wfg info, necessary if an update is applied to this blocking cmpletion.
         */
        bool dropWfgInfo (void);

        /**
         * Creates from existing op.
         */
        BlockingCompletion (BlockingCompletion* other);
    };

} /*namespace must*/

#endif /*BLOCKINGOP_H*/
