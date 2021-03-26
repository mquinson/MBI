////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: This code tries to free an unknown operation, what will cause an error.
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Incorrect  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Incorrect
// HYB: Lacking
// LOOP: Lacking
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
// Expect: noerror???
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

// user defined operation that returns the sum
void myOp(int* invec, int* inoutvec, int* len, MPI_Datatype* dtype)
{
  for (int i = 0; i < *len; i++)
    inoutvec[i] += invec[i];
}

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  // Create an Operation
  MPI_Op op, op_unknown;
  MPI_Op_create((MPI_User_function*)myOp, 0, &op);

  op_unknown = op;

  // Free an Operation
  if (op != MPI_OP_NULL)
    MPI_Op_free(&op);

  if (op_unknown != MPI_OP_NULL)
    MPI_Op_free(&op_unknown);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
