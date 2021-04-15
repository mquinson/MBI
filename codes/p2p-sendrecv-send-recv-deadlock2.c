////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: A sendrecv deadlocks here, it uses a wildcard for the receive
// part.
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
// datarace: never
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 3 ${EXE}
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
  int sbuf, rbuf;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 3) {
    printf("\033[0;31m! This test needs at least 3 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  if (rank == 0)
    MPI_Send(&sbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);

  if (rank == 1)
    MPI_Sendrecv(&sbuf, 1, MPI_INT, 2, 123, &rbuf, 1, MPI_INT, MPI_ANY_SOURCE,
                 789, MPI_COMM_WORLD, &status); /*ERROR: recv tag is wrong*/

  if (rank == 2) {
    MPI_Recv(&rbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD, &status);
    MPI_Send(&sbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
