 /***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: Colective mismatch. Some processes call MPI_Ibarrier followed by
               MPI_Ireduce while others call MPI_Ireduce followed by MPI_Ibarrier 

  List of MPI features:

       P2P:   Lacking
       iP2P:  Lacking
       PERS:  Lacking
       COLL:  Lacking
       iCOLL: Incorrect 
       TOPO:  Lacking
       IO:    Lacking
       RMA:   Lacking
       PROB:  Lacking
       COM:   Lacking
       GRP:   Lacking
       DATA:  Lacking
       OP:    Lacking

  List of error labels:

       deadlock:  transient
       numstab:   never
       segfault:  never
       mpierr:    never
       resleak:   never
       livelock:  never
       datarace:  never

  Test: mpirun -np 2 ${EXE}
  Expected: Wrong order of MPI calls 
            Collective mistmatch. MPI_Ibarrier line 74
            is matched with MPI_Ireduce line 79.
						Some processes call MPI_Ibarrier followed by MPI_Ireduce
						while others call MPI_Ireduce followed by MPI_Ibarrier

****************************************************************************/
//////////////////////       original file begins        ///////////////////



#include <mpi.h>
#include <stdio.h>


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int localsum, sum;
	MPI_Request req1, req2;
	MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \n", rank);

  if (nprocs < 2)
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug "
           "!\033[0;0m\n");

  localsum = 0;
  for (int i = 0; i <= rank; i++) {
    localsum += i;
  }
  printf("process %d has local sum of %d\n", rank, localsum);

  if (rank % 2) {
    MPI_Ibarrier(MPI_COMM_WORLD, &req1);
		MPI_Wait(&req1, &status);
    MPI_Ireduce(&localsum, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD, &req2);
		MPI_Wait(&req2, &status);
  } else {
    MPI_Ireduce(&localsum, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD, &req2);
		MPI_Wait(&req2, &status);
    MPI_Ibarrier(MPI_COMM_WORLD, &req1);
		MPI_Wait(&req1, &status);
  }

  if (rank == 0)
    printf("total sum is %d\n", sum);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
