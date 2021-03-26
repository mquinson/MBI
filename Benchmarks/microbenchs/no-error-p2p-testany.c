////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Use of MPI_Testany without error
//
//// List of features
// P2P: Correct
// iP2P: Correct
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
// LOOP: Correct
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
// Test: mpirun -np 2 ${EXE}
// Expect: noerror
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <assert.h>
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
  int buf2[buf_size];
  int i, flipbit, done, flag;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
  } else if (rank == 0) {
    MPI_Request reqs[3];

    MPI_Irecv(buf0, buf_size, MPI_INT, 1, 1, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(buf1, buf_size, MPI_INT, 1, 2, MPI_COMM_WORLD, &reqs[1]);
    MPI_Irecv(buf2, buf_size, MPI_INT, 1, 3, MPI_COMM_WORLD, &reqs[2]);

    for (i = 3; i > 0; i--) {
      MPI_Send(&flipbit, 1, MPI_INT, 1, i, MPI_COMM_WORLD);

      flag = 0;
      printf("req = %p", reqs);
      while (!flag)
        MPI_Testany(i, reqs, &done, &flag, &status);

      assert(done == (i - 1));
    }
  } else if (rank == 1) {
    memset(buf0, 1, buf_size * sizeof(int));

    for (i = 3; i > 0; i--) {
      MPI_Recv(&flipbit, 1, MPI_INT, 0, i, MPI_COMM_WORLD, &status);
      MPI_Send(buf0, buf_size, MPI_INT, 0, i, MPI_COMM_WORLD);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
