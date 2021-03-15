////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: This code constructs many groups and free them
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
// GRP: Correct
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
// LOOP: Correct
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

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define ITERATIONS 10

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  int i;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Group worldgroup, newgroup[ITERATIONS];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Comm_group(MPI_COMM_WORLD, &worldgroup);

  MPI_Barrier(MPI_COMM_WORLD);

  for (i = 0; i < ITERATIONS; i++) {
    /* create groups that don't include the local rank... */
    MPI_Group_excl(worldgroup, 1, &rank, &newgroup[i]);
  }

  for (i = 0; i < ITERATIONS; i++) {
    MPI_Group_free(&newgroup[i]);
  }

  MPI_Group_free(&worldgroup);
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
