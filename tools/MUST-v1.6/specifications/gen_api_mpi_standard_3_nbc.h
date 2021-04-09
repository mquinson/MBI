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
 *  @file
 *
 *  MPI-calls of MPI-3 non-blocking collectives (and some other related calls).

 *  @author Tobias Hilbrich
 *
 * \note For Fortran we need additional information for the arguments.
 * 		 That is necessary for the handle conversion for MPI-Implementations like
 * 		 OpenMPI.
 * 		 We need for all MPI-Handles of pointer type information how the argument
 * 		 is used. That means we have to know whether it is:
 * 			* an out single value
 * 			* an in-out single value
 * 			* an array for input purpoeses
 * 			* an out array for output purposses
 * 			* an in-out array
 * 		For the arrays we additionally need to know their size !
 * 		We have to denote all this in the argument name, so we will append at the
 * 		end of these argument names:
 *          * IGNORE not added to trace records
 * 			* SINGLE_IN for an in single value (this is often used for requests)
 * 			* SINGLE_OUT for an out single value
 * 			* SINGLE_IO for an in-out single value
 * 			* ARRAY_IN_sizeargument an array for input purpoeses
 *          * ARRAY_IN_SIZE_OF_commargument use comm size for size of array, commargument is the comm
 * 			* ARRAY_OUT_sizeargument an out array for output purposses
 * 			* ARRAY_IO_sizeargument an in-out array
 * 		Where "sizeargument" is the fixed numeric size or the argument speci-
 * 		fing the array size.
 * 		Otherwise we would have to create the wrapper manually.
 *
 */

int MPI_Iallgather(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Iallgatherv(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* displs {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Iallreduce(const void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ialltoall(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ialltoallv(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_size:comm}, int* sdispls {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* rdispls {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ialltoallw(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_size:comm}, int* sdispls {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype* sendtypes {ARRAY_IN|OP:comm_size:comm}, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* rdispls {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype* recvtypes {ARRAY_IN|OP:comm_size:comm}, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ibarrier(MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ibcast(void* buffer {SINGLE_IN}, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Igather(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Igatherv(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* displs {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ireduce(const void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ireduce_scatter(const void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Ireduce_scatter_block(const void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Iscan(const void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Iexscan(const void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Iscatter(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Iscatterv(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_size:comm}, int* displs {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});

int MPI_Neighbor_allgather(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Ineighbor_allgather(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Neighbor_allgatherv(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_indegree:comm}, int* displs {ARRAY_IN|OP:comm_indegree:comm}, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Ineighbor_allgatherv(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_indegree:comm}, int* displs {ARRAY_IN|OP:comm_indegree:comm}, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Neighbor_alltoall(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Ineighbor_alltoall(const void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Neighbor_alltoallv(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_outdegree:comm}, int* sdispls {ARRAY_IN|OP:comm_outdegree:comm},  MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_indegree:comm}, int* rdispls {ARRAY_IN|OP:comm_indegree:comm}, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Ineighbor_alltoallv(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_outdegree:comm}, int* sdispls {ARRAY_IN|OP:comm_outdegree:comm}, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_indegree:comm}, int* rdispls {ARRAY_IN|OP:comm_indegree:comm}, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Neighbor_alltoallw(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_outdegree:comm}, MPI_Aint* sdispls {ARRAY_IN|OP:comm_outdegree:comm}, MPI_Datatype* sendtypes {ARRAY_IN|OP:comm_outdegree:comm}, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_indegree:comm}, MPI_Aint* rdispls {ARRAY_IN|OP:comm_indegree:comm}, MPI_Datatype* recvtypes {ARRAY_IN|OP:comm_indegree:comm}, MPI_Comm comm);
int MPI_Ineighbor_alltoallw(const void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_outdegree:comm}, MPI_Aint* sdispls {ARRAY_IN|OP:comm_outdegree:comm}, MPI_Datatype* sendtypes {ARRAY_IN|OP:comm_outdegree:comm}, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_indegree:comm}, MPI_Aint* rdispls {ARRAY_IN|OP:comm_indegree:comm}, MPI_Datatype* recvtypes {ARRAY_IN|OP:comm_indegree:comm}, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});

int MPI_Reduce_local(const void* inbuf {SINGLE_IN}, void* inoutbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op);
