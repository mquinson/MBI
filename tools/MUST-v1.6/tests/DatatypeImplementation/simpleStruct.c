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
 * @file simpleStruct.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    MPI_Init(&argc,&argv);

    MPI_Datatype        structtype;

    int blocklens[] = {2, 3, 2, 3};
    MPI_Aint indices[] = {0, 20, 40, 60};
    MPI_Datatype types[] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Type_struct (4, blocklens, indices, types, &structtype);

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(structtype);

    MPI_Type_free(&structtype);

    MPI_Finalize ();

    return 0;
}
