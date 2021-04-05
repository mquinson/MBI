////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Correct use of point to point communication with MPI_ANY_SOURCE
//
//				 Communication pattern:
//
// Feat_persistent: Lacking
//				   P0           P1
//				 barrier      barrier
//				 Irecv(any)   Irecv(any)
//				 send(1)      recv(0)
//				  |send(1)    send(0)
//				  |recv(1)     |send(0)
//         *NUMREPS      |recv(0)
//				 wait         *NUMREPS-1
//				              send(0)
//				              wait
//				 barrier      barrier
//
//
//// List of features
// P2P: Correct
// iP2P: Correct
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
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: noerror
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
#define NUMREPS 3

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf0[buf_size];
  int buf1[buf_size];
  int i;
  MPI_Status status;
  MPI_Request req;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (NUMREPS < 1)
    printf("\033[0;31m! Not enough repetitions !\033[0;0m\n");

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
  } else if (rank == 0) {
    memset(buf0, 0, buf_size * sizeof(int));
    MPI_Irecv(buf1, buf_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
    MPI_Send(buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD);
    for (i = 0; i < NUMREPS; i++) {
      MPI_Send(buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    }
    MPI_Wait(&req, &status);
  } else if (rank == 1) {
    MPI_Irecv(buf1, buf_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
    MPI_Recv(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Send(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    for (i = 0; i < NUMREPS - 1; i++) {
      MPI_Send(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Recv(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    MPI_Send(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Wait(&req, &status);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
