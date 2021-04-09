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
 * @file collScattervNoErrorLarge.cpp
 * Collective matching test.
 *
 * Description:
 * Performs a MPI_Scatterv collective with no error
 *
 *  @date 23.03.2012
 *  @author Joachim Protze
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
    int size, rank, i;
    int inbuf[6]; 

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (size<3){
        if (rank == 0)
            std::cout << "This test needs at least 3 processes" << std::endl;
        MPI_Finalize ();
        return 1;
    }
    
    MPI_Datatype conti;
    MPI_Type_contiguous(3-(rank%3),MPI_INT,&conti);
    MPI_Type_commit(&conti);

    int *outbuf, *displs, *sendcnts;
    outbuf=new int[size*10];
    for (i=0; i<size*10; i++)
        outbuf[i]=rank*size*10+i;
    
    displs=new int[size];
    sendcnts=new int[size];
    for (i=0; i<size; i++)
    {
        displs[i]=i*10;
        sendcnts[i]=6;
    }

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    MPI_Scatterv(outbuf, sendcnts, displs, MPI_INT, inbuf, 6/(3-(rank%3)), conti, 1, MPI_COMM_WORLD);

    MPI_Type_free(&conti);
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    delete[] outbuf;
    delete[] displs;
    delete[] sendcnts;

    MPI_Finalize ();


    return 0;
}
