////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: Complex example that tries to enforce the "deciding" mode of
// must (if queues become too big). No deadlock, just a late completion.
//
//// List of features
// P2P: Correct
// iP2P: Correct
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
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 4 ${EXE}
// Expect: noerror
//
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
  int buf, buf2, buf3, i;
  MPI_Status status;
  MPI_Request request, request2;

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

  // Irecv with match, we complete it later
  MPI_Irecv(&buf3, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
  MPI_Send(&buf, 1, MPI_INT, (rank + 1) % nprocs, 666, MPI_COMM_WORLD);

  // Lots of other comm.
  for (i = 0; i < 50000; i++) {
    MPI_Irecv(&buf2, 1, MPI_INT, (rank - 1 + nprocs) % nprocs, 666,
              MPI_COMM_WORLD, &request2);
    MPI_Send(&buf, 1, MPI_INT, (rank + 1) % nprocs, 666, MPI_COMM_WORLD);
    MPI_Wait(&request2, &status);
  }

  // Now complete and do an additional round of wild-card communication
  MPI_Wait(&request, &status);
  MPI_Irecv(&buf2, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
  MPI_Send(&buf, 1, MPI_INT, (rank + 1) % nprocs, 666, MPI_COMM_WORLD);
  MPI_Wait(&request, &status);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
