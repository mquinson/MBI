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
 * @file DCollectiveOp.h
 *       @see must::DCollectiveOp.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */


#ifndef DCOLLECTIVEOP_H
#define DCOLLECTIVEOP_H

#include "MustEnums.h"
#include "MustTypes.h"
#include "BaseIds.h"
#include "I_CommTrack.h"
#include "I_DatatypeTrack.h"
#include "I_OpTrack.h"
#include "I_DOperation.h"
#include "I_DCollectiveOpProcessor.h"

using namespace gti;

namespace must
{
    /**
     * A collective operation.
     */
    class DCollectiveOp : public I_DOperation
    {
    public:

        /**
         * Constructor for no transfer collectives.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        DCollectiveOp (
                I_DCollectiveOpProcessor* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                MustCommType commHandle,
                int numRanks,
                int fromChannel,
                bool hasRequest,
                MustRequestType request
        );

        /**
         * Constructor for send/recv to/from root.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        DCollectiveOp (
                I_DCollectiveOpProcessor* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                MustCommType commHandle,
                bool isSend,
                int count,
                I_DatatypePersistent* type,
                MustDatatypeType typeHandle,
                I_OpPersistent *op,
                MustOpType opHandle,
                int destSource,
                int numRanks,
                int fromChannel,
                bool hasRequest,
                MustRequestType request
        );

        /**
         * Constructor for send/recv to/from all ranks in comm.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        DCollectiveOp (
                I_DCollectiveOpProcessor* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                MustCommType commHandle,
                bool isSend,
                int count,
                I_DatatypePersistent* type,
                MustDatatypeType typeHandle,
                I_OpPersistent *op,
                MustOpType opHandle,
                int numRanks,
                int fromChannel,
                bool hasRequest,
                MustRequestType request
        );

        /**
         * Constructor for send/recv to/from all ranks in comm
         * with multiple counts.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        DCollectiveOp (
                I_DCollectiveOpProcessor* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                MustCommType commHandle,
                bool isSend,
                int* counts,
                I_DatatypePersistent* type,
                MustDatatypeType typeHandle,
                I_OpPersistent *op,
                MustOpType opHandle,
                int numRanks,
                int fromChannel,
                bool hasRequest,
                MustRequestType request
        );

        /**
         * Constructor for send/recv to/from all ranks in comm
         * with multiple counts and types.
         *
         * Controll of comm and any other persistent handle infos is
         * given to this object.
         * Further, controll of any input arrays is given to this.
         */
        DCollectiveOp (
                I_DCollectiveOpProcessor* matcher,
                MustParallelId pId,
                MustLocationId lId,
                MustCollCommType collId,
                I_CommPersistent *comm,
                MustCommType commType,
                bool isSend,
                int* counts,
                I_DatatypePersistent** types,
                MustDatatypeType *typeHandles,
                I_OpPersistent *op,
                MustOpType opHandle,
                int numRanks,
                int fromChannel,
                bool hasRequest,
                MustRequestType request
        );

        /**
         * Destructor.
         * Erases the datatype and comm.
         */
        ~DCollectiveOp (void);

        /**
         * @see I_DOperation::process
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
         * @param other operation that violates with this operation.
         * @return true iff successful.
         */
        bool printCollectiveMismatch (DCollectiveOp *other);

        /**
         * Creates a log event for a root mismatch between this
         * collective and the other collective.
         * @param other operation that violates with this operation.
         * @return true iff successful.
         */
        bool printRootMismatch (DCollectiveOp *other);

        /**
         * Creates a log event for an operation mismatch between this
         * collective and the other collective.
         * @param other operation that violates with this operation.
         * @return true iff successful.
         */
        bool printOpMismatch (DCollectiveOp *other);

        /**
         * Creates a log event for a mismatch between a blocking and a non-blocking
         * collective event.
         * @example MPI_Barrier matched with MPI_Ibarrier.
         * @param other operation that violates with this operation.
         * @return true iff successful.
         */
        bool printBlockingNonBlockingMismatch (DCollectiveOp *other);

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
         * This is a rank in MPI_COMM_WORLD, not in the communicator!
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
         * Returns true if this is the first (of potentially two)
         * operations needed for the complete wave.
         */
        bool isFirstOpOfWave (void);

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
         * Returns the LId of this op.
         * @return LId.
         */
        MustParallelId getLId (void);

        /**
         * Validates the type,count used by this send/receive transfer against the given
         * receive/send transfer.
         * If a mismatch is found it is logged automatically.
         *
         * @param other transfer to compare with.
         * @return true if types and counts are matching, false otherwise.
         */
        bool validateTypeMatch (DCollectiveOp* other);

        /**
         * Validates the type,count used by this send/receive transfer against the given
         * receive/send transfer information.
         * If a mismatch is found it is logged automatically.
         *
         * @param pId parallel id of type match information.
         * @param lId location id of type match information.
         * @param type to check against.
         * @param count to check against.
         * @return true if types and counts are matching, false otherwise.
         */
        bool validateTypeMatch (MustParallelId pId, MustLocationId lId, I_Datatype* type, int count);

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
         * Returns how many ranks are represented by this op.
         */
        int getNumRanks (void);

        /**
         * Creates a reduced record with this operation as representant.
         * @param numRanks that this record represents.
         */
        void createReducedRecord (int numRanks, int commStride, int commOffset);

        /**
         * Returns true if intra communication is needed to correctness
         * check this collective.
         * @param true if intra comm needed to check this collective.
         */
        bool needsIntraCommToCheck (void);

        /**
         * Checks whether this op has the same counts array specified type signatues as the given operation.
         */
        bool validateCountsArrayEquality (DCollectiveOp *other);

        /**
         * Checks whether this op has the same counts array as the given operation.
         */
        bool validateJustCountsArrayEquality (DCollectiveOp *other);

        /**
         * Starts intra layer communication to transmit any part of this operations
         * information that might be needed on another place on the same layer.
         * @param waveNumber to which this operation belongs (wave number is relative to its communicator).
         */
        void intraCommunicateTypeMatchInfos (int waveNumber);

        /**
         * Returns the logical timestamp of this op (if set).
         */
        MustLTimeStamp getLTimeStamp (void);

        /**
         * Sets the logical timestamp of this op.
         */
        void setLTimeStamp (MustLTimeStamp ts);

        /**
         * Returns true if this op has a request.
         * @return true iff present.
         */
        bool hasRequest (void);

        /**
         * Returns the request of this operation, only a valid query if
         * hasRequest() == true.
         * @return request of operation.
         */
        MustRequestType getRequest (void);

    protected:
        I_DCollectiveOpProcessor* myMatcher;

        MustParallelId myPId; /**< Information on context that created the send/recv.*/
        MustLocationId myLId; /**< Information on context that created the send/recv.*/

        bool myIsSendTransfer;
        bool myIsReceiveTransfer;
        bool myIsToOne; /**<True if this is a transfer to a single process, if false it is a transfer to all processes in the communicator.*/

        MustCollCommType myCollId;
        I_CommPersistent *myComm;
        MustCommType myCommHandle;
        int myCount;
        int* myCounts; /**< If != NULL -> multiple counts. For transfers with multiple counts.*/
        I_DatatypePersistent* myType;
        MustDatatypeType myTypeHandle;
        I_DatatypePersistent** myTypes; /**< If != NULL -> multiple types. For transfers with multiple types.*/
        MustDatatypeType *myTypeHandles;
        I_OpPersistent* myOp; /**< If != NULL -> has operation. Operation, for collectives that have one.*/
        MustOpType myOpHandle;
        int myDestSource; /**< Root for collectives that have a root, i.e. where myIsToOne == true (as rank in MPI_COMM_WORLD!).*/
        int myCommSize; /**< Cached value for size of the communicator.*/
        int myRank; /**< Cached value of issuer rank (derived from PId).*/

        int myNumRanks; /**< Number of ranks represented by this operation.*/

        int myFromChannel; /**< Channel from which this node received this op.*/

        MustLTimeStamp myTS;

        bool myHasRequest;
        MustRequestType myRequest;


        /**
         * Initialize myCommSize.
         */
        void initializeCommSize (void);

        /**
         * Internal implementation to match types.
         */
        bool matchTypes (
                MustParallelId sendPId,
                MustLocationId sendLId,
                bool sendIsSend,
                int sendCount,
                I_Datatype* sendType,
                MustParallelId receivePId,
                MustLocationId receiveLId,
                bool receiveIsReceive,
                int receiveCount,
                I_Datatype* receiveType);
    };
} /*namespace must*/

#endif /*DCOLLECTIVEOP_H*/
