////////////////// MPI bugs collection header //////////////////
//
// Origin: MUST
//
// Description: Performs a MPI_Send_init/MPI_Recv_init, MPI_Start() and do a wait afterwards.
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
// DATA: Lacking
// OP: Correct
// LOOP: Correct
// SP: Lacking
//
//// List of errors
// deadlock: never
// numstab: never
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: noerror
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////


#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int main (int argc, char** argv)
{
	int size, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Status status;
	MPI_Request request;
	//Enough tasks ?
	if (size < 2)
	{
		printf("This test needs at least 2 processes!");
		MPI_Finalize();
		return 1;
	}

	//Say hello
	printf("Hello, I am rank %d of %d processes\n", rank, size);

	if (rank == 0)
	{
		MPI_Send_init (&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD, &request);
		MPI_Start(&request);
	}

	if (rank == 1)
	{
		MPI_Recv_init (&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &request);
		MPI_Start(&request);
	}

	MPI_Wait(&request,&status);
	if(request != MPI_REQUEST_NULL)
		MPI_Request_free (&request);

	//Say bye bye
	printf("Signing off, rank %d\n",rank);

	MPI_Finalize ();

	return 0;
}
