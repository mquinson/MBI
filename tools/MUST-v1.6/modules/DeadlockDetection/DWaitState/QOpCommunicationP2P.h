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
 * @file QOpCommunicationP2P.h
 *       @see must::QOpCommunicationP2P.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#include "QOpCommunication.h"

#ifndef QOPCOMMUNICATIONP2P_H
#define QOPCOMMUNICATIONP2P_H

using namespace gti;

namespace must
{
    /**
     * A P2P communication operation.
     */
    class QOpCommunicationP2P : public QOpCommunication
    {
    public:

        /**
         * Constructor for any P2P communication operation.
         * @see QOpCommunication::QOpCommunication
         * @param isSend true if this is a send, receive iff false.
         * @param sourceTarget source or target of receive/send.
         * @param isWc true if this is a wildcard receive (irrespective of whether we got and applied a receive update to the sourceTarget already).
         * @param mode if this is a send, this is the send mode.
         * @param tag of this P2P op.
         */
        QOpCommunicationP2P (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                I_CommPersistent *comm,
                bool isSend,
                int sourceTarget,
                bool isWc,
                MustSendMode mode,
                int tag
        );

        /**
         * @see QOp::printVariablesAsLabelString
         */
        virtual std::string printVariablesAsLabelString (void);

        /**
         * @see QOp::asOpCommunicationP2P.
         */
        virtual QOpCommunicationP2P* asOpCommunicationP2P (void);

        /**
         * If this is a receive operation, it updates the matching state of this
         * operation.
         * @param pIdSend parallelId of the send.
         * @param sendTS logical timestamp of the send in its operation trace.
         */
        void setMatchingInformation (
                MustParallelId pIdSend,
                MustLTimeStamp sendTS);

        /**
         * True if this is a non-blocking op, false othewise.
         */
        virtual bool isNonBlockingP2P (void);

        /**
         * @see QOp::notifyActive
         */
        virtual void notifyActive (void);

        /**
         * @see QOp::blocks
         */
        virtual bool blocks (void);

        /**
         * @see QOp::needsToBeInTrace
         */
        virtual bool needsToBeInTrace (void);

        /**
         * @see QOpCommunication::isMatchedWithActiveOps
         */
        virtual bool isMatchedWithActiveOps (void);

        /**
         * Notifies the send P2P op that we got the ReceiveActiveRequest.
         * @param receiveLTS logical timestamp of the receive.
         */
        void notifyGotReceiveActiveRequest (MustLTimeStamp receiveLTS);

        /**
         * Notifies the receive P2P op that we got the ReceiveActiveAcknowledge.
         */
        void notifyGotReceiveActiveAcknowledge (void);

        /**
         * Sets this send as a sendrecv send part, i.e., it becomes non-blocking
         * (without the need for a wait and without using a request)
         */
        void setAsSendrecvSend (void);

        /**
         * Sets this send as a sendrecv receive part.
         * Associates the send part with this receive, such that this receive
         * only unblocks once that both itself and the send are matched
         * with active operations.
         *
         * The send's reference count must be increased by one before
         * calling this, since we will decrease it by one in our destructor.
         */
        void setAsSendrecvRecv (QOpCommunicationP2P* send);

        /**
         * @see QOp::forwardWaitForInformation
         */
        virtual void forwardWaitForInformation (
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * @see QOpCommunication::forwardThisOpsWaitForInformation
         */
        virtual void forwardThisOpsWaitForInformation (
                int subIdToUse,
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * @see QOp::getPingPongNodes
         */
        virtual std::set<int> getPingPongNodes (void);

        /**
         * Return true if this is a send operation.
         * @return true iff send.
         */
        bool isSend (void);

        /**
         * Returns the world rank for the source/target of the receive/send.
         * @param  pOutIsWc pointer to storage for bool, is set to true if this is a wildcard receive.
         */
        int getSourceTarget (bool *pOutIsWc);

    protected:
        bool myIsSend;
        int mySourceTarget; /**< Source for receives, dest for sends, both translated into MPI_COMM_WORLD.*/
        int myTag;

        //Only of interest for sends
        bool myGotRecvBecameActive; /**< True if we got the recv became active request for this send; Once this becomes active we need to reply an acknowledge. */
        MustSendMode myMode;
        bool mySentRecvBecameActiveAcknowledge; /**< True if myGotRecvBecameActive==true and we sent out the acknowledge to the receiver.*/
        MustLTimeStamp myTSOfMatchingReceive;
        bool myIsSendrecvSend;

        //Only of interest for receives
        bool myGotRecvMatchUpdate;
        MustLTimeStamp myTSOfMatchingSend; /**< If this is a receive, this is set to the timestamp of the send that matches this operation, once that DP2PMatch matched it.*/
        bool myIsWc; /**< True if a wildcard was specified for this op (if so mySourceTarget is either the constant for MPI_ANY_SOURCE or it is some rank which the MPI determined as a matching partner).*/
        bool mySentBecameActiveRequest; /**< True if active, myGotRecvMatchUpdate==true, and already send the ReceiveActiveRequest.*/
        bool myGotActiveAcknowledge; /**< True if mySentBecameActiveRequest==true, and received the ReceiveActiveAcknowledge.*/
        QOpCommunicationP2P* myAssociatedSend; /**< If this is a sendrecv receive, this is the send part.*/

        /**
         * Destructor.
         */
        virtual ~QOpCommunicationP2P (void);
    };

} /*namespace must*/

#endif /*QOPCOMMUNICATIONP2P_H*/
