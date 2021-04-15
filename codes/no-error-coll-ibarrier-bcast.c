////////////////// MPI bugs collection header //////////////////
//
// Origin: Parcoach
//
// Description: This code is correct as the Ibarrier and Bcast functions are
// called on different communicators
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct
// iCOLL: Correct
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Correct
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
// datarace: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | OK
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

#define buf_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  MPI_Comm comm = MPI_COMM_WORLD;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf0[buf_size];
  int buf1[buf_size];
  MPI_Request req;
  MPI_Comm dupcomm;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  memset(buf0, 0, buf_size * sizeof(int));
  memset(buf1, 1, buf_size * sizeof(int));

  MPI_Comm_dup(comm, &dupcomm);
  MPI_Barrier(comm);

  switch (rank) {
  case 0:
    MPI_Bcast(buf0, buf_size, MPI_INT, 1, dupcomm);
    MPI_Ibarrier(comm, &req);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
    break;

  default:
    MPI_Ibarrier(comm, &req);
    MPI_Bcast(buf0, buf_size, MPI_INT, 1, dupcomm);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
    break;
  }

  MPI_Comm_free(&dupcomm);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
