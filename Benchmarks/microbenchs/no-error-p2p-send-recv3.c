////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: Correct use of send-recv communication - ring
//
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
// datarace: never
//
// Test: mpirun -np 4 ${EXE}
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

#define buf_size 128

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int i, left, right;
  int data = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  left  = (rank + nprocs - 1) % nprocs;
  right = (rank + nprocs + 1) % nprocs;
  i     = 0;
  if (rank % 2 == 0) {
    MPI_Send(&data, 0, MPI_INT, right, 0, MPI_COMM_WORLD);
    MPI_Recv(&data, 0, MPI_INT, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  } else {
    MPI_Recv(&data, 0, MPI_INT, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send(&data, 0, MPI_INT, right, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
