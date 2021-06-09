#! /usr/bin/python3
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

BEGIN_MPI_FEATURES
	P2P!basic: Lacking
	P2P!nonblocking: Lacking
	P2P!persistent: Lacking
	COLL!basic: Lacking 
	COLL!nonblocking: Lacking 
	COLL!persistent: Lacking
	COLL!tools: @{toolfeature}@
	RMA: Lacking
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

#define buff_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
	int color = 10;
	MPI_Comm newcom;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

	if(rank == 1) {
  	@{operation1}@ 
	}else{
    @{change_color}@ /* MBIERROR */
  	@{operation2}@ 
	}

	if(newcom != MPI_COMM_NULL)
		MPI_Comm_free(&newcom);

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

# this code is for MPI_Comm_split

for c in tcoll4color: 
    patterns = {}
    patterns = {'c': c}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
    patterns['toolfeature'] = 'Yes' if c in tcoll4color else 'Lacking'
    patterns['c'] = c
    patterns['operation1'] = operation[c]("1")
    patterns['operation2'] = operation[c]("2")

    # Generate the code with invalid color
    replace = patterns
    replace['shortdesc'] = 'Invalid color in @{c}@'
    replace['longdesc'] = f'invalid color in @{c}@'
    replace['outcome'] = 'ERROR: InvalidOtherArg'
    replace['errormsg'] = 'Invalid Argument in collective. @{c}@ has an invalid color (see line @{line:MBIERROR}@)'
    replace['change_color'] = 'color=-10;'
    make_file(template, f'CollInvalidOtherArg_{c}_nok.c', replace)
