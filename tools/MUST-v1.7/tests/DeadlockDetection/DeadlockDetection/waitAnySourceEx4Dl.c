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
 * @file waitAnySourceEx4Dl.c
 * Complex example where a deadlock happens while wildcard source updates won't arrive, even though they are partners available for them.
 *
 * Description:
 * Complex deadlock, gives MUST a hard time.
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
        if (size < 4)
        {
            printf ("This test needs at least 4 processes!\n");
            MPI_Finalize();
            return 1;
        }

        //Say hello
        printf ("Hello, I am rank %d of %d processes.\n", rank, size);

        if (rank == 0)
        {
            MPI_Irecv (&size, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
            //This is the error, would also have to provide a send

            MPI_Recv (&buf, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status); //This will hang

            MPI_Wait (&request, &status);
        }
        if (rank == 1)
        {
            MPI_Irecv (&size, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
            MPI_Send (&buf, 1, MPI_INT, 0, 666, MPI_COMM_WORLD);

            MPI_Recv (&buf, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status); //This will hang

            MPI_Wait (&request, &status);
        }
        if (rank == 2)
        {
            MPI_Irecv (&size, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
            MPI_Send (&buf, 1, MPI_INT, 1, 666, MPI_COMM_WORLD);

            MPI_Recv (&buf, 1, MPI_INT, 3, 123, MPI_COMM_WORLD, &status); //This will hang

            MPI_Wait (&request, &status);
        }
        if (rank == 3)
        {
            MPI_Irecv (&size, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
            MPI_Send (&buf, 1, MPI_INT, 2, 666, MPI_COMM_WORLD);
            MPI_Wait (&request, &status); //This will hang

            MPI_Send (&buf, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
            MPI_Send (&buf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
            MPI_Send (&buf, 1, MPI_INT, 2, 123, MPI_COMM_WORLD);
        }

        //Say bye bye
        printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
