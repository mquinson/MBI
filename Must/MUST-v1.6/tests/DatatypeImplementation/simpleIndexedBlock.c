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
 * @file simpleIndexedBlock.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    MPI_Init(&argc,&argv);

    MPI_Datatype        indexedtype;

    int indices[] = {0, 4, 10, 14, 20};

    MPI_Type_create_indexed_block (4, 2, indices, MPI_INT, &indexedtype);

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(indexedtype);

    MPI_Type_free(&indexedtype);

    MPI_Finalize ();

    return 0;
}
