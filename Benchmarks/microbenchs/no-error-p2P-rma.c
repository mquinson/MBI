////////////////// MPI bugs collection header //////////////////
//
// Origin: MC-CChecker
//
// Description: This is fatalBug.c from MC-CChecker tool. The code has no error.
// 
//// List of features
// P2P: Correct
// iP2P: Lacking
// PERS: Lacking
// COLL: Lacking
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Correct
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
// HYB: Lacking
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
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: noerror
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

int main(int argc, char **argv)
{
	int size, rank, sharedBuffer, localBuffer, i;
	MPI_Win win;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	localBuffer = rank;
	sharedBuffer = rank;
	MPI_Win_create(&sharedBuffer, 1, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	
	if (rank == 0)
	{
		MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win); 
		MPI_Put(&localBuffer, 1, MPI_INT, 1, 0, 1, MPI_INT, win);
		MPI_Win_unlock(1, win);
		
		MPI_Send(&localBuffer, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	else if (rank == 1)
	{
		MPI_Recv(&localBuffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win);
		MPI_Put(&localBuffer, 1, MPI_INT, 1, 0, 1, MPI_INT, win);
		MPI_Win_unlock(1, win);

		for (i = rank + 1; i < size; i++)
		{
			MPI_Send(&localBuffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	}
	else //if (rank >= 2)
	{
		MPI_Recv(&localBuffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
		MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, win);
		MPI_Get(&localBuffer, 1, MPI_INT, 1, 0, 1, MPI_INT, win);
		MPI_Win_unlock(1, win);
	}

	MPI_Finalize();	
	return 0;
}
