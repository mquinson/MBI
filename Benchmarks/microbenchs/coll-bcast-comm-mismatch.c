////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Mismatch communicators in MPI_Bcast. A deadlock occurs with
// nprocs = 3
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
// COM: Correct
// GRP: Lacking
// DATA: Incorrect
// OP: Lacking
// LOOP:  Lacking
// SP: Correct
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: deadlock
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define buf_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Comm inverted_comm;
  int bcast_rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (nprocs != 3) {
    printf("\033[0;31m! This test requires exactly 3 processes !\033[0;0m\n");
  } else {
    /* create invertedMPI_COMM_WORLDunicator... */
    MPI_Comm_split(MPI_COMM_WORLD, 0, nprocs - rank, &inverted_comm);

    if (rank == 1) {
      MPI_Bcast(&rank, 1, MPI_INT, 1, inverted_comm);
      MPI_Bcast(&bcast_rank, 1, MPI_INT, 2, MPI_COMM_WORLD);
    } else if (rank == 2) {
      MPI_Bcast(&rank, 1, MPI_INT, 2, MPI_COMM_WORLD);
      MPI_Bcast(&bcast_rank, 1, MPI_INT, 1, inverted_comm);
    } else {
      MPI_Bcast(&bcast_rank, 1, MPI_INT, 2, MPI_COMM_WORLD);
      MPI_Bcast(&bcast_rank, 1, MPI_INT, 1, inverted_comm);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
