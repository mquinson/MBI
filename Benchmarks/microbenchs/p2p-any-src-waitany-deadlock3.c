////////////////// MPI bugs collection header //////////////////
//
// Origin: Hermes
//
// Description: A deadlock occurs if P0 first send is non-blocking and it's second send happens before P1's.
//              buf_size should be smaller than the tmp buffer size used by send implementation.
//
//				 Communication pattern:
//
//				  P0         P1        P2
//                              barrier    barrier   barrier
//				send(1)    send(2)  recv(any)
//				send(2)    recv(0)   recv(0)
//				 
//         
//
//
//// List of features
// P2P: Incorrect
// iP2P: Lacking
// PERS: Lacking
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
// Test: mpirun -np 3 ${EXE}
// Expect: deadlock
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include "mpi.h"
#include <stdio.h>
#include <string.h>

#define buf_size 1

int main(int argc, char** argv) 
{
    int nprocs = -1;
    int rank   = -1;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int namelen = 128;
    int token0[buf_size];
    int token1[buf_size+1];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);
    printf("rank %d is alive on %s\n", rank, processor_name);

    MPI_Barrier(MPI_COMM_WORLD);

    if (nprocs < 3) {
	printf("\033[0;31m! This test needs at least 3 processes !\033[0;0m\n");
    } else if (rank == 0) {
	memset(token0, 1, (buf_size) * sizeof(int));
	memset(token1, 1, (buf_size + 1) * sizeof(int));
	MPI_Send(&token0, buf_size, MPI_INT, 1, 0, MPI_COMM_WORLD);
	MPI_Send(&token1, buf_size+1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
	memset(token1, 1, (buf_size + 1) * sizeof(int));
	sleep(5);
	MPI_Send(&token1, buf_size+1, MPI_INT, 2, 20, MPI_COMM_WORLD);
	MPI_Recv(&token1, buf_size+1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (rank == 2) {
	MPI_Recv(&token1, buf_size+1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&token1, buf_size+1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Finalize();
    printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
    return 0;

}
