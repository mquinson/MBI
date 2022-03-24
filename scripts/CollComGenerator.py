#! /usr/bin/python3
import os
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

   Version of MPI: Conforms to MPI 1.1, does not require MPI 2 implementation

BEGIN_MPI_FEATURES
  P2P!basic: Lacking
  P2P!nonblocking: Lacking
  P2P!persistent: Lacking
  COLL!basic: @{collfeature}@
  COLL!nonblocking: @{icollfeature}@
  COLL!persistent: Lacking
  COLL!tools: Yes
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
  int root = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  MPI_Op op = MPI_SUM;
  MPI_Datatype type = MPI_INT;
  MPI_Comm newcom;
  MPI_Comm_split(MPI_COMM_WORLD, 0, nprocs - rank, &newcom);

  @{change_com}@

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  @{init}@
  @{start}@
  @{operation}@ /* MBIERROR */
  @{fini}@
  @{free}@

  if(newcom != MPI_COMM_NULL && newcom != MPI_COMM_WORLD)
    MPI_Comm_free(&newcom);

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


# Generate code with one collective
for c in coll + icoll + ibarrier:
    patterns = {}
    patterns = {'c': c}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
    patterns['collfeature'] = 'Yes' if c in coll else 'Lacking'
    patterns['icollfeature'] = 'Yes' if c in icoll + ibarrier else 'Lacking'
    patterns['c'] = c
    patterns['init'] = init[c]("1")
    patterns['start'] = start[c]("1")
    patterns['fini'] = fini[c]("1")
    patterns['free'] = free[c]("1")
    patterns['operation'] = operation[c]("1")

    # Generate the correct code => to remove?
    replace = patterns
    replace['shortdesc'] = 'Collective @{c}@ with correct arguments'
    replace['longdesc'] = f'All ranks in newcom call {c} with correct arguments'
    replace['outcome'] = 'OK'
    replace['errormsg'] = ''
    replace['change_com'] = '/* No error injected here */'
    make_file(template, f'ParamMatching_Com_{c}_ok.c', replace)

    # Generate the incorrect communicator matching
    replace = patterns
    replace['shortdesc'] = 'Collective @{c}@ with a communicator mismatch'
    replace['longdesc'] = f'Odd ranks call the collective on newcom while even ranks call the collective on MPI_COMM_WORLD'
    replace['outcome'] = 'ERROR: CommunicatorMatching'
    replace['errormsg'] = 'Communicator mistmatch in collectives. @{c}@ at @{filename}@:@{line:MBIERROR}@ has newcom or MPI_COMM_WORLD as a communicator.'
    replace['change_com'] = 'if (rank % 2)\n    newcom = MPI_COMM_WORLD; /* MBIERROR */'
    make_file(template, f'ParamMatching_Com_{c}_nok.c', replace)

    # Generate the coll with newcom=MPI_COMM_NULL
    replace = patterns
    replace['shortdesc'] = f'Collective @{c}@ with newcom=MPI_COMM_NULL'
    replace['longdesc'] = f'Collective @{c}@ with newcom=MPI_COMM_NULL'
    replace['outcome'] = 'ERROR: InvalidCommunicator'
    replace['errormsg'] = 'Invalid communicator. @{c}@ at @{filename}@:@{line:MBIERROR}@ has MPI_COMM_NULL as a communicator.'
    replace['change_com'] = 'newcom = MPI_COMM_NULL; /* MBIERROR */'
    make_file(template, f'InvalidParam_ComNull_{c}_nok.c', replace)
