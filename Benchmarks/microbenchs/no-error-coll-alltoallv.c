////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: All processes execute an MPI_Alltoallv with matching and valid arguments
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
// HYB: Lacking
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
// compliance: never
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

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int *sbuf = NULL, *rbuf = NULL, *scounts = NULL, *rcounts = NULL, *sdispls = NULL, *rdispls = NULL, i, root;

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

  sbuf    = (int*)malloc(sizeof(int) * nprocs * 2);
  rbuf    = (int*)malloc(sizeof(int) * nprocs * 2);
  scounts = (int*)malloc(sizeof(int) * nprocs);
  rcounts = (int*)malloc(sizeof(int) * nprocs);
  sdispls = (int*)malloc(sizeof(int) * nprocs);
  rdispls = (int*)malloc(sizeof(int) * nprocs);

  for (i = 0; i < nprocs; i++) {
    scounts[i] = 2; // Mathias corrected this to 1, why?
    rcounts[i] = 2;
    sdispls[i] = (nprocs - (i + 1)) * 2;
    rdispls[i] = i * 2;
  }

  MPI_Alltoallv(sbuf, scounts, sdispls, MPI_INT, rbuf, rcounts, rdispls, MPI_INT, MPI_COMM_WORLD);

  if (sbuf)
    free(sbuf);
  if (rbuf)
    free(rbuf);
  if (scounts)
    free(scounts);
  if (rcounts)
    free(rcounts);
  if (sdispls)
    free(sdispls);
  if (rdispls)
    free(rdispls);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
