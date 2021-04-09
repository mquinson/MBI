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
 * @file datatype.c
 * A must datatype test.
 * Contains no errors.
 *
 * @author Tobias Hilbrich, Joachim Protze
 */

#include "datatypeTest.h"

typedef struct
{
    double d[2];
    unsigned long long l;
} myStruct;

int main (int argc, char** argv)
{
    int rank,size;
    int fType;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //A basic type
    test_btype (MPI_INT);

    // Two user defined types
    // the MPI_Type_struct represents the above myStruct
    myStruct temp;
    MPI_Datatype        t1, t2;
    int                 blocklens[2] = {1,1};
    MPI_Aint            displs[2];
    MPI_Datatype        types[2];

    MPI_Address (&(temp.d[0]), &(displs[0]));
    MPI_Address (&(temp.l), &(displs[1]));
    displs[1] = displs[1] - displs[0];
    displs[0] = 0;

    MPI_Type_contiguous (2, MPI_DOUBLE, &t1);
    test_type (t1);

    types[0] = t1;
    types[1] = MPI_UNSIGNED_LONG_LONG;
    MPI_Type_struct (2, blocklens, displs, types, &t2);
    test_type (t2);

    printf ("Hello, I am %d of %d tasks.\n", rank, size);

    MPI_Finalize ();

    return 0;
}
