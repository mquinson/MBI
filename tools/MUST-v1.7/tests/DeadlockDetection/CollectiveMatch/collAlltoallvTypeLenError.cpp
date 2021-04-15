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
 * @file collAlltoallvTypeLenError.cpp
 * Collective matching test.
 *
 * Description:
 * Performs a MPI_Alltoallv collective with an error
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


    MPI_Datatype conti;
    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    int recvcnts[3] = {18,12,6};
    int sendcnts[3] = {18,12,6};
    int displs[3] = {0,36,72};
    int sendtypesize;
    sendtypesize = (3-rank);

    MPI_Type_contiguous(sendtypesize, MPI_INT, &conti);
    MPI_Type_commit(&conti);

    for (i=0; i<3; i++)
    {
        recvcnts[i] = ((3 + rank - i)%3+1)*6 + rank%2;
        sendcnts[i] = ((3 - rank + i)%3+1)*6 / sendtypesize;
    }

    MPI_Alltoallv(outbuf, sendcnts, displs, conti, inbuf, recvcnts, displs, MPI_INT, MPI_COMM_WORLD);

    MPI_Type_free(&conti);
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize ();

    return 0;
}
