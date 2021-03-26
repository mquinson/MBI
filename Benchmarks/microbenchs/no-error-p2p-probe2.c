////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: Correct use of MPI_Probe and MPI_Get_count to find the size of an incomming message
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
// PROB: Correct
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
#include <stdlib.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Status status;
  int mytag, ierr, icount, j, *i;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
  }

  mytag = 123;
  if (rank == 0) {
    j      = 200;
    icount = 1;
    ierr   = MPI_Send(&j, icount, MPI_INT, 1, mytag, MPI_COMM_WORLD);
  }
  if (rank == 1) {
    ierr = MPI_Probe(0, mytag, MPI_COMM_WORLD, &status);
    ierr = MPI_Get_count(&status, MPI_INT, &icount);
    i    = (int*)malloc(icount * sizeof(int));
    printf("getting %d\n", icount);
    ierr = MPI_Recv(i, icount, MPI_INT, 0, mytag, MPI_COMM_WORLD, &status);
    printf("i= ");
    for (j = 0; j < icount; j++)
      printf("%d ", i[j]);
    printf("\n");
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
