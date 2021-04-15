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
 * @file collInverseCommNoError.c
 * A test wich uses a user defined communicator and issues correct collectives (No Error).
 *
 * Description:
 * A new communicator that splits MPI_COMM_WORLD into odd and even ranks is used.
 * On each comm different collectives are issued.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
    int rank, size, temp, root, i;
    MPI_Comm csplit;

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

    //== Create splited odd/even comm
    MPI_Comm_split (MPI_COMM_WORLD, rank%2, size-rank, &csplit);
    MPI_Barrier (MPI_COMM_WORLD);
    //Do different things on each comm
    if (rank % 2)
    {
        for (i = 0; i < 5; i++)
        {
            MPI_Bcast (&size, 1, MPI_INT, 0, csplit);
            MPI_Barrier (csplit);
        }
    }
    else
    {
        for (i = 0; i < 5; i++)
        {
            MPI_Barrier (csplit);
            MPI_Bcast (&size, 1, MPI_INT, 0, csplit);
        }
    }


    MPI_Barrier (MPI_COMM_WORLD);

    //Clean up
    MPI_Comm_free (&csplit);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
