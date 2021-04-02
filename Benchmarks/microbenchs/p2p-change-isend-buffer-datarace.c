////////////////// MPI bugs collection header //////////////////
//
// Origin: PARCOACH
//
// Description: The buffer message is changed before the communication is
// complete.
//
//// List of features
// P2P: Lacking
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
// datarace: transient
//
// Test: mpirun -np 2 ${EXE}
// Expect: datarace
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  int message = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    message = 10;
    MPI_Request request;
    MPI_Isend(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
    printf("Sent %d to 1\n", message);
    ++message; // here is the error
    MPI_Wait(&request, MPI_STATUS_IGNORE);
  }
  if (rank == 1) {
    MPI_Recv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Recieved %d from 0\n", message);
  }

  MPI_Finalize();

  return 0;
}
