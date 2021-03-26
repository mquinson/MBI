////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: All processes execute an MPI_Alltoallw with matching and valid arguments
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
// DATA: Correct
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
  int i;
  int inbuf[200], outbuf[200];

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

  for (i = 0; i < 100; i++)
    outbuf[i] = rank * 100 + i;

  MPI_Datatype contis[3];

  int recvcnts[3] = {18, 12, 6};
  int sendcnts[3] = {18, 12, 6};
  int displs[3]   = {0 * sizeof(int), 36 * sizeof(int), 72 * sizeof(int)};
  int typesize;

  for (i = 0; i < 3; i++) {
    typesize = (2 + rank + i) % 3 + 1;
    MPI_Type_contiguous(typesize, MPI_INT, contis + i);
    MPI_Type_commit(contis + i);
    recvcnts[i] = ((3 + rank - i) % 3 + 1) * 6 / typesize;
    sendcnts[i] = ((3 - rank + i) % 3 + 1) * 6 / typesize;
  }

  MPI_Alltoallw(outbuf, sendcnts, displs, contis, inbuf, recvcnts, displs, contis, MPI_COMM_WORLD);

  for (i = 0; i < 3; i++) {
    MPI_Type_free(contis + i);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
