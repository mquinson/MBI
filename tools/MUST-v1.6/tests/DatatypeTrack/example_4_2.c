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
 * @file example_4_2.c
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

    MPI_Datatype        oldtype, newtype;
    int                 blocklens[2] = {1,1};
    MPI_Aint            displs[2] = {0,8};
    MPI_Datatype        types[2] = {MPI_DOUBLE, MPI_CHAR};

    MPI_Type_struct (2, blocklens, displs, types, &oldtype);
    test_type (oldtype);


    MPI_Type_contiguous (3, oldtype, &newtype);
    test_type (newtype);

    MPI_Finalize ();

    return 0;
}
