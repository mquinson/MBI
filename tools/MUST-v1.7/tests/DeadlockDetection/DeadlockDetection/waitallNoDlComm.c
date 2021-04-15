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
 * @file waitallNoDlComm.c
 * Simple test with an MPI_Waitall call causes no deadlock (No Error), but uses a user defined communicator.
 *
 * Description:
 * There is no deadlock in this test, we call correct and matching MPI calls.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,rankB,size;
    MPI_Status statuses[3];
    MPI_Request requests[3];
    MPI_Comm comm, comm2;
    MPI_Group group,inverseGroup;
    int buf[100];
    int i;
    int *ranksIncl;
    MPI_Datatype type,type2,typeB;

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

    MPI_Comm_dup (MPI_COMM_WORLD, &comm);
    MPI_Comm_group (comm, &group);

    ranksIncl = (int*) malloc (sizeof(int) * size);

    for (i = 0; i < size; i++)
    {
        ranksIncl[i] = size - i - 1;
    }
    MPI_Group_incl (group, size, ranksIncl, &inverseGroup);
    MPI_Comm_create (comm, inverseGroup, &comm2);
    MPI_Comm_rank (comm2, &rankB);
    MPI_Type_contiguous (2, MPI_INT, &type);
    typeB = type;
    MPI_Type_contiguous (2, type, &type2);
    MPI_Type_contiguous (2, type2, &type);
    MPI_Type_commit (&type);
    MPI_Type_commit (&typeB);

    printf ("Hello, I am rankB %d from rank %d.\n", rankB, rank);

    for (i = 0; i < 1000; i++)
    {
        if (rank == 0)
        {
            MPI_Isend (&(buf[0]), 4, typeB, 0, 666, comm2, &(requests[0]));
            MPI_Irecv (&(buf[8]), 4, typeB, 0, 666, comm2, &(requests[1]));
            MPI_Waitall (2, requests, statuses);
        }

        if (rank == 1)
        {
            MPI_Irecv (&(buf[0]), 1, type, 1, 666, comm2, &(requests[0]));
            MPI_Isend (&(buf[8]), 1, type, 1, 666, comm2, &(requests[1]));
            MPI_Waitall (2, requests, statuses);
        }

        MPI_Barrier (MPI_COMM_WORLD);
    }

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
