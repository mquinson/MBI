////////////////// MPI bugs collection header //////////////////
//
// Origin: MPI Bug Collection suite
//
// Description: Potential deadlock that manifests if standard mode send is not buffered.
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
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: deadlock
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
  int buf0[buf_size];
  int buf1[buf_size];
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2)
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug !\033[0;0m\n");

  if (rank == 0) {
    memset(buf0, 0, buf_size * sizeof(int));
    MPI_Send(buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD);
    MPI_Recv(buf1, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
  } else if (rank == 1) {
    memset(buf1, 1, buf_size * sizeof(int));
    MPI_Send(buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Recv(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
