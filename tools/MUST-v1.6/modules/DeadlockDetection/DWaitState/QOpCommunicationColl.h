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
 * @file QOpCommunicationColl.h
 *       @see must::QOpCommunicationColl.
 *
 *  @date 01.03.2013
 *  @author Tobias Hilbrich
 */

#ifndef QOPCOMMUNICATIONCOLL_H
#define QOPCOMMUNICATIONCOLL_H

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#include "QOpCommunication.h"

using namespace gti;

namespace must
{
    /**
     * Information to link matching collectives.
     */
    class QCollectiveMatchInfo
    {
    public:
        /**
         * Constructor.
         * @param numRanksInComm amount of ranks that participate in this collective (for this TBON node).
         *
         * Sets reference count of this object to 1, if ref count becomes 0 it frees itself.
         */
        QCollectiveMatchInfo (
            int numRanksInComm);

        /**
         * Increments reference count.
         */
        int incRefCount (void);

        /**
         * Decreases reference count, potentially deletes the object.
         */
        int erase (void);

        /**
         * Adds the given rank as having activated its matching collective operation.
         */
        void addAsActive (int rank);

        /**
         * Returns true if all matching collective operations joined the collective.
         */
        bool allActive (void);

        /**
         * Returns number of active ranks.
         */
        int getNumActive (void);

        /**
         * Returns number of ranks in comm on TBON node.
         */
        int getNumRanksInComm (void);

    private:
        /**
         * Destructor.
         */
        ~QCollectiveMatchInfo (void);
        int myRefCount;

        int myNumRanksInComm; /**< Number of ranks attached to this head for the comm of the collective.*/
        int myNumActive; /**< Current count of active ops for this collective.*/
        std::list<int> myJoinedRanks; /**< Ranks that are active in this collective.*/
    };

    /**
     * A collective communication operation (Either blocking or non-blocking).
     */
    class QOpCommunicationColl : public QOpCommunication
    {
    public:

        /**
         * Constructor for any communication operation.
         * @see QOpCommunication::QOpCommunication
         * @param collType collective type.
         * @param waveNumberInComm wave number within this communicator.
         */
        QOpCommunicationColl (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                I_CommPersistent *comm,
                MustCollCommType collType,
                MustLTimeStamp waveNumberInComm
        );

        /**
         * @see QOp::printVariablesAsLabelString
         */
        virtual std::string printVariablesAsLabelString (void);

        /**
         * @see QOp::asOpCommunicationColl
         */
        virtual QOpCommunicationColl* asOpCommunicationColl (void);

        /**
         * Sets the given match info as for this collective.
         * Increments the reference count of the match info by one.
         * Decrements the reference count of the match info in its destructor.
         */
        void setMatchInfo (QCollectiveMatchInfo* info);

        /**
         * Notifies this op of a CollectiveActiveAcknowledge
         */
        void notifyActiveAcknowledge (void);

        /**
         * Returns true if this waits for an acknowledge for the given communicator information
         */
        bool waitsForAcknowledge (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize);

        /**
         * Returns true if this is the collective for MPI_Finalize
         */
        bool isFinalize (void);

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
         * @see QOp::forwardWaitForInformation
         */
        virtual void forwardWaitForInformation (
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * @see QOpCommunication::isMatchedWithActiveOps
         */
        virtual bool isMatchedWithActiveOps (void);

        /**
         * @see QOpCommunication::forwardThisOpsWaitForInformation
         */
        virtual void forwardThisOpsWaitForInformation (
                int subIdToUse,
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * Forwards the Nbc background information for this op.
         * Automatically checks whether the operation is still
         * uncompleted and whether it actually is an NBC.
         * If either is violated this becomes a no-op.
         */
        virtual void handleNbcBackgroundForwarding (void);

    protected:
        MustCollCommType myCollType;
        MustLTimeStamp myWaveNumberInComm;
        QCollectiveMatchInfo* myMatchInfo;
        bool myActiveAndAddedToLocalMatch; /**< True if this op became active and was added to the active count of the match info (to avoid adding it twice).*/
        bool myGotActiveAcknowledge; /**< If true we sent out a CollectiveActive request and received an CollectiveActiveAcknowledge in return, i.e. we unblock now!.*/

        /**
         * Destructor.
         */
        virtual ~QOpCommunicationColl (void);
    };

} /*namespace must*/

#endif /*QOPCOMMUNICATIONCOLL_H*/
