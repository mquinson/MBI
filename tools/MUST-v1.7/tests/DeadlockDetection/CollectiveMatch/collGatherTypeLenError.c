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
 * @file collGatherTypeLenError.c
 * A test with an incorrect MPI_Gather call (Error).
 *
 * Description:
 * All processes execute an MPI_Gather each rank sends 3 ints using a count of 3,
 * while the root only receives 2 ints from each rank by using a derived type. (ERROR)
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
         sendBuf[3],
         root;
    MPI_Status status;
    MPI_Datatype rType;

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

    if (rank == 0)
    {
        temp = (int*) malloc (sizeof(int) * size * 2);
        MPI_Type_contiguous (2, MPI_INT, &rType);
        MPI_Type_commit (&rType);
    }

    MPI_Gather (&sendBuf, 3, MPI_INT, temp, 1, rType, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        MPI_Type_free (&rType);
        if (temp) free (temp);
    }

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
