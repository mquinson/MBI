////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description:  All processes execute an MPI_Gather. Each rank sends 3 ints using a count of 3,
//  while the root only receives 2 ints from each rank by using a derived type.
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
// DATA: Correct
// OP: Lacking
// HYB: Lacking
// LOOP: Lacking
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: transient 
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: mpierr
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
  int *temp   = NULL, sendBuf[3], root;
  MPI_Status status;
  MPI_Datatype rType;

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

  if (rank == 0) {
    temp = (int*)malloc(sizeof(int) * nprocs * 2);
    MPI_Type_contiguous(2, MPI_INT, &rType);
    MPI_Type_commit(&rType);
  }

  MPI_Gather(&sendBuf, 3, MPI_INT, temp, 1, rType, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    MPI_Type_free(&rType);
    if (temp)
      free(temp);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
