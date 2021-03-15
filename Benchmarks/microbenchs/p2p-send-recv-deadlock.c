////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Send-Recv messages. A deadlock may occur, depending on the eager-limit.
//     Eager limit: term used to describe a method of sending short messages used by many MPI implementation
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
// compliance: never
// datarace: never
//
// Test: mpirun -np 2 $zero_buffer ${EXE}
// Expect: deadlock
//
// Test: mpirun -np 4 $zero_buffer ${EXE}
// Expect: deadlock
//
// Test: mpirun -np 2 $infty_buffer ${EXE}
// Expect: noerror
//
// Test: mpirun -np 4 $infty_buffer ${EXE}
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

#define buf_size 32000

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  float data[buf_size];
  int tag = 30;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (rank == 0)
    printf("\033[0;31m WARNING: This test depends on the MPI's eager limit. Set it appropriately.\033[0;0m\n");

  int dest = (rank == nprocs - 1) ? (0) : (rank + 1);
  data[0]  = rank;
  MPI_Send(data, buf_size, MPI_FLOAT, dest, tag, MPI_COMM_WORLD);
  printf("(%d) sent data %f\n", rank, data[0]);

  int src = (rank == 0) ? (nprocs - 1) : (rank - 1);
  MPI_Recv(data, buf_size, MPI_FLOAT, src, tag, MPI_COMM_WORLD, &status);
  printf("(%d) got data %f\n", rank, data[0]);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
