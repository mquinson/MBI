////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Send/Recv communication on a new communicator created with MPI_Comm_split. Blocking recv-recv causing a
// deadlock.
//
//             Communication pattern:
//
//                 P0      P1
//              recv(1)  recv(0)
//              send(1)  send(0)
//
//
//// List of features
// P2P: Incorrect
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Correct
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
// LOOP: Lacking
// SP: Correct
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
  MPI_Comm comm;
  int drank, dnprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (nprocs < 3) {
    printf("\033[0;31m! This test needs at least 3 processes\033[0;0m\n");
  } else {
    MPI_Comm_split(MPI_COMM_WORLD, rank % 2, nprocs - rank, &comm);

    if (comm != MPI_COMM_NULL) {
      MPI_Comm_size(comm, &dnprocs);
      MPI_Comm_rank(comm, &drank);

      if (dnprocs > 1) {
        if (drank == 0) {
          memset(buf0, 0, buf_size * sizeof(int));
          MPI_Recv(buf1, buf_size, MPI_INT, 1, 0, comm, &status);
          MPI_Send(buf0, buf_size, MPI_INT, 1, 0, comm);
        } else if (drank == 1) {
          memset(buf1, 1, buf_size * sizeof(int));
          MPI_Recv(buf0, buf_size, MPI_INT, 0, 0, comm, &status);
          MPI_Send(buf1, buf_size, MPI_INT, 0, 0, comm);
        }
      } else {
        printf("(%d) Derived communicator too small (size = %d)\n", rank, dnprocs);
      }

      MPI_Comm_free(&comm);
    } else {
      printf("(%d) Got MPI_COMM_NULL\n", rank);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
