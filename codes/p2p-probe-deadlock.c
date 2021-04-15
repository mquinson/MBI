////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: MPI_Probe is a blocking call that returns only after a matching
// message has been found. By calling MPI_Probe before MPI_Recv, a deadlock is
// created.
//
//				 Communication pattern:
//
//				   P0       P1
//				 barrier  barrier
//				 Probe	  Probe
//				 Send(1)  Recv(0)
//				 Recv(1)  Send(0)
//
//
//// List of features
// P2P: Incorrect
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Incorrect
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | ERROR: deadlock
  END_MBI_TESTS
*/
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
  int i, j;
  double x;
  MPI_Status status;

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
    MPI_Probe(1, 0, MPI_COMM_WORLD, &status);
    MPI_Send(&i, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    MPI_Recv(&x, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &status);
  } else if (rank == 1) {
    x = 1.0;
    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Send(&x, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
