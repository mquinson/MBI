////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: Complex example where a deadlock happens while wildcard source updates won't arrive
//
//				 Communication pattern:
//
//				     P0         P1          P2         P3
//				  Irecv(any)  Irecv(any)  Irecv(any)  Irecv(any)
//				  recv(3)     send(0)     send(1)     send(2)
//				  wait        recv(3)     recv(3)     wait
//				              wait        wait        send(0)
//				                                      send(1)
//																							send(2)
//
//// List of features
// P2P: Incorrect
// iP2P: Incorrect
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
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 4 ${EXE}
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
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf;
  MPI_Status status;
  MPI_Request request;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 4) {
    printf("\033[0;31m! This test needs at least 4 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  if (rank == 0) {
    MPI_Irecv(&nprocs, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
    // This is the error, would also have to provide a send

    MPI_Recv(&buf, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status); // This will hang

    MPI_Wait(&request, &status);
  }
  if (rank == 1) {
    MPI_Irecv(&nprocs, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
    MPI_Send(&buf, 1, MPI_INT, 0, 666, MPI_COMM_WORLD);

    MPI_Recv(&buf, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status); // This will hang

    MPI_Wait(&request, &status);
  }
  if (rank == 2) {
    MPI_Irecv(&nprocs, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
    MPI_Send(&buf, 1, MPI_INT, 1, 666, MPI_COMM_WORLD);

    MPI_Recv(&buf, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status); // This will hang

    MPI_Wait(&request, &status);
  }
  if (rank == 3) {
    MPI_Irecv(&nprocs, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
    MPI_Send(&buf, 1, MPI_INT, 2, 666, MPI_COMM_WORLD);
    MPI_Wait(&request, &status); // This will hang

    MPI_Send(&buf, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
    MPI_Send(&buf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
    MPI_Send(&buf, 1, MPI_INT, 2, 123, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
