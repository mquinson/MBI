////////////////// MPI bugs collection header //////////////////
//
// Origin: Parcoach
//
// Description: Communicator mismatch. Some processes call MPI_Barrier on MPI_COMM_WORLD while others call a barrier on newcomm (communicator created with MPI_Comm_split). A deadlock occurs if nprocs > 1.
//
/// List of features
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
// DATA: Lacking
// OP: Lacking
// LOOP: Lacking
// SP: Correct
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: transient
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: deadlock
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
  MPI_Comm comm;
  MPI_Comm newcomm;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;

  MPI_Init(&argc, &argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Comm_split(comm, rank % 2, rank, &newcomm);

  if (nprocs < 2)
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug !\033[0;0m\n");

  if (rank % 3)
    MPI_Barrier(comm);
  else
    MPI_Barrier(newcomm);

  MPI_Comm_free(&comm);
  MPI_Comm_free(&newcomm);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
