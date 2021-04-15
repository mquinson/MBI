////////////////// MPI bugs collection header //////////////////
//
// Origin: Parcoach
//
// Description: Collective mismatch. Some processes call 9 barriers while others
// call 10 barriers. A deadlock occurs if nprocs > 1
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
//
//// List of errors
// error_deadlock: transient
// error_numstab: never
// error_segfault: never
// error_mpierr: never
// error_resleak: never
// error_livelock: never
// error_other: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | ERROR: deadlock
  END_MBI_TESTS
*/
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int i = 1, j = 10;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2)
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug "
           "!\033[0;0m\n");

  if (rank % 2) {
    while (i < 10) {
      MPI_Barrier(MPI_COMM_WORLD);
      i++;
    }
  } else {
    while (j < 20) {
      MPI_Barrier(MPI_COMM_WORLD);
      j++;
    }
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
