////////////////// MPI bugs collection header //////////////////
//
// Origin: Aislinn
//
// Description: Process 0 executes a broadcast, followed by a blocking recv operation. Process one first executes a
//              blocking send that matches the recv, followed by broadcast call that matches the broadcast of process
//              0. The broadcast call on process 0 may block until process one executes the matching broadcast
//              call, so that the recv is not executed. Process one will definitely block on the send and so, in this
//              case, never executes the broadcast.
//
//                  Communication pattern:
//
//                     P0        P1        Pi
//                   Bcast     send(0)    Bcast
//                   recv(1)   Bcast
//
//
//// List of features
// P2P: Incorrect
// iP2P: Lacking
// PERS: Lacking
// COLL: Incorrect  
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
// compliance: never
// datarace: never
//
// Test: mpirun -np 3 $zero_buffer ${EXE}
// Expect: deadlock
//
// Test: mpirun -np 3 $infty_buffer ${EXE}
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
  int r;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test requires at least 2 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  int data[nprocs];
  memset(data, 0, sizeof(int) * nprocs);

  if (rank == 0) {
    MPI_Bcast(data, 1, MPI_INT, 2, MPI_COMM_WORLD);
    MPI_Recv(&rank, 1, MPI_INT, 1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  } else if (rank == 1) {
    MPI_Send(&rank, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
    MPI_Bcast(data, 1, MPI_INT, 2, MPI_COMM_WORLD);
  } else {
    MPI_Bcast(data, 1, MPI_INT, 2, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
