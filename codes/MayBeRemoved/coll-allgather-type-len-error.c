/***************************************************************************
/////////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MUST (name in MUST test suite: collAllgatherTypeLenError.cpp)

 Description: Performs a MPI_Allgather collective with a type error that
               occurs when nprocs = 3


  List of MPI features:

       P2P:   Lacking
       iP2P:  Lacking
       PERS:  Lacking
       COLL:  Incorrect
       iCOLL: Lacking
       TOPO:  Lacking
       IO:    Lacking
       RMA:   Lacking
       PROB:  Lacking
       COM:   Lacking
       GRP:   Lacking
       DATA:  Correct
       OP:    Lacking

  List of error labels:

       deadlock:  never
       numstab:   never
       segfault:  never
       mpierr:    transient
       resleak:   never
       livelock:  never
       datarace:  never

  BEGIN_MBI_TESTS
   $ mpirun -np 3 ${EXE}
   | ERROR: Collective mismatch
   | Expected: MPI_Allgather line 84 
  END_MBI_TESTS

****************************************************************************/
//////////////////       original file begins        //////////////////



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
  int i;
  int inbuf[30], outbuf[6];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs != 3) {
    if (rank == 0)
      printf("\033[0;31m! This test needs exactly 3 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  for (i = 0; i < 6; i++)
    outbuf[i] = rank * 6 + i;

  MPI_Datatype conti;
  MPI_Type_contiguous(3 - rank, MPI_INT, &conti);
  MPI_Type_commit(&conti);

  int size_in, size_ints;
  MPI_Type_size(conti, &size_in);
  MPI_Type_size(MPI_INT, &size_ints);
  fprintf(stderr, "Rank %d sends %d x %d INTS and receive %d INTS\n", rank,
          6 / (3 - rank), size_in / size_ints, 6 + rank % 2);
  MPI_Allgather(outbuf, 6 / (3 - rank), conti, inbuf, 6 + rank % 2, MPI_INT,
                MPI_COMM_WORLD);

  MPI_Type_free(&conti);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
