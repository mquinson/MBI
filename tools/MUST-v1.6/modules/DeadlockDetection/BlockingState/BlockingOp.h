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
 * @file BlockingOp.h
 *       @see must::BlockingOp.
 *
 *  @date 08.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#ifndef BLOCKINGOP_H
#define BLOCKINGOP_H

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_Operation.h"
#include "I_Comm.h"
#include "I_P2PMatch.h"

#include <list>
#include <string>
#include <map>

using namespace gti;

namespace must
{
    class BlockingState;

    /**
     * Any blocking operation.
     */
    class BlockingOp : public I_Operation
    {
    public:

        /**
         * Constructor for any Blocking state.
         */
        BlockingOp (
                BlockingState* state,
                MustParallelId pId,
                MustLocationId lId
        );

        /**
         * Destructor.
         */
        virtual ~BlockingOp (void);

        /**
         * Offers the completion of a send (not buffered mode for blocking sends)
         * to this blocking op.
         *
         * @param hasRequest true if the matched send has a request, false otherwise.
         * @param request of the send if present.
         * @return true if this was consumed by the blocking op and false otherwise.
         */
        virtual bool offerMatchedSend (bool hasRequest, MustRequestType request) = 0;

        /**
         * Offers the completion of a receive to this blocking op.
         *
         * @param hasRequest true if the matched receive has a request, false otherwise.
         * @param request of the receive if present.
         * @returns true if this was consumed by the blocking op and false otherwise.
         */
        virtual bool offerMatchedReceive (bool hasRequest, MustRequestType request) = 0;

        /**
         * Offers completion of a collective to this op.
         * @returns true if this was consumed by the blocking op and false otherwise.
         */
        virtual bool offerMatchedCollective (void) = 0;

        /**
         * Returns true if all waited for conditions appeared.
         */
        virtual bool canComplete (void) = 0;

        /**
         * Returns true if this needs a secondary operation.
         * Returns false as default implementation.
         * Example is MPI_Sendrecv which needs a primary
         * and a secondary operation.
         * @return true if this needs a secondary operation.
         */
        virtual bool needsSecondary (void);

        /**
         * Returns the rank that issued the operation.
         * @return rank.
         */
        int getIssuerRank (void);

        /**
         * Returns parallel id of op.
         * @return parallel id.
         */
        MustParallelId getPId (void);

        /**
         * Returns location id of op.
         * @return location id.
         */
        MustLocationId getLId (void);

        /**
         * Returns true if this is a collective op.
         * @return true if collective
         */
        virtual bool isCollective (void);

        /**
         * Returns true if this operation needs some send operation
         * from the given rank (rank in MPI_COMM_WORLD).
         * @return true iff yes.
         */
        virtual bool waitsForASend (int fromRank);

        /**
         * Returns true if this operation needs some receive operation
         * from the given rank (rank in MPI_COMM_WORLD).
         * @return true iff yes.
         */
        virtual bool waitsForAReceive (int fromRank);

        /*
         * -----------------------------------------------
         *                       Deadlock Related Stuff
         * -----------------------------------------------
         */
        /**
         * Returns true if this is a mixed op that uses
         * both AND and OR semantic. Right now the
         * only case where I am aware of such is a call
         * to MPI_Waitall with wildcard receive associated
         * requests. (and Sendrecv)
         *
         * These mixed ops use AND as the primary
         * arc type and sub nodes having OR arcs.
         *
         * So first query for this, if it returns false, query
         * for BlockingOp::getWaitType.
         *
         * @return true if this is a AND-OR op (rare).
         */
        virtual bool isMixedOp (void) = 0;

        /**
         * Returns the type of the wait of the operation.
         * Only valid for operation that are not mixed,
         * i.e. where BlockingState::isMixedOp returns
         * false.
         * @return arc type spanned by this op.
         */
        virtual ArcType getWaitType (void) = 0;

        /**
         * Returns the number of sub nodes (of the OR type)
         * needed for this mixed operation, i.e.
         * BlockingState::isMixedOp returned true.
         *
         * @return number of sub nodes.
         */
        virtual int mixedOpGetNumSubNodes (void) = 0;

        /**
         * Returns a list of ranks for which this op waits.
         * In addition the user can specify a pointer to
         * storage for a string list that is used to procide
         * arc labels for these ranks.
         *
         * For mixed ops this includes all ranks for which AND
         * semantic is used, the additional sub nodes are not
         * included here as its the users responsibility to map
         * these to additional node ids.
         *
         * @param outLabels [out] NULL or storage for arc labels.
         * @param pReferences [out] list of size of the returned list with potential pId,lId reference pairs that can be added to the label (if present).
         * @param commLabels [in] that label all active communicators with a symbol.
         * @return list of waited for ranks.
         */
        virtual std::list<int> getWaitedForRanks (
                std::list<std::string> *outLabels,
                std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *pReferences,
                std::map<I_Comm*, std::string> &commLabels) = 0;

        /**
         * Gets the ranks for which the sub node with the given id
         * waits (using OR semanctic).
         *
         * Only valid for mixed ops.
         *
         * The user can provide a pointer to storage for a label that describes
         * the subnode (not the nodes for which the sub node waits).
         *
         * @param subId of the sub node (\in 0...numSubIds-1).
         * @param outLabel NULL or storage for subId arc label.
         * @param outHasReference true if the label can be extended with a reference.
         * @param outPId reference PId if present.
         * @param outLId reference LId if present.
         * @param commLabels [in] that label all active communicators with a symbol.
         * @return list of ranks for which the sub node waits.
         */
        virtual std::list<int> getSubNodeWaitedForRanks (
                int subId,
                std::string *outLabel,
                bool *outHasReference,
                MustParallelId *outPId,
                MustLocationId *outLId,
                std::map<I_Comm*, std::string> &commLabels) = 0;

        /**
         * Checks whether this blocking op is a collective op that
         * matches with the given comm and collective id.
         * @param collId of collective
         * @param comm of collective to compare
         * @return true if matching, false otherwise.
         */
        virtual bool isMatchingColl (MustCollCommType collId, I_Comm *comm);

        /**
         * Registers a secondary operation with a primary one.
         * The pointer is only used as a reference and will be freed
         * by the caller.
         * @param secondary op to register with this primary op.
         */
        virtual void registerSecondaryOp (BlockingOp* secondary);

        /**
         * Creates a copy of this blocking op.
         */
        virtual BlockingOp* copy (void) = 0;

        /**
         * Copies this op, used if this op is in the reordering.
         * @see I_Operation::copyQueuedOp.
         */
        I_Operation* copyQueuedOp (void);

        /**
         * Returns a list of all communicators that are used by this operation.
         */
        virtual std::list<I_Comm*> getUsedComms (void) = 0;

    protected:
        BlockingState* myState;

        MustParallelId myPId;
        MustLocationId myLId;

        int myRank; /**<Cached value of myPId transformed into a rank.*/

        /**
         * Helper to add the p2p info to a wait for return.
         * @param label to use for this request.
         * @param info of request to add to wait.
         * @param outToRanks list of waited for ranks to add to.
         * @param outLabels list of labels to add to.
         * @param outReferences references to add to.
         */
        bool applyP2PToWait (
                std::string label,
                P2PInfo *info,
                std::list<int> *outToRanks,
                std::list<std::string> *outLabels,
                std::list<std::pair<bool, std::pair<MustParallelId,MustLocationId> > > *outReferences,
                std::map<I_Comm*, std::string> &commLabels);
    };

} /*namespace must*/

#endif /*BLOCKINGOP_H*/
