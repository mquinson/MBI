////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: Correct send/recv program
//
//			 Communication pattern:
//
//			   P0       P1
//			 send(1)  recv(0)
//
//
//// List of features
// P2P: Correct
// iP2P: Lacking
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
// HYB: Lacking
// LOOP: Lacking
// SP: Lacking
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// compliance: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: noerror
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int tag, source, destination, count;
  int buffer;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  tag         = 1234;
  source      = 0;
  destination = 1;
  count       = 1;

  if (nprocs < 2) {
    printf("\033[0;31m! This test requires at least 2 processes \033[0;0m\n");
  }

  if (rank == source) {
    buffer = 5678;
    MPI_Send(&buffer, count, MPI_INT, destination, tag, MPI_COMM_WORLD);
    printf("processor %d  sent %d\n", rank, buffer);
  }
  if (rank == destination) {
    MPI_Recv(&buffer, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
    printf("processor %d  got %d\n", rank, buffer);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
