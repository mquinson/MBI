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
 * @file IntegrityIfNullStatusesError.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 * Description:
 * Performs a send and Irecv to get a request array. This request array will be tested with testall.
 * The array of statuses is set to null. this will cause an error
 *
 *  @date 03.06.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
    int size, rank;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    if (size < 2)
    {
        std::cerr << "This test needs at least 2 processes!"<< std::endl;
        MPI_Finalize();
        return 1;
    }

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    MPI_Status status[4];
    MPI_Request request[4];
    int buffer[4],flag;

    if (rank == 1)
    {
        for(int i=0; i<4;i++)
        {
            MPI_Irecv(&(buffer[i]), 1, MPI_INT, 0, (42+i), MPI_COMM_WORLD, &(request[i]));
        }

        flag = 0;
        MPI_Waitall(4, request, NULL); //This will cause an error if MPI_STATUSES_IGNORE is not defined as NULL or not defined at all.
    }


    if (rank == 0)
    {
        for(int i=0; i<4;i++)
        {
            buffer[i] = i;
            MPI_Send(&(buffer[4]), 1, MPI_INT, 1, (42+i), MPI_COMM_WORLD);
        }
    }

    //Say bye bye
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize ();

    return 0;
}
