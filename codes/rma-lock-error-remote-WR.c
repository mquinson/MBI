////////////////// MPI bugs collection header //////////////////
//
// Origin: Nasty-MPI
//
// Description: Unsynchronized Put-Get sequence
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
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
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | ERROR: datarace
  END_MBI_TESTS
*/
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <assert.h>
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
  int X = -1; // Window buffer
  int s = 10; // Local buffer
  int r = 0;  // Local buffer

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

  MPI_Win_create(&X, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD,
                 &win);

  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, (rank + 1) % nprocs, 0, win);

  MPI_Put(&s, 1, MPI_INT, (rank + 1) % nprocs, 0, 1, MPI_INT, win);
  MPI_Get(&r, 1, MPI_INT, (rank + 1) % nprocs, 0, 1, MPI_INT, win);

  MPI_Win_unlock((rank + 1) % nprocs, win);

  // assert(r==10);

  MPI_Win_free(&win);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}