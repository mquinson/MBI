////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Create many types, failing to free some
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
// DATA: Incorrect
// OP: Lacking
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: always
// livelock: never
// datarace: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | ERROR: resleak
  END_MBI_TESTS
*/
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define ITERATIONS 10
#define TYPES_PER_ITERATION 3
#define TYPES_LOST_PER_ITERATION 1
#define TYPES_TO_COMMIT 1

#define buf_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int i, j;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Datatype newtype[TYPES_PER_ITERATION];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  for (i = 0; i < ITERATIONS; i++) {
    for (j = 0; j < TYPES_PER_ITERATION; j++) {
      MPI_Type_contiguous(128, MPI_INT, &newtype[j]);

      if (j >= TYPES_PER_ITERATION - TYPES_TO_COMMIT) {
        MPI_Type_commit(&newtype[j]);
      }

      if (j < TYPES_PER_ITERATION - TYPES_LOST_PER_ITERATION) {
        MPI_Type_free(&newtype[j]);
      }
    }

    if (((i % (ITERATIONS / 10)) == 0) && (rank == 0))
      printf("iteration %d completed\n", i);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
