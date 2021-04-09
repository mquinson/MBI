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
 * @file collAllgathervTypeLenError.c
 * A test with a type matching error in MPI_Allgatherv call (ERROR).
 *
 * Description:
 * All processes execute an MPI_Allgatherv where a type match error happens.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
    int rank,
         size,
         *temp = NULL,
         *rcounts = NULL,
         *rdispls = NULL,
         i,
         root;
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    if (size < 2)
    {
        printf ("This test needs at least 4 processes!\n");
        MPI_Finalize();
        return 1;
    }

    //Say hello
    printf ("Hello, I am rank %d of %d processes.\n", rank, size);

    temp = (int*) malloc (sizeof(int) * size*2);
    rcounts = (int*) malloc (sizeof(int) * size);
    rdispls = (int*) malloc (sizeof(int) * size);

    for (i = 0; i < size; i++)
    {
        rcounts[i] = 1;
        rdispls[i] = 2*(size - (i+1));
    }

    if (rank == 3)
        rcounts[1] = 2; //THIS IS THE ERROR must be 1

    MPI_Allgatherv (&rank, 1, MPI_INT, temp, rcounts, rdispls, MPI_INT, MPI_COMM_WORLD);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    if (temp) free (temp);
    if (rcounts) free (rcounts);
    if (rdispls) free (rdispls);


    MPI_Finalize ();

    return 0;
}
