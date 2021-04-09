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
 * @file example_4_6.c
 * A must datatype test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypeTest.h"

int main (int argc, char** argv)
{

    int fType;
    MPI_Init(&argc,&argv);

    MPI_Datatype        type1, newtype;
    int                 blocklens[3] = {1,1,3};
    MPI_Aint            displs[3] = {0,8,26};
    MPI_Datatype        types[3] = {MPI_DOUBLE, MPI_CHAR, MPI_CHAR};

    MPI_Type_struct (2, blocklens, displs, types, &type1);
    test_type (type1);


    blocklens[0] = 2;
    blocklens[1] = 1;
    displs[0] = 0;
    displs[1] = 16;
    types[0] = MPI_FLOAT;
    types[1] = type1;
    MPI_Type_struct (3, blocklens, displs, types, &newtype);
    test_type (newtype);

    MPI_Finalize ();

    return 0;
}
