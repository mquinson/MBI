////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: A new communicator that reverses the order of MPI_COMM_WORLD is
// used in a Bcast and
//              a barrier, the same collectives are also executed on
//              MPI_COMM_WORLD.
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct
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
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: noerror
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int temp, root, *ranksIncl, i;
  MPI_Group gworld, ginverse;
  MPI_Comm cinverse;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  //== Create an inverse comm
  MPI_Comm_group(MPI_COMM_WORLD, &gworld);
  ranksIncl = (int *)malloc(sizeof(int) * nprocs);
  for (i = 0; i < nprocs; i++)
    ranksIncl[i] = nprocs - i - 1;
  MPI_Group_incl(gworld, nprocs, ranksIncl, &ginverse);
  MPI_Comm_create(MPI_COMM_WORLD, ginverse, &cinverse);

  // Do some colls on world and the inverse comm
  MPI_Bcast(&rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Barrier(cinverse);
  MPI_Bcast(&rank, 1, MPI_INT, 0, cinverse);
  MPI_Barrier(MPI_COMM_WORLD);

  // Clean up
  MPI_Comm_free(&cinverse);
  MPI_Group_free(&ginverse);
  MPI_Group_free(&gworld);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
