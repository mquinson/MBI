////////////////// MPI bugs collection header //////////////////
//
// Origin: Parcoach
//
// Description: MPI_THREAD_SERIALIZED is the minimum level of compliance that should be used
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
// HYB: Incorrect
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
// compliance: always
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: compliance
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include "omp.h"
#include <stdio.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define NUM_THREADS 2


int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int provided;
  int tid=0, nbt=0;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

#pragma omp parallel num_threads(NUM_THREADS)
  {
    if (omp_get_num_threads() < 2)
      printf("\033[0;31m! This test needs at least 2 OpenMP threads to produce a bug !\033[0;0m\n");

    nbt = omp_get_num_threads();
    tid = omp_get_thread_num();
    printf("hello from OpenMP thread %d/%d in P%d\n", tid, nbt, rank);


#pragma omp single
    {
      printf("OpenMP thread %d in P%d call MPI_Barrier\n", omp_get_thread_num(), rank);
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
