/***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: mcs.anl.gov

  Description: Root mismatch. Two processes execute broadcast operations in reverse order.
							 Example described in 
								http://www.mcs.anl.gov/research/projects/mpi/mpi-standard/mpi-report-1.1/node86.htm#Node86 


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
  numstab:   transient
  mpierr:    never
  resleak:   never
  datarace:  never
  various:   never
END_ERROR_LABELS

BEGIN_MBI_TESTS
   $ mpirun -np 2 ${EXE}
   | Root mistmatch
	 | Process 0 calls MPI_Bcast line 75 with root 0 while process 1 calls
   | MPI_Bacst line 80 with root 1.
END_MBI_TESTS

****************************************************************************/
//////////////////////       original file begins        ///////////////////


#include <assert.h>
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
  int buf1, buf2;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs != 2) {
    printf("\033[0;31m! This test requires exactly 2 processes !\033[0;0m\n");
  } else {

    buf1 = rank;
    buf2 = rank;

    switch (rank) {
    case 0:
      MPI_Bcast(&buf1, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(&buf2, 1, MPI_INT, 1, MPI_COMM_WORLD);
      break;
    case 1:
      MPI_Bcast(&buf2, 1, MPI_INT, 1, MPI_COMM_WORLD);
      MPI_Bcast(&buf1, 1, MPI_INT, 0, MPI_COMM_WORLD);
      break;
    }
    printf("process %d: buf1=%d, buf2=%d\n", rank, buf1, buf2);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
