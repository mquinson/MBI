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
 * @file simpleSubarray.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    MPI_Init(&argc,&argv);

    MPI_Datatype        subarraytype;

    int sizes[] = {5,5,5};
    int subsizes[] = {2,2,2};
    int starts[] = {1,2,3};

    MPI_Type_create_subarray (3, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &subarraytype);

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(subarraytype);

    MPI_Type_free(&subarraytype);

    MPI_Finalize ();

    return 0;
}
