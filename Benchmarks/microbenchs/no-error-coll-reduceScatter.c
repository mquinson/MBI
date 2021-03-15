////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: This code performs a MPI_Reduce_scatter with no error
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
// HYB: Lacking
// LOOP: Lacking
// SP: Lacking
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// compliance: never
// datarace: never
//
// Test: mpirun -np 3 ${EXE}
// Expect: noerror
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
    int i;
    int inbuf[20], outbuf[20];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);
    printf("rank %d is alive on %s\n", rank, processor_name);

    if (nprocs != 3) {
	if (rank == 0)
	    printf("\033[0;31m! This test needs at least 3 processes !\033[0;0m\n");
	MPI_Finalize();
	return 1;
    }

    for (i = 0; i < 20; i++)
	outbuf[i] = rank * 20 + i;

    MPI_Datatype conti;
    MPI_Type_contiguous(3 - rank, MPI_INT, &conti);
    MPI_Type_commit(&conti);

    int recvcnts[3] = {5, 6, 7};

    for (int r = 0; r < nprocs; r++) {
	
	recvcnts[r] += r % 2;

    }
	
    MPI_Reduce_scatter(outbuf, inbuf, recvcnts, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    MPI_Type_free(&conti);

    MPI_Finalize();
    printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
    return 0;
}
