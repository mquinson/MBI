/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file MatchIndexedBlockLengthError.c
 * Type matching test with an error.
 *
 * Description:
 * A single send-recv match, the send and reveice both use an indexed block type,
 * the receive can not hold the full typesignature of the send (ERROR).
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size;
    long data[100];
    MPI_Status status;
    MPI_Datatype newType;
    int displacements[3]= {3, 6, 9};
    int blocklength = 3;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    	if (size < 2)
    	{
    		printf ("This test needs at least 2 processes!\n");
    		MPI_Finalize();
    		return 1;
    	}

    	//Say hello
    	printf ("Hello, I am rank %d of %d processes.\n", rank, size);

    	//Create a conti type
    	if (rank == 1)
    	    blocklength = 2; //ERROR: must be equal on both tasks
    	MPI_Type_create_indexed_block (3, blocklength, displacements, MPI_LONG, &newType);
    	MPI_Type_commit (&newType);

    	if (rank == 0)
    		MPI_Send (data, 2, newType, 1, 666, MPI_COMM_WORLD);

    	if (rank == 1)
    		MPI_Recv (data, 2, newType, 0, 666, MPI_COMM_WORLD, &status);

    	MPI_Type_free(&newType);
    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
