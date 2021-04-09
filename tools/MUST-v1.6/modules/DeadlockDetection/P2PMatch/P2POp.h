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
 * @file P2POp.h
 *       @see must::P2POp.
 *
 *  @date 26.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Operation.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"
#include "I_DatatypeTrack.h"

#ifndef P2POP_H
#define P2POP_H

#include "P2PMatch.h"

using namespace gti;

namespace must
{
    /**
     * A send or recv operation.
     */
    class P2POp : public I_Operation
    {
    public:

        /**
         * Constructor for send/recv.
         * Control of the datatype and comm is given to this class
         * and will be erased in its destuctor.
         */
        P2POp (
                P2PMatch* matcher,
                bool isSend,
                int tag,
                int toRank,
                I_CommPersistent* comm,
                I_DatatypePersistent* datatype,
                int count,
                MustParallelId pId,
                MustLocationId lId,
                MustSendMode mode = MUST_UNKNOWN_SEND);

        /**
         * Constructor for non-blocking send/recv.
         * Control of the datatype and comm is given to this class
         * and will be erased in its destuctor.
         */
        P2POp (
                P2PMatch* matcher,
                bool isSend,
                int tag,
                int toRank,
                MustRequestType request,
                I_CommPersistent* comm,
                I_DatatypePersistent* datatype,
                int count,
                MustParallelId pId,
                MustLocationId lId,
                MustSendMode mode = MUST_UNKNOWN_SEND);

        /**
         * Destructor.
         * Erases the datatype and comm.
         */
        ~P2POp (void);

        /**
         * @see I_Operation::process
         */
        PROCESSING_RETURN process (int rank);

        /**
         * @see I_Operation::print
         */
        GTI_RETURN print (std::ostream &out);

        /**
         * Checks whether this is a wildcard receive and if so
         * it adds this operation to the queue of operations
         * that can't be processed right now but use a wild
         * card source. Takes care to not add the operation
         * multiple times.
         */
        void addToSuspendedWCOpQueue (void);

        /**
         * Uses the logger of the matcher to log this send/receive
         * as a lost message.
         */
        void logAsLost (int rank);

        /**
         * Returns the dest/source of the send/recv.
         * Returns MPI_ANY_SOURCE for wild-card
         * receives. The dest/source is already translated into
         * MPI_COMM_WORLD.
         * @return to rank.
         */
        int getToRank (void);

        /**
         * Returns the comm of this op.
         * @return comm.
         */
        I_Comm* getComm (void);

        /**
         * Returns the comm of this op as
         * its persistent pointer.
         * Do not erase the persistent comm.
         * Use getCommCopy to get a copy.
         *
         * @return comm.
         */
        I_CommPersistent* getPersistentComm (void);

        /**
         * Returns a persistent comm copied from
         * this ops comm.
         * @return comm.
         */
        I_CommPersistent* getCommCopy (void);

        /**
         * Returns the rank that issued this operation.
         */
        int getIssuerRank (void);

        /**
         * Compares the tags of this op and the other op.
         * If one is a receive and the other a send where
         * either the receive uses MPI_ANY_TAG or both
         * tags are equal, returns true. False otherwise.
         * @param other op.
         * @return true if matching.
         */
        bool matchTags (P2POp* other);

        /**
         * Returns true if this is a send/receive with an associated
         * request and false otherwise.
         * @return true if op with request.
         */
        bool hasRequest (void);

        /**
         * Returns the associated request, make sure to check
         * whether this op has any request at all, if not
         * the outcome of this operation is undefined.
         * @return associated request if present.
         */
        MustRequestType getRequest (void);

        /**
         * Used for wc receives to update their source.
         * @param newToRank new source.
         */
        void updateToSource (int newToRank);

        /**
         * Performs type matching with the given
         * operation. Must only be called if both
         * operations actually match, will cause
         * the creation of log messages in the
         * error case.
         *
         * @return true if successful, false otherwise.
         */
        bool matchTypes (P2POp* other);

        /**
         * Returns the tag of this send/recv operation.
         */
        int getTag (void);

        /**
         * Returns the send mode, only valid if this is a send.
         * @return send mode.
         */
        MustSendMode getSendMode (void);

        /**
         * Returns true if this was issued as a wc receive.
         */
        bool wasIssuedAsWcReceive (void);

        /**
         * Returns true iff this is a send.
         * @return true iff send.
         */
        bool isSend (void);

        /**
         * Returns parallel id of this op.
         * @return parallel id.
         */
        MustParallelId getPId (void);

        /**
         * Returns location id of thi op.
         * @return location id.
         */
        MustLocationId getLId (void);

        /**
         * creates a copy of this op.
         */
        P2POp* copy (void);

        /**
         * Copies this op, used if this op is in the reordering.
         * @see I_Operation::copyQueuedOp.
         */
        I_Operation* copyQueuedOp (void);

        /**
         * Returns true if this op is in the
         */
        bool isInSuspendedWcOpQueue (void);

        /**
         * For wc recvs this stores the first valid match that we encountered.
         * It may be used for deciding suspension reasons.
         * @param worldRank of the rank that provides a matching send.
         */
        void setFirstWorldRankWithValidMatch (int worldRank);

        /**
         * Returns the first world rank tha tprovided a valid match or
         * -1 if none was set yet.
         * @return rank.
         */
        int getFirstWorldRankWithValidMatch (void);
        

    protected:
        P2PMatch* myMatcher;

        bool myIsSend; /**< True if this is a send, false otherwise, only used where this information is required.*/

        int myTag; /**< Tag for this send or receive..*/
        int myRank; /**< MPI_COMM_WORLD rank that issued this op.*/
        int myToRank; /**< Translated (into MPI_COMM_WORLD): source for receives (possibly MPI_ANY_SOURCE), dest for sends (This is redundant and only used for comfort when queueing non-wildcard recvs into the wildcard queue for later processing. */
        bool wasWcReceive; /**< Stores whether this was issued as a wc receive.*/
        int myWcRecvFirstMatchingWorldRank; /**< stores the first rank in comm world of which we saw a matching send.*/

        bool myHasRequest; /**< True if this send/recv has a request.*/
        MustRequestType myRequest; /**< Request if present, no persistent info needed, requests are at least as long available as a p2p op.*/

        I_CommPersistent* myComm; /**< The communicator of the send/recv, only set if not available otherwise.*/

        I_DatatypePersistent* myType; /**< The datatype associated with the send/recv.*/
        int myCount; /**< The send/recv count.*/

        MustParallelId myPId; /**< Information on context that created the send/recv.*/
        MustLocationId myLId; /**< Information on context that created the send/recv.*/

        bool myIsInSusendedWcOpQueue; /**< Tracks whether this op was already added to the lists of suspended wildcard receives.*/

        MustSendMode mySendMode;

        /**
         * Private constructor that copies the given op.
         */
        P2POp (P2POp* from);
        
        void generateTypemismatchHtml (std::string dotFile, std::string htmlFile, std::string imageFile);
    };

} /*namespace must*/

#endif /*P2POP_H*/
