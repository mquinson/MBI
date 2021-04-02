////////////////// MPI bugs collection header //////////////////
//
// Origin: Nasty-MPI
//
// Description: This is a potential correction of rma-lock-error-local-WW.c
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Correct
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// LOOP: Lacking
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: noerror
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
  int X;   // Window buffer
  int buf; // Local buffer

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  buf = 0;
  X = rank;

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  MPI_Win_create(&X, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD,
                 &win);

  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, (rank + 1) % nprocs, 0, win);

  buf = 1;
  MPI_Get(&buf, 1, MPI_INT, (rank + 1) % nprocs, 0, 1, MPI_INT, win);

  MPI_Win_unlock((rank + 1) % nprocs, win);

  MPI_Win_free(&win);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
