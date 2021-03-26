////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Correct use of MPI_Probe on any tag.
//
//// List of features
// P2P: Correct
// iP2P: Correct
// PERS: Lacking
// COLL: Correct  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Correct
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
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
  int i, j;
  double x;
  MPI_Status status;
  MPI_Request req0, req1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
  } else if (rank == 0) {
    i = 0;
    x = 1.0;
    MPI_Isend(&i, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req0);
    MPI_Isend(&x, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &req1);
    MPI_Wait(&req1, &status);
    MPI_Wait(&req0, &status);
  } else if (rank == 1) {
    for (j = 0; j < 2; j++) {
      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (status.MPI_TAG == 0)
        MPI_Recv(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
      else
        MPI_Recv(&x, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
