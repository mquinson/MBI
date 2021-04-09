////////////////// MPI bugs collection header //////////////////
//
// Origin: MC-Checker
//
// Description: Consistency error accross processes for passive mode, window
// location accessed concurrently
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Incorrect
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: transient
//
// Test: mpirun -np 3 ${EXE}
// Expect: datarace
//
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
  MPI_Win win;
  int X;        // Window buffer
  int localbuf; // Local buffer

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  localbuf = rank;
  X = 4;

  if (nprocs < 3) {
    printf("\033[0;31m! This test needs at least 3 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  MPI_Win_create(&X, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD,
                 &win);

  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0 || rank == 2) {
    MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win);

    if (rank == 0) {
      MPI_Put(&localbuf, 1, MPI_INT, 1, 0, 1, MPI_INT, win);
    } else {
      MPI_Get(&localbuf, 1, MPI_INT, 1, 0, 1, MPI_INT, win);
    }

    MPI_Win_unlock(1, win);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Win_free(&win);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
