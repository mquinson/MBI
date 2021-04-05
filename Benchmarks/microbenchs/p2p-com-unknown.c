////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: Performs a send, recv with an unknown communicator in send call
//
//// List of features
// P2P: Incorrect
// iP2P: Lacking
// PERS: Lacking
// COLL: Incorrect
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
// segfault: always
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: mpierr
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

  // Enough tasks ?
  if (size < 2) {
    printf("This test needs at least 2 processes!");
    MPI_Finalize();
    return 1;
  }

  // Say hello
  printf("Hello, I am rank %d of %d processes\n", rank, size);

  MPI_Comm comm1, comm2;
  int period = 1;
  MPI_Cart_create(MPI_COMM_WORLD, 1, &size, &period, 0, &comm1);
  comm2 = comm1;
  MPI_Comm_free(&comm1);
  if (rank == 0) {
    MPI_Send(&size, 1, MPI_INT, 1, 42, comm2);
  }

  if (rank == 1) {
    MPI_Recv(&size, 1, MPI_INT, 0, 42, comm2, &status);
  }

  // Say bye bye
  printf("Signing off, rank %d\n", rank);

  MPI_Finalize();

  return 0;
}
