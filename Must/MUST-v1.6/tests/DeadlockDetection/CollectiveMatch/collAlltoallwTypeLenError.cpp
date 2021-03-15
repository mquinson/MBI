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
 * @file collAlltoallwTypeLenError.cpp
 * Collective matching test.
 *
 * Description:
 * Performs a MPI_Alltoallw collective with an error
 *
 *
 *  @date 27.03.2012
 *  @author Joachim Protze
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
    int size, rank, i;
    int inbuf[200], outbuf[200];

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (size!=3){
        if (rank == 0)
            std::cout << "This test needs 3 processes" << std::endl;
        MPI_Finalize ();
        return 1;
    }
    
    for (i=0; i<100; i++)
        outbuf[i]=rank*100+i;

    MPI_Datatype contis[3];

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    int recvcnts[3] = {18,12,6};
    int sendcnts[3] = {18,12,6};
    int displs[3] = {0*sizeof(int),48*sizeof(int),96*sizeof(int)};
    int typesize;

    for (i=0; i<3; i++)
    {
        typesize = (2 + rank + i)%3+1;
        MPI_Type_contiguous(typesize, MPI_INT, contis+i);
        MPI_Type_commit(contis+i);
        recvcnts[i] = ((3 + rank - i)%3+1 + rank%2)*6 / typesize;
        sendcnts[i] = ((3 - rank + i)%3+1)*6 / typesize;
    }
    MPI_Alltoallw(outbuf, sendcnts, displs, contis, inbuf, recvcnts, displs, contis, MPI_COMM_WORLD);

    for (i=0; i<3; i++)
    {
        MPI_Type_free(contis + i);
    }
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize ();

    return 0;
}
