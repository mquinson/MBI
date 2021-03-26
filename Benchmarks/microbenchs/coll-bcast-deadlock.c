////////////////// MPI bugs collection header //////////////////
//
// Origin: mcs.anl.gov
//
// Description: There is a cyclic dependency if the broadcast is a synchronizing operation. Example described in
// http://www.mcs.anl.gov/research/projects/mpi/mpi-standard/mpi-report-1.1/node86.htm#Node86
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Incorrect
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Correct
// GRP: Correct
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
// LOOP:  Lacking
// SP: Correct
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
// Test: mpirun -np 3 ${EXE}
// Expect: deadlock
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <assert.h>
#include <stdio.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char** argv)
{
  int nprocs = -1;
  int rank   = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int buf1, buf2;
  MPI_Comm comm0, comm1, comm2;
  MPI_Group world_group, group0, group1, group2;
  const int ranks_comm0[2] = {0, 1};
  const int ranks_comm1[2] = {1, 2};
  const int ranks_comm2[2] = {0, 2};

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs != 3) {
    printf("\033[0;31m! This test requires exactly 3 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  MPI_Comm_group(MPI_COMM_WORLD, &world_group);

  MPI_Group_incl(world_group, 2, ranks_comm0, &group0);
  MPI_Comm_create_group(MPI_COMM_WORLD, group0, 0, &comm0);

  MPI_Group_incl(world_group, 2, ranks_comm1, &group1);
  MPI_Comm_create_group(MPI_COMM_WORLD, group1, 0, &comm1);

  MPI_Group_incl(world_group, 2, ranks_comm2, &group2);
  MPI_Comm_create_group(MPI_COMM_WORLD, group2, 0, &comm2);

  buf1 = rank;
  buf2 = rank;

  switch (rank) {
    case 0:
      MPI_Bcast(&buf1, 1, MPI_INT, 0, comm0);
      MPI_Bcast(&buf2, 1, MPI_INT, 1, comm2); // P2 has rank 1 in comm2
      break;
    case 1:
      MPI_Bcast(&buf1, 1, MPI_INT, 0, comm1); // P1 has rank 0 in comm1
      MPI_Bcast(&buf2, 1, MPI_INT, 0, comm0);
      break;
    case 2:
      MPI_Bcast(&buf1, 1, MPI_INT, 1, comm2); // P2 has rank 1 in comm2
      MPI_Bcast(&buf2, 1, MPI_INT, 0, comm1); // P1 has rank 0 in comm1
      break;
  }

  printf("process %d: buf1=%d, buf2=%d\n", rank, buf1, buf2);

  MPI_Group_free(&world_group);
  MPI_Group_free(&group0);
  MPI_Group_free(&group1);
  MPI_Group_free(&group2);

  if (comm0 != MPI_COMM_NULL)
    MPI_Comm_free(&comm0);
  if (comm1 != MPI_COMM_NULL)
    MPI_Comm_free(&comm1);
  if (comm2 != MPI_COMM_NULL)
    MPI_Comm_free(&comm2);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
