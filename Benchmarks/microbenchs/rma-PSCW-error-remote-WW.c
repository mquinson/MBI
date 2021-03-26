////////////////// MPI bugs collection header //////////////////
//
// Origin: MC-Checker
//
// Description: Consistency error accross processes for active mode. P0 and P2 may access the
// window location in P1 concurrently
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Incorrect
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// LOOP: Lacking
// SP: Correct
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: transient
//
// Test: mpirun -np 3 ${EXE}
// Expect: datarace
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
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
  MPI_Win win;
  int X; // Window buffer
  int localbuf; // Local buffer
  MPI_Group world_group;
  MPI_Group gp1; // group containing P1
  MPI_Group gp02; // group containing P0 and P2
  const int ranks02[2] = {0, 2};
  const int ranks1[1] = {1};

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  localbuf=rank;
  X=4;

  
  MPI_Comm_group(MPI_COMM_WORLD, &world_group);
  MPI_Group_incl(world_group, 2, ranks02, &gp02);
  MPI_Group_incl(world_group, 1, ranks1, &gp1);

  if (nprocs < 3) {
    printf("\033[0;31m! This test needs at least 3 processes !\033[0;0m\n");
    MPI_Finalize();
    return 1;
  }

  MPI_Win_create(&X, sizeof(int),sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &win);

  if(rank == 0 || rank == 2){
    MPI_Win_start(gp1, 0, win);
    MPI_Put(&localbuf, 1, MPI_INT, 1, 0, 1, MPI_INT, win);
    MPI_Win_complete(win);
  }else{
    if(rank == 1){
      MPI_Win_post(gp02, 0, win);
      MPI_Win_wait(win);
    }
  }

  MPI_Group_free(&world_group);
  MPI_Group_free(&gp02);
  MPI_Group_free(&gp1);
  MPI_Win_free(&win);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
