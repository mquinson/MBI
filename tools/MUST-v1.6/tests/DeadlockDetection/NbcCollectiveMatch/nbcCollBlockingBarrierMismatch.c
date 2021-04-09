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
 * @file nbcCollBlockingBarrierMismatch.c
 * This test case attempts to match an MPI_Ibarrier with an MPI_Barrier,
 * which is not allowed according to the MPI-3 standard.
 *
 * Description:
 * All processes execute an MPI_Ibarrier on MPI_COMM_WORLD,
 * they then wait on the resulting request.
 * However, process 1 executes an MPI_Barrier instead.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
    int rank,
         size;
    MPI_Status status;
    MPI_Request request;

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

    if (rank == 1)
    {
        MPI_Ibarrier (MPI_COMM_WORLD, &request);
        MPI_Wait (&request, &status);
    }
    else
    {
        MPI_Barrier (MPI_COMM_WORLD);
    }

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
