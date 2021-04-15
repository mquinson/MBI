////////////////// MPI bugs collection header //////////////////
//
// Origin: MPI Correctness Benchmark
//
// Description: All processes call a nonblocking MPI_Barrier
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
// iCOLL: Correct
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
/*
  BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | OK
  END_MBI_TESTS
*/
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
	MPI_Request req;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (rank % 2)
    MPI_Ibarrier(MPI_COMM_WORLD, &req);

  if (!(rank % 2))
    MPI_Ibarrier(MPI_COMM_WORLD, &req);


  MPI_Wait(&req, MPI_STATUS_IGNORE);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
