 /***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: Colective mismatch. Some processes call MPI_Barrier followed by
               MPI_Reduce while others call MPI_Reduce followed by MPI_Barrier 

BEGIN_MPI_FEATURES
  P2P:   Lacking
  iP2P:  Lacking
  PERS:  Lacking
  COLL:  Incorrect
  iCOLL: Lacking 
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
  | Collective mistmatch. MPI_Barrier line 72
  | is matched with MPI_Reduce line 75.
	|	Some processes call MPI_Barrier followed by MPI_Reduce
	|	while others call MPI_Reduce followed by MPI_Barrier
END_MBI_TESTS

****************************************************************************/
//////////////////////       original file begins        ///////////////////



#include <mpi.h>
#include <stdio.h>


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int localsum, sum;

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
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&localsum, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  } else {
    MPI_Reduce(&localsum, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
  }

  if (rank == 0)
    printf("total sum is %d\n", sum);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
