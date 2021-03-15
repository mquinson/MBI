////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: This code creates many types and fails to free some
//
//// List of features
// P2P: Lacking
// iP2P: Correct
// PERS: Lacking
// COLL: Correct  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Incorrect
// OP: Lacking
// HYB: Lacking
// LOOP: Lacking
// SP: Lacking
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: always
// livelock: never
// compliance: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: resleak
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define ITERATIONS 10
#define TYPES_PER_ITERATION 3
#define TYPES_LOST_PER_ITERATION 1
#define TYPES_TO_COMMIT 1

#define buf_size 128

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  int i, j, partner, buf_j;
  int buf[buf_size * TYPES_TO_COMMIT];
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  MPI_Datatype newtype[TYPES_PER_ITERATION];
  MPI_Request reqs[TYPES_TO_COMMIT];
  MPI_Status statuses[TYPES_TO_COMMIT];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test needs at least 2 processes !\033[0;0m\n");
  } else {
    MPI_Barrier(MPI_COMM_WORLD);

    if ((nprocs % 2 == 0) || (rank != nprocs - 1)) {
      partner = (rank % 2) ? rank - 1 : rank + 1;

      for (i = 0; i < ITERATIONS; i++) {
        for (j = 0; j < TYPES_PER_ITERATION; j++) {
          MPI_Type_contiguous(buf_size, MPI_INT, &newtype[j]);

          buf_j = j + TYPES_TO_COMMIT - TYPES_PER_ITERATION;

          if (buf_j >= 0) {
            MPI_Type_commit(&newtype[j]);

            if (rank % 2) {
              MPI_Irecv(&buf[buf_j * buf_size], 1, newtype[j], partner, buf_j, MPI_COMM_WORLD, &reqs[buf_j]);
            } else {
              MPI_Isend(&buf[buf_j * buf_size], 1, newtype[j], partner, buf_j, MPI_COMM_WORLD, &reqs[buf_j]);
            }
          }

          if (j < TYPES_PER_ITERATION - TYPES_LOST_PER_ITERATION) {
            MPI_Type_free(&newtype[j]);
          }
        }
        MPI_Waitall(TYPES_TO_COMMIT, reqs, statuses);
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
