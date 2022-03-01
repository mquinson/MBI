#! /usr/bin/python3
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: @{origin}@ 

  Description: @{shortdesc}@
    @{longdesc}@


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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10

int main(int argc, char **argv) {
  int rank, numProcs;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int *winbuf = malloc(N * sizeof(int));

	MPI_Win win;
  MPI_Win_create(&winbuf, N * sizeof(int), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	MPI_Datatype type = MPI_INT;
	int target = (rank + 1) % numProcs; 

	if(rank == 0){
  	@{epoch}@
		@{change_arg}@
		@{init}@ 
 		@{operation}@ /* MBIERROR2 */

  	@{finEpoch}@
	} else {
  	@{epoch}@

  	@{finEpoch}@
	}

  MPI_Win_free(&win);

	free(winbuf);

  MPI_Finalize();
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

        # Generate a code with a null type 
        replace = patterns 
        replace['shortdesc'] = 'Invalid argument in one-sided operation.' 
        replace['longdesc'] = 'A one-sided operation has MPI_DATATYPE_NULL as a type.'
        replace['outcome'] = 'ERROR: InvalidDatatype' 
        replace['change_arg'] = 'type = MPI_DATATYPE_NULL;' 
        replace['errormsg'] = '@{p}@ at @{filename}@:@{line:MBIERROR}@ has MPI_DATATYPE_NULL as a type'
        make_file(template, f'InvalidParam_BufferNullCond_{e}_{p}_nok.c', replace)

        # Generate a code with an invalid type 
        replace = patterns 
        replace['shortdesc'] = 'Invalid argument in one-sided operation.' 
        replace['longdesc'] = 'Use of an invalid datatype in one-sided operation.'
        replace['outcome'] = 'ERROR: InvalidDatatype' 
        replace['change_arg'] = 'MPI_Type_contiguous (2, MPI_INT, &type); MPI_Type_commit(&type);MPI_Type_free(&type); /* MBIERROR2 */' 
        replace['errormsg'] = 'Invalid Datatype in @{p}@ at @{filename}@:@{line:MBIERROR}@'
        make_file(template, f'InvalidParam_DatatypeCond_{e}_{p}_nok.c', replace)

