////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: This code constructs many groups without freeing some
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
// GRP: Incorrect
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
// LOOP: Incorrect
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

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define ITERATIONS 100
#define GROUPS_PER_ITERATION 3
#define GROUPS_LOST_PER_ITERATION 1

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  int i, j;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Group newgroup[GROUPS_PER_ITERATION];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  for (i = 0; i < ITERATIONS; i++) {
    for (j = 0; j < GROUPS_PER_ITERATION; j++) {
      MPI_Comm_group(MPI_COMM_WORLD, &newgroup[j]);

      if (j < GROUPS_PER_ITERATION - GROUPS_LOST_PER_ITERATION) {
        MPI_Group_free(&newgroup[j]);
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
