////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: A MPI_Comm_dup function is used to create newcom which is never freed.
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
// COM: Incorrect
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
// LOOP: Lacking
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: always
// livelock: never
// compliance: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: resleak
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
  int nprocs    = -1;
  int rank      = -1;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Comm newcomm;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(comm);
  MPI_Comm_dup(comm, &newcomm);
  MPI_Barrier(comm);

  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  MPI_Finalize();
  return 0;
}
