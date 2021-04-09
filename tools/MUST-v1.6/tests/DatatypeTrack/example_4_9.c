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
 * @file example_4_9.c
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
    int                 blocklens[3] = {1,1,1};
    MPI_Aint            displs[3] = {-3,0,6};

#ifdef HAVE_MPI_TYPE_CREATE_RESIZED
    MPI_Type_create_resized (MPI_INT, displs[0], displs[2], &type1);
#else
    MPI_Datatype        types[3] = {MPI_LB, MPI_INT, MPI_UB};
    MPI_Type_struct (3, blocklens, displs, types, &type1);
#endif
    test_type (type1);

    MPI_Type_contiguous (2, type1, &newtype);
    test_type (newtype);

    MPI_Finalize ();

    return 0;
}
