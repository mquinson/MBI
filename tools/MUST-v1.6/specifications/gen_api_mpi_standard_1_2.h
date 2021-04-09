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
 *  MPI-calls of MPI-1.2-standard (complete C language binding.)
 *  and parts of MPI-2 standard (file I/O, one-sided communication)
 *
 *
 *  @see MPI-1.2 standard
 *  @see MPI-2 standard
 *
 *  @author Bettina Krammer, Katrin Bidmon, Matthias Mueller, Tobias Hilbrich
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
 *  $Id: mpi_standard.h 838 2008-06-13 10:44:34Z hpchimml $
 */
//=========================================================================
//                          MPI-1.2 standard
//=========================================================================

int MPI_Abort(MPI_Comm comm, int errorcode);
int MPI_Address(void* location {SINGLE_IN}, MPI_Aint* address {SINGLE_OUT});
int MPI_Allgather(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Allgatherv(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* displs {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Allreduce(void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Alltoall(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Alltoallv(void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_size:comm}, int* sdispls {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* rdispls {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype recvtype, MPI_Comm comm);
int MPI_Attr_delete(MPI_Comm comm, int keyval);
int MPI_Attr_get(MPI_Comm comm, int keyval, void* attribute_val {SINGLE_IN}, int* flag {SINGLE_OUT});
int MPI_Attr_put(MPI_Comm comm, int keyval, void* attribute_val {SINGLE_IN});
int MPI_Barrier(MPI_Comm comm );
int MPI_Bcast(void* buffer {SINGLE_IN}, int count, MPI_Datatype datatype, int root, MPI_Comm comm );
int MPI_Bsend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Bsend_init(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Buffer_attach( void* buffer {SINGLE_IN}, int size);
int MPI_Buffer_detach( void* buffer {SINGLE_IN}, int* size {SINGLE_OUT});
int MPI_Cancel(MPI_Request* request {SINGLE_IN});
int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int* coords {ARRAY_OUT|ARG:maxdims});
int MPI_Cart_create(MPI_Comm comm_old, int ndims, int* dims {ARRAY_IN|ARG:ndims}, int* periods {ARRAY_IN|ARG:ndims}, int reorder, MPI_Comm* comm_cart {SINGLE_OUT});
int MPI_Cart_get(MPI_Comm comm, int maxdims, int* dims {ARRAY_OUT|ARG:maxdims}, int* periods {ARRAY_OUT|ARG:maxdims}, int* coords {ARRAY_OUT|ARG:maxdims});
int MPI_Cart_map(MPI_Comm comm, int ndims, int* dims {ARRAY_IN|ARG:ndims}, int* periods {ARRAY_IN|ARG:ndims}, int* newrank {SINGLE_OUT});
int MPI_Cart_rank(MPI_Comm comm, int* coords {ARRAY_IN|OP:comm_size:comm}, int* rank {SINGLE_OUT});
int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int* rank_source {SINGLE_OUT}, int* rank_dest {SINGLE_OUT});
int MPI_Cart_sub(MPI_Comm comm, int* remain_dims {ARRAY_IN|OP:comm_size:comm}, MPI_Comm* newcomm {SINGLE_OUT});
int MPI_Cartdim_get(MPI_Comm comm, int* ndims {SINGLE_OUT});
int MPI_Comm_compare(MPI_Comm comm1,MPI_Comm comm2, int* result {SINGLE_OUT});
int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm* newcomm {SINGLE_OUT});
int MPI_Comm_dup(MPI_Comm comm, MPI_Comm* newcomm {SINGLE_OUT});
int MPI_Comm_free(MPI_Comm* comm {SINGLE_IO});
int MPI_Comm_group(MPI_Comm comm, MPI_Group* group {SINGLE_OUT});
int MPI_Comm_rank(MPI_Comm comm, int* rank {SINGLE_OUT});
int MPI_Comm_remote_group(MPI_Comm comm, MPI_Group* group {SINGLE_OUT});
int MPI_Comm_remote_size(MPI_Comm comm, int* size {SINGLE_OUT});
int MPI_Comm_size(MPI_Comm comm, int* size {SINGLE_OUT});
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm* newcomm {SINGLE_OUT});
int MPI_Comm_test_inter(MPI_Comm comm, int* flag {SINGLE_OUT});
int MPI_Dims_create(int nnodes, int ndims, int* dims {ARRAY_OUT|ARG:ndims});
int MPI_Errhandler_create(MPI_Handler_function* function {SINGLE_IN}, MPI_Errhandler* errhandler {SINGLE_OUT});
int MPI_Errhandler_free(MPI_Errhandler* errhandler {SINGLE_IO});
int MPI_Errhandler_get(MPI_Comm comm, MPI_Errhandler* errhandler {SINGLE_OUT});
int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
int MPI_Error_class(int errorcode, int* errorclass {SINGLE_OUT});
int MPI_Error_string(int errorcode, char* string {ARRAY_OUT|OP:deref:resultlen}, int* resultlen {SINGLE_OUT});
int MPI_Finalize(void);
int MPI_Gather(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
int MPI_Gatherv(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, int* displs {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype recvtype, int root, MPI_Comm comm);
int MPI_Get_count(MPI_Status* status {SINGLE_IN}, MPI_Datatype datatype, int* count {SINGLE_OUT});
int MPI_Get_elements(MPI_Status* status {SINGLE_IN}, MPI_Datatype datatype, int* count {SINGLE_OUT});
int MPI_Get_processor_name(char* name {ARRAY_OUT|OP:deref:resultlen}, int* resultlen {SINGLE_OUT});
int MPI_Get_version(int* version {SINGLE_OUT}, int* subversion {SINGLE_OUT});
int MPI_Graph_create(MPI_Comm comm_old, int nnodes, int* indices {ARRAY_IN|ARG:nnodes}, int* edges {ARRAY_IN|OP:graph_edge_count:indices:nnodes}, int reorder, MPI_Comm* comm_graph {SINGLE_OUT});
int MPI_Graph_get(MPI_Comm comm, int maxindices, int maxedges, int* indices {ARRAY_OUT|ARG:maxindices}, int* edges {ARRAY_OUT|ARG:maxedges});
int MPI_Graph_map(MPI_Comm comm, int nnodes, int* indices {ARRAY_IN|ARG:nnodes}, int* edges {ARRAY_IN|OP:graph_edge_count:indices:nnodes}, int* newrank {SINGLE_OUT});
int MPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors, int* neighbors {ARRAY_OUT|ARG:maxneighbors});
int MPI_Graph_neighbors_count(MPI_Comm comm, int rank, int* nneighbors {SINGLE_OUT});
int MPI_Graphdims_get(MPI_Comm comm, int* nnodes {SINGLE_OUT}, int* nedges {SINGLE_OUT});
int MPI_Group_compare(MPI_Group group1,MPI_Group group2, int* result {SINGLE_OUT});
int MPI_Group_difference(MPI_Group group1, MPI_Group group2, MPI_Group* newgroup {SINGLE_OUT});
int MPI_Group_excl(MPI_Group group, int n, int* ranks {ARRAY_IN|ARG:n}, MPI_Group* newgroup {SINGLE_OUT});
int MPI_Group_free(MPI_Group* group {SINGLE_IO});
int MPI_Group_incl(MPI_Group group, int n, int* ranks {ARRAY_IN|ARG:n}, MPI_Group* newgroup {SINGLE_OUT});
int MPI_Group_intersection(MPI_Group group1, MPI_Group group2, MPI_Group* newgroup {SINGLE_OUT});
int MPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], MPI_Group* newgroup {SINGLE_OUT});
int MPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], MPI_Group* newgroup {SINGLE_OUT});
int MPI_Group_rank(MPI_Group group, int* rank {SINGLE_OUT});
int MPI_Group_size(MPI_Group group, int* size {SINGLE_OUT});
int MPI_Group_translate_ranks(MPI_Group group1, int n, int* ranks1 {ARRAY_IN|ARG:n}, MPI_Group group2, int* ranks2 {ARRAY_OUT|ARG:n});
int MPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group* newgroup {SINGLE_OUT});
int MPI_Ibsend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Init(int* argc {SINGLE_IO}, char*** argv {SINGLE_IN});
int MPI_Init_thread(int* argc {SINGLE_IO}, char*** argv {SINGLE_IN}, int required, int* provided {SINGLE_OUT});
int MPI_Initialized(int* flag {SINGLE_OUT});
int MPI_Intercomm_create(MPI_Comm local_comm, int local_leader, MPI_Comm peer_comm, int remote_leader, int tag, MPI_Comm* newintercomm {SINGLE_OUT});
int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm* newintracomm {SINGLE_OUT});
int MPI_Iprobe(int source, int tag, MPI_Comm comm, int* flag {SINGLE_OUT}, MPI_Status* status {SINGLE_OUT});
int MPI_Irecv(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Irsend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Isend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Issend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Keyval_create(MPI_Copy_function* copy_fn {SINGLE_IN}, MPI_Delete_function* delete_fn {SINGLE_IN}, int* keyval {SINGLE_OUT}, void* extra_state {SINGLE_IN});
int MPI_Keyval_free(int* keyval {SINGLE_IO});
int MPI_Op_create(MPI_User_function* function {SINGLE_IN}, int commute, MPI_Op* op {SINGLE_OUT});
int MPI_Op_free( MPI_Op* op {SINGLE_IO});
int MPI_Pack(void* inbuf {SINGLE_IN}, int incount, MPI_Datatype datatype, void* outbuf {SINGLE_IN}, int outsize, int* position {SINGLE_IO},  MPI_Comm comm);
int MPI_Pack_size(int incount, MPI_Datatype datatype, MPI_Comm comm, int* size {SINGLE_OUT});
int MPI_Pcontrol(int level, ...);
int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status* status {SINGLE_OUT});
int MPI_Recv(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status* status {SINGLE_OUT});
int MPI_Recv_init(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Reduce(void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Reduce_scatter(void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int* recvcounts {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Request_free(MPI_Request* request {SINGLE_IO});
int MPI_Rsend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Rsend_init(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Scan(void* sendbuf {SINGLE_IN}, void* recvbuf {SINGLE_IN}, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm );
int MPI_Scatter(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
int MPI_Scatterv(void* sendbuf {SINGLE_IN}, int* sendcounts {ARRAY_IN|OP:comm_size:comm}, int* displs {ARRAY_IN|OP:comm_size:comm}, MPI_Datatype sendtype, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
int MPI_Send(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Send_init(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Sendrecv(void* sendbuf {SINGLE_IN}, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void* recvbuf {SINGLE_IN}, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status* status {SINGLE_OUT});
int MPI_Sendrecv_replace(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int sendtag, int source, int recvtag, MPI_Comm comm, MPI_Status* status {SINGLE_OUT});
int MPI_Ssend(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Ssend_init(void* buf {SINGLE_IN}, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request {SINGLE_OUT});
int MPI_Start(MPI_Request* request {SINGLE_IO});
int MPI_Startall(int count, MPI_Request* array_of_requests {ARRAY_IO|ARG:count});
int MPI_Test(MPI_Request* request {SINGLE_IO}, int* flag {SINGLE_OUT}, MPI_Status* status {SINGLE_OUT});
int MPI_Test_cancelled(MPI_Status* status {SINGLE_IN}, int* flag {SINGLE_OUT});
int MPI_Testall(int count, MPI_Request* array_of_requests {ARRAY_IO|ARG:count}, int* flag {SINGLE_OUT}, MPI_Status* array_of_statuses {ARRAY_OUT|ARG:count});
int MPI_Testany(int count, MPI_Request* array_of_requests {ARRAY_IO|ARG:count}, int* index {SINGLE_OUT}, int* flag {SINGLE_OUT}, MPI_Status* status {SINGLE_OUT});
int MPI_Testsome(int incount, MPI_Request* array_of_requests {ARRAY_IO|ARG:incount}, int* outcount {SINGLE_OUT}, int* array_of_indices {ARRAY_OUT|OP:deref:outcount}, MPI_Status* array_of_statuses {ARRAY_OUT|OP:deref:outcount});
int MPI_Topo_test(MPI_Comm comm, int* status {SINGLE_OUT});
int MPI_Type_commit(MPI_Datatype* datatype {SINGLE_IO});
int MPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype* newtype {SINGLE_OUT});
int MPI_Type_extent(MPI_Datatype datatype, MPI_Aint* extent {SINGLE_OUT});
int MPI_Type_free(MPI_Datatype* datatype {SINGLE_IO});
int MPI_Type_hindexed(int count, int* array_of_blocklengths {ARRAY_IN|ARG:count}, MPI_Aint* array_of_displacements {ARRAY_IN|ARG:count}, MPI_Datatype oldtype, MPI_Datatype* newtype {SINGLE_OUT});
int MPI_Type_hvector(int count, int blocklength, MPI_Aint stride, MPI_Datatype oldtype, MPI_Datatype* newtype {SINGLE_OUT});
int MPI_Type_indexed(int count, int* array_of_blocklengths {ARRAY_IN|ARG:count}, int* array_of_displacements {ARRAY_IN|ARG:count}, MPI_Datatype oldtype, MPI_Datatype* newtype {SINGLE_OUT});
int MPI_Type_lb(MPI_Datatype datatype, MPI_Aint* displacement {SINGLE_OUT});
int MPI_Type_size(MPI_Datatype datatype, int* size {SINGLE_OUT});
int MPI_Type_struct(int count, int* array_of_blocklengths {ARRAY_IN|ARG:count}, MPI_Aint* array_of_displacements {ARRAY_IN|ARG:count}, MPI_Datatype* array_of_types {ARRAY_IN|ARG:count}, MPI_Datatype* newtype {SINGLE_OUT});
int MPI_Type_ub(MPI_Datatype datatype, MPI_Aint* displacement {SINGLE_OUT});
int MPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype* newtype {SINGLE_OUT});
int MPI_Unpack(void* inbuf {SINGLE_IN}, int insize, int* position {SINGLE_IO}, void* outbuf {SINGLE_IN}, int outcount, MPI_Datatype datatype, MPI_Comm comm);
int MPI_Wait(MPI_Request* request {SINGLE_IO}, MPI_Status* status {SINGLE_OUT});
int MPI_Waitall(int count, MPI_Request* array_of_requests {ARRAY_IO|ARG:count}, MPI_Status* array_of_statuses {ARRAY_OUT|ARG:count});
int MPI_Waitany(int count, MPI_Request* array_of_requests {ARRAY_IO|ARG:count}, int* index {SINGLE_OUT}, MPI_Status* status {SINGLE_OUT});
int MPI_Waitsome(int incount, MPI_Request* array_of_requests {ARRAY_IO|ARG:incount}, int* outcount {SINGLE_OUT}, int* array_of_indices {ARRAY_OUT|OP:deref:outcount}, MPI_Status* array_of_statuses {ARRAY_OUT|OP:deref:outcount});
double MPI_Wtick(void);
double MPI_Wtime(void);

/*
 * prototypes for user-defined functions
 */

typedef int MPI_Copy_function(MPI_Comm oldcomm, int keyval, void* extra_state, void* attribute_val_in, void* attribute_val_out, int* flag);
typedef int MPI_Delete_function(MPI_Comm comm, int keyval, void* attribute_val, void* extra_state);
typedef void MPI_Handler_function(MPI_Comm* comm, int* flag, ...);
typedef void MPI_User_function(void* invec, void* inoutvec, int* len, MPI_Datatype* datatype);




