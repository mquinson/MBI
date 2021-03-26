////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: Type mismatch in MPI_Gather. P0 calls MPI_Gather with MPI_INT while others call MPI_Gather with MPI_FLOAT
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
// HYB: Lacking
// LOOP: Lacking
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: transient
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: numstab
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
  int* values;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (rank == 0 || rank == 2) {
    values = (int*)malloc(sizeof(int) * nprocs);
  } else {
    values = (int*)malloc(sizeof(int));
  }

  *values = nprocs + rank;

  if (rank != 2)
    MPI_Gather(values, 1, MPI_INT, values, 1, MPI_INT, 0, MPI_COMM_WORLD);
  else
    MPI_Gather(values, 1, MPI_FLOAT, values, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

  free(values);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
