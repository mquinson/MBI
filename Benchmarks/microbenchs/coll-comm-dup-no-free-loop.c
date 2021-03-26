////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Leak many communicators created with MPI_Comm_dup.
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
// COM: Incorrect
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// LOOP:  Incorrect
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: always
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: resleak
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define ITERATIONS 10
#define COMMS_PER_ITERATION 3
#define COMMS_LOST_PER_ITERATION 1

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  int i, j;
  MPI_Comm newcomm[COMMS_PER_ITERATION];
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  for (i = 0; i < ITERATIONS; i++) {
    for (j = 0; j < COMMS_PER_ITERATION; j++) {
      MPI_Comm_dup(MPI_COMM_WORLD, &newcomm[j]);
      MPI_Barrier(newcomm[j]);

      if (j < COMMS_PER_ITERATION - COMMS_LOST_PER_ITERATION) {
        MPI_Comm_free(&newcomm[j]);
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  MPI_Finalize();
  return 0;
}
