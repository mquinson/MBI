////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Nonblocking receives immediately followed by a wait. A deadlock
// occurs if nprocs>1.
//
//// List of features
// P2P: Lacking
// iP2P: Incorrect
// PERS: Lacking
// COLL: Correct
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// LOOP: Lacking
// SP: Lacking
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: deadlock
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define buf_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf0[buf_size];
  MPI_Status status;
  MPI_Request req;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug "
           "!\033[0;0m\n");
  } else {
    int dest = (rank == nprocs - 1) ? (0) : (rank + 1);
    int src = (rank == 0) ? (nprocs - 1) : (rank - 1);
    printf("rank %d sends to %d and recv from %d\n", rank, dest, src);
    memset(buf0, rank, buf_size * sizeof(int));
    MPI_Irecv(buf0, buf_size, MPI_INT, src, 0, MPI_COMM_WORLD, &req);
    MPI_Wait(&req, &status);
    MPI_Send(buf0, buf_size, MPI_INT, dest, 0, MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
