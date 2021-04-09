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
 * @file QOp.h
 *       @see must::QOp.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_Comm.h"

#include <set>
#include <list>
#include <map>

#ifndef QOP_H
#define QOP_H

namespace must
{
    /*Forward declaration*/
    class DWaitState;
    class QOpCommunicationP2P;
    class QOpCommunicationColl;

    /**
     * Any operation that we consider in our timestamp queues.
     */
    class QOp
    {
    public:

        /**
         * Constructor for any timestamped operation.
         * @param dws pointer to the distributed wait state module.
         * @param pID of this op.
         * @param lId of this op.
         * @param ts time stamp of this op.
         */
        QOp (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts
        );

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
         * Returns the timestamp of this op.
         */
        MustLTimeStamp getTimeStamp (void);

        /**
         *  Increases reference count by one.
         *
         *  (Initial reference count of a newly
         *  created object is 1)
         *
         *  @return new reference count.
         */
        int incRefCount(void);

        /**
         * Decreases reference count by one.
         * If the reference count becomes 0,
         * it deletes this object.
         * @return new reference count.
         */
        int erase ();

        /**
         * Prints information on this operation as a single dot node.
         * @param out stream to write to.
         * @param nodePrefix prefix to put in front of the choosen node name.
         * @param color of the node.
         * @return full name of the new node.
         */
        virtual std::string printAsDot (std::ofstream &out, std::string nodePrefix, std::string color);

        /**
         * Prints all variables of this operation that should be in the dot output as a label string.
         * Format must be "<key>=<value>" if multiple keys are present the must be separated
         * with "|", e.g., "pId=6|lId=5".
         */
        virtual std::string printVariablesAsLabelString (void);

        /**
         * If this is an QOpCommunicationP2P implementation it returns
         * the respective pointer to it and NULL otherwise.
         * @return pointer casted or NULL if invalid attempt.
         */
        virtual QOpCommunicationP2P* asOpCommunicationP2P (void);

        /**
         * If this is an QOpCommunicationColl implementation it returns
         * the respective pointer to it and NULL otherwise.
         * @return pointer casted or NULL if invalid attempt.
         */
        virtual QOpCommunicationColl* asOpCommunicationColl (void);

        /**
         * Notifies the op that either it just became active or that an change
         * to this or a depending operation was made. As a consequence it
         * may be called multiple times! So ops must track whether this
         * is a superfluous notify.
         *
         * If an op determines that this notify is important, i.e., a condition
         * arised that requires us to send a request event or an acknowledge
         * event, the implementation of this will create the respective event.
         *
         * Finally this must influence the return of the QOp::blocks call,
         * which may be called directly after this call.
         */
        virtual void notifyActive (void) = 0;

        /**
         * Returns true if this is a blocking call that is still waiting for some
         * condition to be met in order to unblock.
         */
        virtual bool blocks (void) = 0;

        /**
         * Returns true if this op needs to be in the trace and false otherwise.
         * The caller must guarantee that the op is already active.
         */
        virtual bool needsToBeInTrace (void) = 0;

        /**
         * Notifies the operation that it should forward all of its
         * wait-for information.
         *
         * @param commLabels to name communicators with.
         */
        virtual void forwardWaitForInformation (
                std::map<I_Comm*, std::string> &commLabels) = 0;

        /**
         * Returns a list of all communicators that are used by this operation.
         */
        virtual std::list<I_Comm*> getUsedComms (void) = 0;

        /**
         * For P2P send operations this is the node id that hosts the
         * matching receive. For collectives or receives, no ping-pongs
         * are needed. For completions this is the combination of this
         * call for all non-blocking sends.
         *
         * State of the send is irrelevant, i.e., even if a send already
         * forwarded a ReceiveActiveAcknowledge, it still returns the nodeId.
         */
        virtual std::set<int> getPingPongNodes (void);

    protected:
        DWaitState* myState;

        MustParallelId myPId;
        MustLocationId myLId;

        MustLTimeStamp myTS;

        int myRank; /**<Cached value of myPId transformed into a rank.*/

        int myRefCount;

        /**
         * Destructor.
         */
        virtual ~QOp (void);
    };

} /*namespace must*/

#endif /*QOP_H*/
