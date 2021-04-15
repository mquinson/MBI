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
 * @file CollectiveOp.h
 *       @see must::CollectiveOp.
 *
 *  @date 29.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */


#ifndef COLLECTIVEOP_H
#define COLLECTIVEOP_H

#include "CollectiveMatch.h"
#include "MustEnums.h"
#include "I_Operation.h"

using namespace gti;

namespace must
{
    /**
     * A collective operation.
     */
    class CollectiveOp : public I_Operation
    {
    public:

        /**
         * Constructor for no transfer collectives.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        CollectiveOp (
                CollectiveMatch* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm
        );

        /**
         * Constructor for send/recv to/from root.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        CollectiveOp (
                CollectiveMatch* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                bool isSend,
                int count,
                I_DatatypePersistent* type,
                I_OpPersistent *op,
                int destSource
        );

        /**
         * Constructor for send/recv to/from all ranks in comm.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        CollectiveOp (
                CollectiveMatch* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                bool isSend,
                int count,
                I_DatatypePersistent* type,
                I_OpPersistent *op
        );

        /**
         * Constructor for send/recv to/from all ranks in comm
         * with multiple counts.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        CollectiveOp (
                CollectiveMatch* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                bool isSend,
                int* counts,
                I_DatatypePersistent* type,
                I_OpPersistent *op
        );

        /**
         * Constructor for send/recv to/from all ranks in comm
         * with multiple counts and types.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        CollectiveOp (
                CollectiveMatch* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                bool isSend,
                int* counts,
                I_DatatypePersistent** types,
                I_OpPersistent *op
        );

        /**
         * Destructor.
         * Erases the datatype and comm.
         */
        ~CollectiveOp (void);

        /**
         * @see I_Operation::process
         */
        PROCESSING_RETURN process (int rank);

        /**
         * @see I_Operation::print
         */
        GTI_RETURN print (std::ostream &out);

        /**
         * Returns the comm used by this collective.
         * @return comm.
         */
        I_Comm* getComm (void);

        /**
         * Returns a copy of this ops persistent comm.
         * The user must call erase for this comm.
         * @return comm.
         */
        I_CommPersistent* getCommCopy (void);

        /**
         * Returns a reference to this ops persistent comm.
         * The user must not erase it, it will be erased when
         * this op is destructed. Use caution when using this
         * method.
         * @return comm.
         */
        I_CommPersistent* getPersistentComm (void);

        /**
         * Returns the collective id of this operation.
         * @return id.
         */
        MustCollCommType getCollId (void);

        /**
         * Creates a log event for a collective mismatch between this
         * collective and the other collective.
         */
        bool printCollectiveMismatch (CollectiveOp *other);

        /**
         * Creates a log event for a root missmatch between this
         * collective and the other collective.
         */
        bool printRootMismatch (CollectiveOp *other);

        /**
         * Creates a log event for an operation mismatch between this
         * collective and the other collective.
         */
        bool printOpMismatch (CollectiveOp *other);

        /**
         * Returns the rank (in MPI_COMM_WORLD) that issued this
         * collective.
         * @return rank.
         */
        int getIssuerRank (void);

        /**
         * Returns size of this ops comm.
         * @return size.
         */
        int getCommSize (void);

        /**
         * Returns root rank of this op, if none present
         * returns 0.
         * @return root or 0.
         */
        int getRoot (void);

        /**
         * Returns true if this operation needs a root.
         * @true if op has root.
         */
        bool hasRoot (void);

        /**
         * Returns true if this operation transfers to a single rank.
         * @return true if this is a transfer to a single task.
         */
        bool isToOne (void);

        /**
         * Checks whether this operation is the only transfer
         * needed for this rank and the given collective.
         * If so returns true, if a second operation is needed
         * it returns false.
         * @return true if only op needed for collective.
         */
        bool requiresSecondOp (void);

        /**
         * Returns true if this is a send transfer.
         * @return true if send transfer.
         */
        bool isSendTransfer(void);

        /**
         * Returns true if this is a receive transfer.
         * @return true if receive transfer.
         */
        bool isReceiveTransfer(void);

        /**
         * Returns true if this is a collective
         * without a transfer.
         */
        bool isNoTransfer(void);

        /**
         * Returns true if this op has an associated operation.
         */
        bool hasOp (void);

        /**
         * Returns the op associated with this.
         */
        I_Op* getOp (void);

        /**
         * Returns the PId of this op.
         * @return PId.
         */
        MustParallelId getPId (void);

        /**
         * Validates the type,count used by this send/receive transfer against the given
         * receive/send transfer.
         * If a mismatch is found it is logged automatically.
         *
         * @param other transfer to compare with.
         * @return true if types and counts are matching, false otherwise.
         */
        bool validateTypeMatch (CollectiveOp* other);

        /**
         * Returns true if this transfer uses multiple datatypes.
         * @return true iff multi type transfer.
         */
        bool hasMultipleTypes (void);

        /**
         * Returns true if this transfer uses multiple counts.
         * @return true iff multi count transfer.
         */
        bool hasMultipleCounts (void);

        /**
         * Returns true if this collective uses a transfer with multiple counts.
         * @return true iff multi count transfer.
         */
        bool collectiveUsesMultipleCounts (void);

        /**
         * creates a copy of this op.
         */
        CollectiveOp* copy (void);

        /**
         * Copies this op, used if this op is in the reordering.
         * @see I_Operation::copyQueuedOp.
         */
        I_Operation* copyQueuedOp (void);

    protected:
        CollectiveMatch* myMatcher;

        MustParallelId myPId; /**< Information on context that created the send/recv.*/
        MustLocationId myLId; /**< Information on context that created the send/recv.*/

        bool myIsSendTransfer;
        bool myIsReceiveTransfer;
        bool myIsToOne; /**<True if this is a transfer to a single process, if false it is a transfer to all processes in the communicator.*/

        MustCollCommType myCollId;
        I_CommPersistent *myComm;
        int myCount;
        int* myCounts; /**< If != NULL -> multiple counts. For transfers with multiple counts.*/
        I_DatatypePersistent* myType;
        I_DatatypePersistent** myTypes; /**< If != NULL -> multiple types. For transfers with multiple types.*/
        I_OpPersistent* myOp; /**< If != NULL -> has operation. Operation, for collectives that have one.*/
        int myDestSource; /**< Root for collectives that have a root, i.e. where myIsToOne == true.*/
        int myCommSize; /**< Cached value for size of the communicator.*/
        int myRank; /**< Cached value of issuer rank (derived from PId).*/

        /**
         * Initialize myCommSize.
         */
        void initializeCommSize (void);

        /**
         * Private constructor that copies the given op.
         */
        CollectiveOp (CollectiveOp* from);
    };

} /*namespace must*/

#endif /*COLLECTIVEOP_H*/
