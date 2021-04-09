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
 * @file mpi1Datatypetest.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    int fType;
    MPI_Init(&argc,&argv);

    MPI_Datatype        vectortype, structtype, contitype;
    
    MPI_Type_vector (10, 1, 5, MPI_INT, &vectortype);
    
    int                 blocklens[3] = {1,1,1};
    MPI_Aint            displs[3] = {0,0,4};
    MPI_Datatype        types[3] = {MPI_LB, vectortype, MPI_UB};

    MPI_Type_extent(MPI_INT, displs+2);

    MPI_Type_struct (3, blocklens, displs, types, &structtype);

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(structtype);

    MPI_Type_free(&vectortype);
    MPI_Type_free(&structtype);

    MPI_Finalize ();

    return 0;
}
