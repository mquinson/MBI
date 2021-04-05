////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: All processes execute an MPI_Scatter call with matching and
// valid arguments.
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
#include <stdlib.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int *temp = NULL, *scounts = NULL, *sdispls = NULL, i, rbuf[2], root;

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

  if (rank == 1) {
    temp = (int *)malloc(sizeof(int) * nprocs);
    scounts = (int *)malloc(sizeof(int) * nprocs);
    sdispls = (int *)malloc(sizeof(int) * nprocs);

    for (i = 0; i < nprocs; i++) {
      scounts[i] = 1;
      sdispls[i] = nprocs - (i + 1);
    }
  }

  MPI_Scatterv(temp, scounts, sdispls, MPI_INT, rbuf, 1, MPI_INT, 1,
               MPI_COMM_WORLD);

  if (rank == 1) {
    if (temp)
      free(temp);
    if (scounts)
      free(scounts);
    if (sdispls)
      free(sdispls);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
