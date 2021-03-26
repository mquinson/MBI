////////////////// MPI bugs collection header //////////////////
//
// Description: Operator mismatch in MPI_Reduce. In this code, processes call MPI_Reduce with a different operator which causes a wrong result. This error is visible when using nprocs > 2
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
// SP: Lacking
//
//// List of errors
// deadlock: never
// numstab: transient
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// compliance: never
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

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int localsum, sum;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2)
    printf("\033[0;31m! This test needs at least 3 processes !\033[0;0m\n");

  localsum = 0;
  for (int i = 0; i <= rank; i++) {
    localsum += i;
  }
  printf("process %d has local sum of %d\n", rank, localsum);

  if (rank % 2)
    MPI_Reduce(&localsum, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  else
    MPI_Reduce(&localsum, &sum, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

  printf("process %d has sum of %d\n", rank, sum);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
