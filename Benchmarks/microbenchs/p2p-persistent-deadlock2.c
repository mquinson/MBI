////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: Do some MPI persistent calls, including freeing an active request - the effect of which is
// implementation dependent
//
//				 Communication pattern:
//
//				   P0           				 P1
//				 barrier       			barrier
//				 send_init(1,req0)  recv_init(0,req0)
//				 recv_init(0,req1)  send_init(0,req1)
//				 start(req0)     		start(req0)
//				 start(req1)    		start(req1)
//				 waitall   		 			waitall
//				 startall  		 			startall
//				 waitall   		 			waitall
//				 barrier   		 			barrier
//
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Incorrect
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
  MPI_Request aReq[2], free_req;
  MPI_Status aStatus[2];

  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  MPI_Barrier(MPI_COMM_WORLD);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
  } else {
    if (rank == 0) {
      memset(buf0, 0, buf_size * sizeof(int));

      MPI_Send_init(buf0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &aReq[0]);
      MPI_Recv_init(buf1, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &aReq[1]);

      MPI_Start(&aReq[0]);
      MPI_Start(&aReq[1]);

      MPI_Waitall(2, aReq, aStatus);

      memset(buf0, 1, buf_size * sizeof(int));

      MPI_Startall(2, aReq);

      /* free an active request... */
      free_req = aReq[1];
      MPI_Request_free(&free_req);

      MPI_Waitall(2, aReq, aStatus);
    } else if (rank == 1) {
      memset(buf1, 1, buf_size * sizeof(int));

      MPI_Recv_init(buf0, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &aReq[0]);
      MPI_Send_init(buf1, buf_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &aReq[1]);

      MPI_Start(&aReq[0]);
      MPI_Start(&aReq[1]);

      MPI_Waitall(2, aReq, aStatus);

      memset(buf1, 0, buf_size * sizeof(int));

      MPI_Startall(2, aReq);

      /* free an active request... */
      free_req = aReq[1];
      MPI_Request_free(&free_req);

      MPI_Waitall(2, aReq, aStatus);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Request_free(&aReq[0]);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
