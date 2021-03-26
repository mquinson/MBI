////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: There is no deadlock in this test, we call correct and matching MPI calls.
//               We execute 1 to 3 calls to MPI_Waitsome depending on the interleaving.
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
// Test: mpirun -np 3 ${EXE}
// Expect: noerror
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
  int i, outcount, indices[3], num;
  MPI_Status statuses[3];
  MPI_Request requests[3];
  int buf[3];

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

  if (rank == 0) {
    MPI_Send(&(buf[0]), 1, MPI_INT, 1, 666, MPI_COMM_WORLD);
    MPI_Recv(&(buf[1]), 1, MPI_INT, 1, 666, MPI_COMM_WORLD, statuses);
  }

  if (rank == 1) {
    MPI_Irecv(&(buf[0]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[0]));
    MPI_Isend(&(buf[1]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[1]));
    MPI_Irecv(&(buf[2]), 2, MPI_INT, 2, 123, MPI_COMM_WORLD, &(requests[2]));

    i   = 0;
    num = 0;
    while (i < 3) {
      MPI_Waitsome(3, requests, &outcount, indices, statuses);
      i += outcount;
      num++;
    }
    printf("Finished up with %d calls to MPI_Waitsome.\n", num);
  }

  if (rank == 2) {
    MPI_Send(&(buf[0]), 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
