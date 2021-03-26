////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: The buffer buf0 used in send is changed before the communication is complete.
//
//					 Communication pattern:
//
//					    P0             P1
//					  barrier        barrier
//					  Isend(1,req0)  Irecv(0,req0)
//					  Isend(1,req1)  Irecv(0,req1)
//					  wait(req0)     wait(req0)
//					  wait(req1)     wait(req1)
//					  barrier        barrier
//					  Isend(1,req0)  Irecv(0,req0)
//					  Isend(1,req1)  Irecv(0,req1)
//					  waitall        waitall
//
//// List of features
// P2P: Lacking
// iP2P: Incorrect
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
// numstab: transient
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: numstab
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  int tag1   = 0;
  int tag2   = 0;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf0[128];
  int buf1[128];
  int i;
  MPI_Request aReq[2];
  MPI_Status aStatus[2];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  for (i = 0; i < 128; i++) {
    buf0[i] = i;
    buf1[i] = 127 - i;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  switch (rank) {
    case 0:
      MPI_Isend(buf0, 128, MPI_INT, 1, tag1, MPI_COMM_WORLD, &aReq[0]);
      MPI_Isend(buf1, 128, MPI_INT, 1, tag2, MPI_COMM_WORLD, &aReq[1]);
      /* do some work here */

      buf0[64] = 1000000;

      MPI_Wait(&aReq[0], &aStatus[0]);
      MPI_Wait(&aReq[1], &aStatus[1]);

      break;

    case 1:
      sleep(20);
      MPI_Irecv(buf0, 128, MPI_INT, 0, tag1, MPI_COMM_WORLD, &aReq[0]);
      MPI_Irecv(buf1, 128, MPI_INT, 0, tag2, MPI_COMM_WORLD, &aReq[1]);
      /* do some work here ... */
      MPI_Wait(&aReq[0], &aStatus[0]);
      MPI_Wait(&aReq[1], &aStatus[1]);
      for (i = 0; i < 128; i++) {
        if (i == 0)
          printf("buf0 =");
        printf(" %d ", buf0[i]);
        if (i == 127)
          printf("\n");
      }
      break;

    default:
      /* do nothing */
      break;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  for (i = 0; i < 128; i++) {
    buf0[i] = i;
    buf1[i] = 127 - i;
  }

  switch (rank) {
    case 0:
      MPI_Isend(buf0, 128, MPI_INT, 1, tag1, MPI_COMM_WORLD, &aReq[0]);
      MPI_Isend(buf1, 128, MPI_INT, 1, tag2, MPI_COMM_WORLD, &aReq[1]);
      /* do some work here */

      buf0[64] = 1000000;

      MPI_Waitall(2, aReq, aStatus);

      break;

    case 1:
      MPI_Irecv(buf0, 128, MPI_INT, 0, tag1, MPI_COMM_WORLD, &aReq[0]);
      MPI_Irecv(buf1, 128, MPI_INT, 0, tag2, MPI_COMM_WORLD, &aReq[1]);
      /* do some work here ... */
      MPI_Waitall(2, aReq, aStatus);
      break;

    default:
      /* do nothing */
      break;
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
