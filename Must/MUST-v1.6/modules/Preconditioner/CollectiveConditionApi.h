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
 * @file CollectiveConditionApi.h
 *      P call definition for MUST preconditioned Request calls.
 *
 * @author Joachim Protze
 * @date 06.06.2011
 */

#include "MustEnums.h"
#include "MustTypes.h"
#include "I_DatatypeTrack.h"
#include "I_OpTrack.h"
#include "I_CommTrack.h"

#ifndef COLLECTIVECONDITIONAPI_H
#define COLLECTIVECONDITIONAPI_H

//==Function used for generate transferless collectives
inline int PMust_Coll_No_Transfer (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustCommType comm,
        int numTasks, // counter for event aggregation
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_No_TransferP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustCommType comm,
        int numTasks, // counter for event aggregation
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate single send
inline int PMust_Coll_Send (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks, // counter for event aggregation
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_SendP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate single send for Operation
inline int PMust_Coll_Op_Send (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Op_SendP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple sends
inline int PMust_Coll_Send_n (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Send_nP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple sends for Operation
inline int PMust_Coll_Op_Send_n (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Op_Send_nP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple sends with various buffers
inline int PMust_Coll_Send_buffers (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Send_buffersP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple sends for Operation
inline int PMust_Coll_Op_Send_buffers (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Op_Send_buffersP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple sends with various buffers and counts
inline int PMust_Coll_Send_counts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Send_countsP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );


//==Function used for generate multiple sends with various buffers and counts
inline int PMust_Coll_Op_Send_counts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int counts[],
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Op_Send_countsP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int counts[],
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple sends with various buffers and counts and types
inline int PMust_Coll_Send_types (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Send_typesP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate single recv
inline int PMust_Coll_Recv (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int src, /*Root process to receive from as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_RecvP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int src, /*Root process to receive from as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple recvs
inline int PMust_Coll_Recv_n (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Recv_nP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple recvs for Operation
inline int PMust_Coll_Op_Recv_n (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Op_Recv_nP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        MustOpType op,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple recvs with various buffers
inline int PMust_Coll_Recv_buffers (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Recv_buffersP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple recvs with various buffers and counts
inline int PMust_Coll_Recv_counts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Recv_countsP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate multiple recvs with various buffers and counts and types
inline int PMust_Coll_Recv_types (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Recv_typesP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request
        );

//==Function used for generate the event to check for overlapping send/recv
inline int PMust_Coll_Send_Recv (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType sendbuf,
        const int sdispls[],
        int sdisplslen,
        const int sendcounts[],
        int sendcountslen,
        const MustDatatypeType sendtypes[],
        int sendtypeslen,
        MustAddressType recvbuf,
        const int rdispls[],
        int rdisplslen,
        const int recvcounts[],
        int recvcountslen,
        const MustDatatypeType recvtypes[],
        int recvtypeslen,
        int hasRequest,
        MustRequestType request
        )  {return 0;}

typedef int (*Must_Coll_Send_RecvP) (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly gti::MustCollCommType
        MustAddressType sendbuf,
        const int sdispls[],
        int sdisplslen,
        const int sendcounts[],
        int sendcountslen,
        const MustDatatypeType sendtypes[],
        int sendtypeslen,
        MustAddressType recvbuf,
        const int rdispls[],
        int rdisplslen,
        const int recvcounts[],
        int recvcountslen,
        const MustDatatypeType recvtypes[],
        int recvtypeslen,
        int hasRequest,
        MustRequestType request
        );

#endif /* COLLECTIVECONDITIONAPI_H */
