////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: This will deadlock iff the first recv from P0 is matched with the send
//              from P2 ; in that case, P0 run a recv that matches no send.
//
//// List of features
// P2P: Incorrect
// iP2P: Lacking
// PERS: Incorrect
// COLL: Lacking  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
// LOOP: Lacking
// SP: Lacking
//
//// List of errors
// deadlock: transient
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// compliance: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: deadlock
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char * argv[]) {

    int nprocs = -1;
    int rank   = -1;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int namelen = 128;
    int data = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);
    printf("rank %d is alive on %s\n", rank, processor_name);
  
    if (rank == 0) {
	MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	if (status.MPI_SOURCE == 2)
	    MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (rank == 1 || rank == 2) {
	MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  
    MPI_Finalize();
    printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
    return 0;
}
