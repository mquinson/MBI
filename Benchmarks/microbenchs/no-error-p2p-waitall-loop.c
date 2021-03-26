////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description:  There is no lost message in this test. We call multiple isends and irecvs.
//
//// List of features
// P2P: Lacking
// iP2P: Correct
// PERS: Lacking
// COLL: Lacking  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
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

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int size1[5];
  MPI_Status stats[10];
  MPI_Request reqs[10];
  int i;

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

  if (rank == 0) {
    for (i = 0; i < 5; i++) {
      MPI_Isend(&nprocs, 1, MPI_INT, 1, i, MPI_COMM_WORLD, &(reqs[i]));
    }

    for (i = 5; i < 10; i++) {
      MPI_Irecv(&(size1[i - 5]), 1, MPI_INT, 1, i, MPI_COMM_WORLD, &(reqs[i]));
    }
  }

  if (rank == 1) {
    for (i = 0; i < 5; i++) {
      MPI_Irecv(&(size1[i]), 1, MPI_INT, 0, 4 - i, MPI_COMM_WORLD, &(reqs[i]));
    }

    for (i = 5; i < 10; i++) {
      MPI_Isend(&nprocs, 1, MPI_INT, 0, 14 - i, MPI_COMM_WORLD, &(reqs[i]));
    }
  }

  MPI_Waitall(10, reqs, stats);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
