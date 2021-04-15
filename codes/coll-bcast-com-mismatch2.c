////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Mismatch communicators in MPI_Bcast. A deadlock occurs with
// nprocs > 3
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Incorrect
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Correct
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 4 ${EXE}
   | ERROR: deadlock
  END_MBI_TESTS
*/
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
  MPI_Comm comm = MPI_COMM_WORLD;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Comm nc1;
  int dat = 1234;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(comm);

  if (rank == 0)
    printf("Creating first new comm\n");

  int color = rank % 2;
  int key = 1;
  int nrank;
  int nsize;
  MPI_Comm_split(comm, color, key, &nc1);
  MPI_Comm_size(nc1, &nsize);
  MPI_Comm_rank(nc1, &nrank);
  printf("world task %d/%d maps to new comm task %d/%d\n", nprocs, rank, nsize,
         nrank);

  MPI_Barrier(comm);

  printf("Entering deadlock state.....\n");

  if (rank == 1) {
    MPI_Bcast(&dat, 1, MPI_INT, 0, nc1);
  } else {
    MPI_Bcast(&dat, 1, MPI_INT, 0, comm);
  }
  MPI_Barrier(comm);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
