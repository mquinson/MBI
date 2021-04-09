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
 * @file I_CollectiveCondition.h
 *       @see I_CollectiveCondition.
 *
 *  @date 08.06.2011
 *  @author Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#ifndef I_COLLECTIVECONDITION_H
#define I_COLLECTIVECONDITION_H

/**
 * Interface for providing wildcard receive updates
 * to the lost message detector (I_LostMessage.h).
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CommTrack
 */
class I_CollectiveCondition : public gti::I_Module
{
public:

    /**
     * Preconditioner for collective calls that perform no
     * communication (finalize and comm constructors).
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param coll collective id for this.
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN noTransfer (
            MustParallelId pId,
            MustLocationId lId,
            int coll, // formerly gti::MustCollCommType
            MustCommType comm,
            int hasRequest,
            MustRequestType request
        ) = 0;

    /**
     * Preconditioner for MPI_Gather
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcount repetitions of sendtype
     * @param sendtype datatype for send
     * @param recvbuf buffer for communication (just root)
     * @param recvcount repetitions of recvtype (just root)
     * @param recvtype datatype for recv (just root)
     * @param root collecting node
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN gather (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            int sendcount,
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            int recvcount,
            MustDatatypeType recvtype,
            int root,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Gatherv
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcount repetitions of sendtype
     * @param sendtype datatype for send
     * @param recvbuf buffer for communication (just root)
     * @param recvcounts[] repetitions of recvtype (just root)
     * @param displs[] displacements for recv (just root)
     * @param recvtype datatype for recv (just root)
     * @param root collecting node
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN gatherv (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            int sendcount,
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            const int recvcounts[],
            const int displs[],
            MustDatatypeType recvtype,
            int root,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Reduce
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param recvbuf buffer for communication (just root)
     * @param count repetitions of type
     * @param datatype datatype for send
     * @param op operation for reduction
     * @param root collecting node
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reduce (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustAddressType recvbuf,
            int count,
            MustDatatypeType datatype,
            MustOpType op,
            int root,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Bcast
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param buffer for communication
     * @param count repetitions of sendtype
     * @param datatype datatype for send
     * @param root distributing node
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN bcast (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType buffer,
            int count,
            MustDatatypeType datatype,
            int root,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Scatter
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication (just root)
     * @param sendcount repetitions of sendtype (just root)
     * @param sendtype datatype for send (just root)
     * @param recvbuf buffer for communication
     * @param recvcount repetitions of recvtype
     * @param recvtype datatype for recv
     * @param root distributing node
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN scatter (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            int sendcount,
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            int recvcount,
            MustDatatypeType recvtype,
            int root,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Scatterv
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication (just root)
     * @param sendcounts[] repetitions of sendtype (just root)
     * @param displs[] displacements for send (just root)
     * @param sendtype datatype for send (just root)
     * @param recvbuf buffer for communication
     * @param recvcount repetitions of recvtype
     * @param recvtype datatype for recv
     * @param root distributing node
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN scatterv (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            const int sendcounts[],
            const int displs[],
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            int recvcount,
            MustDatatypeType recvtype,
            int root,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Allgather
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcount repetitions of sendtype
     * @param sendtype datatype for send
     * @param recvbuf buffer for communication
     * @param recvcount repetitions of recvtype
     * @param recvtype datatype for recv
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN allgather (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            int sendcount,
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            int recvcount,
            MustDatatypeType recvtype,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Allgatherv
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcount repetitions of sendtype
     * @param sendtype datatype for send
     * @param recvbuf buffer for communication
     * @param recvcounts[] repetitions of recvtype
     * @param displs[] displacements for recv
     * @param recvtype datatype for recv
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN allgatherv (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            int sendcount,
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            const int recvcounts[],
            const int displs[],
            MustDatatypeType recvtype,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Alltoall
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcount repetitions of sendtype
     * @param sendtype datatype for send
     * @param recvbuf buffer for communication
     * @param recvcount repetitions of recvtype
     * @param recvtype datatype for recv
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN alltoall (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            int sendcount,
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            int recvcount,
            MustDatatypeType recvtype,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Alltoallv
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcounts[] repetitions of sendtype
     * @param sdispls[] displacements for send
     * @param sendtype datatype for send
     * @param recvbuf buffer for communication
     * @param recvcounts[] repetitions of recvtype
     * @param rdispls[] displacements for recv
     * @param recvtype datatype for recv
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN alltoallv (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            const int sendcounts[],
            const int sdispls[],
            MustDatatypeType sendtype,
            MustAddressType recvbuf,
            const int recvcounts[],
            const int rdispls[],
            MustDatatypeType recvtype,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Alltoallw
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param sendcounts[] repetitions of sendtype
     * @param sdispls[] displacements for send
     * @param sendtypes[] datatypes for send
     * @param recvbuf buffer for communication
     * @param recvcounts[] repetitions of recvtype
     * @param rdispls[] displacements for recv
     * @param recvtypes[] datatypes for recv
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN alltoallw (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            const int sendcounts[],
            const int sdispls[],
            const MustDatatypeType sendtypes[],
            MustAddressType recvbuf,
            const int recvcounts[],
            const int rdispls[],
            const MustDatatypeType recvtypes[],
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Allreduce
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param recvbuf buffer for communication
     * @param count repetitions of sendtype
     * @param datatype datatype for send
     * @param op operation for reduction
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN allreduce (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustAddressType recvbuf,
            int count,
            MustDatatypeType datatype,
            MustOpType op,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Reduce_scatter
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param recvbuf buffer for communication
     * @param recvcounts[] repetitions of datatype
     * @param datatype datatype for communication
     * @param op operation for reduction
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reduce_scatter (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustAddressType recvbuf,
            const int recvcounts[],
            MustDatatypeType datatype,
            MustOpType op,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Reduce_scatter_block
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param recvbuf buffer for communication
     * @param recvcount repetitions of datatype
     * @param datatype datatype for communication
     * @param op operation for reduction
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reduce_scatter_block (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustAddressType recvbuf,
            int recvcount,
            MustDatatypeType datatype,
            MustOpType op,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Scan
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param recvbuf buffer for communication (just root)
     * @param count repetitions of sendtype
     * @param datatype datatype for send
     * @param op operation for reduction
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN scan (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustAddressType recvbuf,
            int count,
            MustDatatypeType datatype,
            MustOpType op,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

    /**
     * Preconditioner for MPI_Exscan
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param sendbuf buffer for communication
     * @param recvbuf buffer for communication (just root)
     * @param count repetitions of sendtype
     * @param datatype datatype for send
     * @param op operation for reduction
     * @param comm communicator for communication
     * @param hasRequest true if this collective has an associated request that is used to complete it, false otherwise.
     * @param request if hasRequest==true then this carries the request, otherwise content of this argument is unspecified.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN exscan (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustAddressType recvbuf,
            int count,
            MustDatatypeType datatype,
            MustOpType op,
            MustCommType comm,
            int hasRequest,
            MustRequestType request
    ) = 0;

};/*class I_CollectiveCondition*/

#endif /*I_COLLECTIVECONDITION_H*/
