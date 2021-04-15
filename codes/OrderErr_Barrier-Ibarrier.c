 /***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: PARCOACH

  Description: Colective mismatch. Some processes call MPI_Barrier while
               others call MPI_Ibarrier. 


  List of MPI features:

       P2P:   Lacking
       iP2P:  Lacking
       PERS:  Lacking
       COLL:  Incorrect
       iCOLL: Incorrect
       TOPO:  Lacking
       IO:    Lacking
       RMA:   Lacking
       PROB:  Lacking
       COM:   Lacking
       GRP:   Lacking
       DATA:  Correct
       OP:    Lacking

  List of error labels:

       deadlock:  transient
       numstab:   never
       segfault:  never
       mpierr:    never
       resleak:   never
       livelock:  never
       datarace:  never

  Test: mpirun -np 3 ${EXE}
  Expected: Wrong order of MPI calls 
						Collective mistmatch. MPI_Barrier line 65
						is matched with MPI_Ibarrier line 67 

****************************************************************************/
//////////////////////       original file begins        ///////////////////


#include <mpi.h>
#include <stdio.h>


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  MPI_Request req;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \n", rank);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug "
           "!\033[0;0m\n");
  } else {
    if (rank % 2) {
      MPI_Barrier(MPI_COMM_WORLD);
    } else {
      MPI_Ibarrier(MPI_COMM_WORLD, &req);
      MPI_Wait(&req, &status);
    }
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
