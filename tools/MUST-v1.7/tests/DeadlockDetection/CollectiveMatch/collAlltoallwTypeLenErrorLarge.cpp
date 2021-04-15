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
 * @file collAlltoallwTypeLenErrorLarge.cpp
 * Collective matching test.
 *
 * Description:
 * Performs a MPI_Alltoallw collective that causes type signature length mismatches (Error).
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

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (size<3){
        if (rank == 0)
            std::cout << "This test needs at least 3 processes" << std::endl;
        MPI_Finalize ();
        return 1;
    }
    

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    MPI_Datatype *contis;
    int *inbuf, *outbuf, *displs, *recvcnts, *sendcnts;
    inbuf=new int[size*18];
    outbuf=new int[size*18];
    contis=new MPI_Datatype[size];
    displs=new int[size];
    recvcnts=new int[size];
    sendcnts=new int[size];

    int typesize;

    for (i=0; i<size*18; i++)
        outbuf[i]=rank*size*18+i;

    for (i=0; i<size; i++)
    {
        typesize = (size + 2 - rank + i)%3+1; //THIS is the ERORR, nust be: (2 + rank + i)%3+1
        MPI_Type_contiguous(typesize, MPI_INT, contis+i);
        MPI_Type_commit(contis+i);
        recvcnts[i] = ((size + rank - i)%3+1)*6 / typesize;
        sendcnts[i] = ((size - rank + i)%3+1)*6 / typesize;
        displs[i] = 18*i*sizeof(int);
    }
    if (rank==size-1)
        recvcnts[0]+=1;

    MPI_Alltoallw(outbuf, sendcnts, displs, contis, inbuf, recvcnts, displs, contis, MPI_COMM_WORLD);

    for (i=0; i<size; i++)
    {
        MPI_Type_free(contis + i);
    }
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize ();

    return 0;
}
