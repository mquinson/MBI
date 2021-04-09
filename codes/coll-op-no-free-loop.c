////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: This code constructs some MPI_Ops without freeing them
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Incorrect
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: always
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: resleak
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define OP_COUNT 5

typedef struct {
  double real, imag;
} Complex;

void myProd(void *inp, void *inoutp, int *len, MPI_Datatype *dptr) {
  int i;
  Complex c;
  Complex *in = (Complex *)inp;
  Complex *inout = (Complex *)inoutp;

  for (i = 0; i < *len; ++i) {
    c.real = inout->real * in->real - inout->imag * in->imag;
    c.imag = inout->real * in->imag + inout->imag * in->real;
    *inout = c;
    in++;
    inout++;
  }

  return;
}

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int i;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Op newop[OP_COUNT];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  for (i = 0; i < OP_COUNT; i++)
    MPI_Op_create(myProd, 1, &newop[i]);

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
