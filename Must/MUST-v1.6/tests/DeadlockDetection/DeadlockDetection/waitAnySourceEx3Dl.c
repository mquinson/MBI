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
 * @file waitAnySourceEx3Dl.c
 * Same as waitAnySourceEx2Dl.c but ranks 0-1 are switched.
 *
 * Description:
 * see waitAnySourceEx2Dl.c.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size, buf;
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
            MPI_Isend (&size, 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &request);

            MPI_Recv (&buf, 1, MPI_INT, 0, 777, MPI_COMM_WORLD, &status);

            MPI_Wait (&request, &status);
        }

        if (rank == 0)
        {
            MPI_Irecv (&size, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);

            MPI_Recv (&buf, 1, MPI_INT, 1, 777, MPI_COMM_WORLD, &status);

            MPI_Wait (&request, &status);
        }

        //Say bye bye
        printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
