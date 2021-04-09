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
 * @file DP2POp.h
 *       @see must::DP2POp.
 *
 *  @date 20.01.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#include "MustTypes.h"
#include "I_DOperation.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"
#include "I_DatatypeTrack.h"

#ifndef DP2POP_H
#define DP2POP_H

#include "DP2PMatch.h"

using namespace gti;

namespace must
{
    /**
     * A send or recv operation.
     */
    class DP2POp : public I_DOperation
    {
    public:

        /**
         * Constructor for send/recv.
         * Control of the datatype and comm is given to this class
         * and will be erased in its destuctor.
         */
        DP2POp (
                DP2PMatch* matcher,
                bool isSend,
                int tag,
                int toRank,
                I_CommPersistent* comm,
                I_DatatypePersistent* datatype,
                int count,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                MustSendMode mode = MUST_UNKNOWN_SEND);

        /**
         * Constructor for non-blocking send/recv.
         * Control of the datatype and comm is given to this class
         * and will be erased in its destuctor.
         */
        DP2POp (
                DP2PMatch* matcher,
                bool isSend,
                int tag,
                int toRank,
                MustRequestType request,
                I_CommPersistent* comm,
                I_DatatypePersistent* datatype,
                int count,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                MustSendMode mode = MUST_UNKNOWN_SEND);

        /**
         * Destructor.
         * Erases the datatype and comm.
         */
        ~DP2POp (void);

        /**
         * @see I_Operation::process
         */
        PROCESSING_RETURN process (int rank);

        /**
         * @see I_Operation::print
         */
        GTI_RETURN print (std::ostream &out);

        /**
         * Uses the logger of the matcher to log this send/receive
         * as a lost message.
         */
        void logAsLost (int rank);

        /**
         * Returns the dest/source of the send/recv.
         * Returns MPI_ANY_SOURCE for wild-card
         * receives.
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
        bool matchTags (DP2POp* other);

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
        bool matchTypes (DP2POp* other);

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
        DP2POp* copy (void);

        /**
         * Returns the logical time stamp associated with this operation.
         */
        MustLTimeStamp getLTimeStamp (void);

    protected:
        DP2PMatch* myMatcher;

        bool myIsSend; /**< True if this is a send, false otherwise, only used where this information is required.*/

        int myTag; /**< Tag for this send or receive..*/
        int myRank; /**< MPI_COMM_WORLD rank that issued this op.*/
        int myToRank; /**< Translated (into MPI_COMM_WORLD): source for receives (possibly MPI_ANY_SOURCE), dest for sends (This is redundant and only used for comfort when queueing non-wildcard recvs into the wildcard queue for later processing. */
        bool wasWcReceive; /**< Stores whether this was issued as a wc receive.*/

        bool myHasRequest; /**< True if this send/recv has a request.*/
        MustRequestType myRequest; /**< Request if present, no persistent info needed, requests are at least as long available as a p2p op.*/

        I_CommPersistent* myComm; /**< The communicator of the send/recv.*/

        I_DatatypePersistent* myType; /**< The datatype associated with the send/recv.*/
        int myCount; /**< The send/recv count.*/

        MustParallelId myPId; /**< Information on context that created the send/recv.*/
        MustLocationId myLId; /**< Information on context that created the send/recv.*/

        MustSendMode mySendMode;

        MustLTimeStamp myTS; /**< Logical timestamp that a DP2PListener associated to this op, we do not really care about the timestamp, but use it to interface with the listener.*/

        /**
         * Private constructor that copies the given op.
         */
        DP2POp (DP2POp* from);

        void generateTypemismatchHtml (std::string dotFile, std::string htmlFile, std::string imageFile);
    };

} /*namespace must*/

#endif /*DP2POP_H*/
