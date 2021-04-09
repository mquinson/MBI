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
 * @file ArrayNotValidForCreate_Undefined.cpp
 * This is a a test for the analysis DatatypeCheck.
 *
 * Description:
 * Performs a struct creation with an unknown datatype, what results in an error
 *
 *
 *  @date 24.05.2011
 *  @author Joachim Protze
 */

#include <iostream>
#include <mpi.h>
#include <assert.h>
#include "mustTest.h"

int main (int argc, char** argv)
{
    int size, rank;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    MPI_Datatype oldtypes[] = {MPI_DATATYPE_NULL,MPI_DATATYPE_NULL,MPI_DATATYPE_NULL,MPI_DATATYPE_NULL,MPI_DATATYPE_NULL}, type1, type2;
    MPI_Aint displs[] =  {10,20,30,40,50};
    int blocklens[] = {1,1,1,1,1}, i;

    //Say hello
    std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    if (rank == 0)
    {
        MPI_Type_contiguous(2, MPI_INT, &type1);
        for (i=0; i<5; i++)
            oldtypes[i]=type1;
        MPI_Type_free(&type1);
        assert(type1 == MPI_DATATYPE_NULL);
        MPI_Type_struct(5, blocklens, displs, oldtypes, &type2);
        MPI_Type_free(&type2);
    }
    std::cout << "Signing off, rank " << rank << "." << std::endl;

    MPI_Finalize ();

    return 0;
}
