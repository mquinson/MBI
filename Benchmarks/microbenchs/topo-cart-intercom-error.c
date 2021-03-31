////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST (IsIntercommError.cpp)
//
// Description: Creates a cartesian communicator with an intercommunicator what will cause an error
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
// iCOLL: Lacking
// TOPO: Incorrect
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Correct
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// LOOP:  Lacking
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: transient
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: mpierr
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
  MPI_Comm comm, localcomm, intercomm;

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

  // create a second intracommunicator
  int color = rank % 2;
  MPI_Comm_split(MPI_COMM_WORLD, color, rank, &localcomm);

  // create a intercommunicator
  MPI_Intercomm_create(localcomm, 0, MPI_COMM_WORLD, 1 - color, 42, &intercomm);

  // create a cartesian communicator

  int dims[2], periods[2];
  int source, dest;
  dims[0]    = 1;
  dims[1]    = 1;
  periods[0] = 1;
  periods[1] = 1;

  MPI_Cart_create(intercomm, 2, dims, periods, 0, &comm);

  if (comm != MPI_COMM_NULL)
    MPI_Comm_free(&comm);

  MPI_Comm_free(&intercomm);
  MPI_Comm_free(&localcomm);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
