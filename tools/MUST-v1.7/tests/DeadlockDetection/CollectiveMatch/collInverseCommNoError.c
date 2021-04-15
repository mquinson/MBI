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
 * A new communicator that reverses the order of MPI_COMM_WORLD is used in a Bcast and
 * a barrier, the same collectives are also executed on MPI_COMM_WORLD.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
    int rank,size, temp, root, *ranksIncl, i;
    MPI_Status status;
    MPI_Group gworld, ginverse;
    MPI_Comm cinverse;

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

    //== Create an inverse comm
    MPI_Comm_group (MPI_COMM_WORLD, &gworld);
    ranksIncl = (int*) malloc (sizeof(int) * size);
    for (i = 0; i < size; i++)
        ranksIncl[i] = size - i - 1;
    MPI_Group_incl (gworld, size, ranksIncl, &ginverse);
    MPI_Comm_create (MPI_COMM_WORLD, ginverse, &cinverse);

    //Do some colls on world and the inverse comm
    MPI_Bcast (&rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier (cinverse);
    MPI_Bcast (&rank, 1, MPI_INT, 0, cinverse);
    MPI_Barrier (MPI_COMM_WORLD);

    //Clean up
    MPI_Comm_free (&cinverse);
    MPI_Group_free (&ginverse);
    MPI_Group_free (&gworld);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
