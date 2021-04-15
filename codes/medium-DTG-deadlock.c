////////////////// MPI bugs collection header //////////////////
//
// Origin: Hermes
//
// Description: This code deadlocks if P0 receives from P2 first and depending
// on the buffer mode. We force that with sleep functions.
//
// Communication pattern:
//
//   P0         P1        P2       P3      P4
//  recv(any) send(0)  recv(any) recv(1) send(2)
//  send(3)   send(3)  send(0)   recv(0)
//  recv(any)
//
// Correct situation:
//
//   P0         P1        P2       P3      P4
//  recv(any) send(0)  recv(any)         send(2)
//            send(3)             recv(1)
//  send(3)                       recv(0)
//  recv(any)          send(0)
//
// Erroneous situation:
//
//   P0         P1        P2       P3      P4
//										  recv(any)
//send(2)
//  recv(any)           send(0)
//  send(3)    send(0)            recv(1)
//
//
//// List of features
// P2P: Incorrect
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
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// various: transient
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 5 ${EXE}
   | ERROR: deadlock
  END_MBI_TESTS
*/
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <string.h>

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf_size = 10;
  int buf0[10];
  int buf1[10];
  MPI_Status status;
  MPI_Request req;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 5) {
    printf("\033[0;31m! This test needs at least 5 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  if (rank == 0) {

    MPI_Recv(&buf0, buf_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             &status);

    MPI_Send(&buf0, buf_size, MPI_INT, 3, 0, MPI_COMM_WORLD);
    MPI_Recv(&buf0, buf_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             &status);

  } else if (rank == 1) {
    // memset (buf0, 0, buf_size);

    sleep(30);

    MPI_Send(&buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD);

    MPI_Send(&buf0, buf_size, MPI_INT, 3, 0, MPI_COMM_WORLD);
  } else if (rank == 2) {
    //  memset (buf0, 0, buf_size);

    // sleep (60);
    MPI_Recv(&buf0, buf_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             &status);
    MPI_Send(&buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
  } else if (rank == 3) {
    // memset (buf0, 0, buf_size);

    // sleep (60);
    MPI_Recv(&buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    /*       MPI_Send (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD); */
  } else if (rank == 4) {
    // memset (buf0, 0, buf_size);
    MPI_Send(&buf0, buf_size, MPI_INT, 2, 0, MPI_COMM_WORLD);
    // sleep (60);

    /*       MPI_Send (buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD); */
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
