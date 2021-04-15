/***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)

  Description: Collective mismatch. Process 0 calls MPI_Reduce while others 
               call MPI_Allreduce


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

BEGIN_TESTS
  $ mpirun -np 2 ${EXE}
  | Wrong order of MPI calls 
  | Collective mistmatch. MPI_Reduce line 81 is matched with
  | MPI_Allreduce line 84
END_TESTS

****************************************************************************/
//////////////////////       original file begins        ///////////////////


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define buf_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int sbuf[buf_size];
  int rbuf[buf_size];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes to produce a bug "
           "!\033[0;0m\n");
  } else {
    memset(sbuf, 0, buf_size * sizeof(int));
    memset(rbuf, 1, buf_size * sizeof(int));

    MPI_Barrier(MPI_COMM_WORLD);

    switch (rank) {
    case 0:
      MPI_Reduce(sbuf, rbuf, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      break;
    default:
      MPI_Allreduce(sbuf, rbuf, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
      break;
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
