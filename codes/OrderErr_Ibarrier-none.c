/***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: PARCOACH

  Description: Colective mismatch. Odd processes call MPI_Ibarrier while the
							 others don't 

BEGIN_MPI_FEATURES
  P2P:   Lacking
  iP2P:  Lacking
  PERS:  Lacking
  COLL:  Lacking
  iCOLL: Incorrect
  TOPO:  Lacking
  RMA:   Lacking
  PROB:  Lacking
  COM:   Lacking
  GRP:   Lacking
  DATA:  Lacking
  OP:    Lacking
END_MPI_FEATURES

BEGIN_ERROR_LABELS
  deadlock:  transient
  numstab:   never
  mpierr:    never
  resleak:   never
  datarace:  never
  various:   never
END_ERROR_LABELS

BEGIN_MBI_TESTS
  $ mpirun -np 2 ${EXE}
  | ERROR: Wrong order of MPI calls 
  | Collective mistmatch. MPI_Ibarrier line 64
  | is not called by all processes
END_MBI_TESTS

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
  printf("Hello from rank %d\n", rank);

  if (nprocs < 2)
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug "
           "!\033[0;0m\n");

  if (!rank % 2) {
    MPI_Ibarrier(MPI_COMM_WORLD, &req);
    MPI_Wait(&req, &status);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
