////////////////// MPI bugs collection header //////////////////
//
// Origin: Parcoach
//
// Description: All processes call MPI_Barrier if the number of processes >1
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

typedef struct mystruct {
  int rank;
  int nprocs;
} mystruct_t;

int main(int argc, char** argv)
{
  mystruct_t* S = malloc(sizeof(mystruct_t));
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &S->nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &S->rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", S->rank, processor_name);

  if (S->nprocs > 1)
    MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", S->rank);
  free(S);
  return 0;
}
