////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: Root mismatch in Bcast. All processes execute a Bcast with a
// root of 0, except for rank 1 which uses 1 as its root.
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
// LOOP:  Lacking
// SP: Correct
//
//// List of errors
// deadlock: transient
// numstab: transient
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: various
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
  int temp = 0, root;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2  processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  root = 0;
  if (rank == 0)
    temp = 42;
  if (rank == 1)
    root = 1; /*ERROR, rank 1 uses root of 1 instead of 0*/

  MPI_Bcast(&temp, 1, MPI_INT, root, MPI_COMM_WORLD);

  printf("rank %d has temp=%d\n", rank, temp);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
