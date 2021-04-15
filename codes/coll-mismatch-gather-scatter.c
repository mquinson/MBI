////////////////// MPI bugs collection header //////////////////
//
// Origin: Aislinn
//
// Description: Missmatch ordering of MPI_Scatter and MPI_Gather. This code
// should always deadlock.
//
//
//				 Communication pattern:
//
//				      P0         P1
//                                 Scatter()   Gather()
//				    Gather()  Scatter()
//
//
//
//
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
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
//
//// List of errors
// deadlock: always
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | ERROR: deadlock
  END_MBI_TESTS
*/
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char **argv) {

  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  int in[3], out[3];

  if (rank == 0) {
    MPI_Scatter(in, 3, MPI_INT, out, 3, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(in, 3, MPI_INT, out, 3, MPI_INT, 0, MPI_COMM_WORLD);
  }
  if (rank == 1) {
    MPI_Gather(in, 3, MPI_INT, out, 3, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(in, 3, MPI_INT, out, 3, MPI_INT, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
