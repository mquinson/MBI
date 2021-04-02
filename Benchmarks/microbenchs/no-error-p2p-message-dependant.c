////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: The normal execution flow, which is message dependant, prevent
// proc1
//              from being blocked in a non-matched recv.
//
//// List of features
// P2P: Correct
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
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

  if (rank == 0) {
    int x = 0;

    MPI_Send(&x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
  }
  if (rank == 1) {
    int x = 1;

    MPI_Recv(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (x == 1)
      MPI_Recv(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
