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
 * @file treeRecursion4.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    MPI_Init(&argc,&argv);

    MPI_Datatype        structtype[4];

    int i,j;
    int blocklens[] = {3, 2, 1};
    MPI_Aint indices[] = {12, 24, 32}, extent;
    MPI_Datatype types[] = {MPI_INT, MPI_INT, MPI_INT};

    for (j=0; j<4; j++)
    {
        for (i=1; i<3; i++)
        {
            MPI_Type_extent(types[i-1], &extent);
            indices[i] = indices[i-1] + blocklens[i-1] * extent + 8;
        }
        MPI_Type_struct (3, blocklens, indices, types, &structtype[j]);
        for (i=1; i<3; i++)
        {
            types[i-1] = types[i];
        }
        types[2] = structtype[j];
    }

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(structtype[3]);

    for (j=0; j<4; j++)
    {
        MPI_Type_free(&structtype[j]);
    }

    MPI_Finalize ();

    return 0;
}
