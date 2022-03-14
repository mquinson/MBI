#! /usr/bin/python3
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: @{origin}@

  Description: @{shortdesc}@
    @{longdesc}@

    Version of MPI: Conforms to MPI 1.1, does not require MPI 2 implementation

BEGIN_MPI_FEATURES
    P2P!basic: Lacking
    P2P!nonblocking: Lacking
    P2P!persistent: Lacking
    COLL!basic: Lacking
    COLL!nonblocking: Lacking
    COLL!persistent: Lacking
    COLL!tools: Lacking
    RMA: @{rmafeature}@
END_MPI_FEATURES

BEGIN_MBI_TESTS
  $ mpirun -np 2 ${EXE}
  | @{outcome}@
  | @{errormsg}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10

int main(int argc, char **argv) {
  int nprocs = -1 , rank = -1;
  MPI_Win win;
  int *winbuf = @{malloc}@ // Window buffer

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  MPI_Datatype type = MPI_INT;
  int target = (rank + 1) % nprocs;

  MPI_Win_create(winbuf, N * sizeof(int), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);


  @{epoch}@

  @{init}@
  @{change_arg}@
  @{operation}@ /* MBIERROR */

  @{finEpoch}@

  MPI_Win_free(&win);
  free(winbuf);

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


for e in epoch:
    for p in rma:
        patterns = {}
        patterns = {'e': e, 'p': p}
        patterns['origin'] = "MBI"
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
        patterns['rmafeature'] = 'Yes'
        patterns['p'] = p
        patterns['e'] = e
        patterns['epoch'] = epoch[e]("1")
        patterns['finEpoch'] = finEpoch[e]("1")
        patterns['init'] = init[p]("1")
        patterns['operation'] = operation[p]("1")
        patterns['change_arg'] = ""
        patterns['malloc'] = "malloc(N * sizeof(int));"

        # Generate a code with a null type
        replace = patterns
        replace['shortdesc'] = 'Invalid argument in one-sided operation.'
        replace['longdesc'] = 'A one-sided operation has MPI_DATATYPE_NULL as a type.'
        replace['outcome'] = 'ERROR: InvalidDatatype'
        replace['change_arg'] = 'type = MPI_DATATYPE_NULL;'
        replace['errormsg'] = '@{p}@ at @{filename}@:@{line:MBIERROR}@ has MPI_DATATYPE_NULL as a type'
        make_file(template, f'InvalidParam_DatatypeNull_{e}_{p}_nok.c', replace)

        # Generate a code with a null buffer
        replace = patterns
        replace['origin'] = 'MPI-Corrbench'
        replace['shortdesc'] = 'nullptr is invalid in one-sided operation.'
        replace['longdesc'] = 'A one-sided operation has an invalid buffer.'
        replace['outcome'] = 'ERROR: InvalidBuffer'
        replace['init'] = 'int * localbuf1 = malloc(sizeof(int));'
        replace['change_arg'] = 'localbuf1 = NULL;'
        replace['operation'] = operation[p]("1").replace('&localbuf1', 'localbuf1')
        replace['errormsg'] = '@{p}@ at @{filename}@:@{line:MBIERROR}@ has an invalid buffer'
        make_file(template, f'InvalidParam_BufferNull_{e}_{p}_nok.c', replace)

        # Generate a code with an invalid type
        replace = patterns
        replace['origin'] = 'MBI'
        replace['shortdesc'] = 'Invalid argument in one-sided operation.'
        replace['longdesc'] = 'Use of an invalid datatype in one-sided operation.'
        replace['outcome'] = 'ERROR: InvalidDatatype'
        replace['change_arg'] = 'MPI_Type_contiguous (2, MPI_INT, &type); MPI_Type_commit(&type);MPI_Type_free(&type); /* MBIERROR2 */'
        replace['errormsg'] = 'Invalid Datatype in @{p}@ at @{filename}@:@{line:MBIERROR}@'
        make_file(template, f'InvalidParam_Datatype_{e}_{p}_nok.c', replace)

        # Generate a code with invalid buffer
        replace = patterns
        patterns['origin'] = "MPI-Corrbench"
        replace['shortdesc'] = 'Invalid invalid buffer (buffer must be allocated)'
        replace['longdesc'] = 'Use of an invalid buffer in MPI_Win_create.'
        replace['outcome'] = 'ERROR: InvalidBuffer'
        patterns['malloc'] = "NULL; /* MBIERROR2 */"
        patterns['operation'] = ""
        replace['change_arg'] = ""
        replace['errormsg'] = 'Invalid buffer in Win_create at @{filename}@:@{line:MBIERROR2}@'
        make_file(template, f'InvalidParam_InvalidBufferWinCreate_{e}_{p}_nok.c', replace)
