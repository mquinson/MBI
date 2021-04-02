////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: Creates a cartesian communicator, and tries to get information
// with Cart_get without triggering any errors or warnings
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
// iCOLL: Lacking
// TOPO: Correct
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

int main(int argc, char **argv) {

  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_Status status;

  if (size < 2) {
    printf("\033[0;31m! This test needs at 2 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  // Say hello
  printf("Hello, I am rank %d of %d processes\n", rank, size);

  // create a cartesian communicator
  MPI_Comm comm;
  int dims[2], periods[2], coords[2];
  int source, dest;
  dims[0] = 2;
  dims[1] = 1;
  periods[0] = 1;
  periods[1] = 1;

  MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &comm);
  MPI_Cart_get(comm, 2, dims, periods, coords);

  if (comm != MPI_COMM_NULL)
    MPI_Comm_free(&comm);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
