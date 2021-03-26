////////////////// MPI bugs collection header //////////////////
//
// Origin: CIVL
//
// Description: This example demonstrates the usage of MPI collective operations,
//              which should be called in the same orders for all MPI processes.
//              This example has an error when there are more than five MPI processes.
//
//// List of features
// P2P: Lacking
// iP2P: Lacking
// PERS: Lacking
// COLL: Correct  
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Correct
// OP: Lacking
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
// datarace: never
//
// Test: mpirun -np 5 ${EXE}
// Expect: noerror
//
// Test: mpirun -np 6 ${EXE}
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

int main(int argc, char * argv[]) {
    int nprocs = -1;
    int rank   = -1;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int namelen = 128;
    int num;
    int recv;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);
    printf("rank %d is alive on %s\n", rank, processor_name);
    
    if(rank == 0) num = 3;

    MPI_Bcast(&num, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Allreduce(&num, &recv, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if(rank != 5)
	MPI_Reduce(&recv, &num, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Allreduce(&num, &recv, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    assert(recv == (3*nprocs*nprocs + 3*(nprocs-1)));

    MPI_Finalize();
    printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
    return 0;
}
