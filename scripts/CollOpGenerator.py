#! /usr/bin/python3
import sys
from generator_utils import make_file

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

BEGIN_MPI_FEATURES
	P2P!basic: Lacking
	P2P!nonblocking: Lacking
	P2P!persistent: Lacking
	P2P!probe: Lacking
	COLL!basic: @{collfeature}@
	COLL!nonblocking: @{icollfeature}@
	COLL!persistent: Lacking
	COLL!probe: Lacking
	COLL!tools: Lacking
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

#define buff_size 128


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  MPI_Op op = MPI_SUM;
  @{change_op}@

  @{init}@
  @{operation}@ /* MBIERROR */
	@{fini}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

collectives = ['MPI_Reduce', 'MPI_Allreduce']
icollectives = ['MPI_Ireduce'] 

init = {}
operation = {}
fini = {}

init['MPI_Reduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Reduce'] = lambda n: f"MPI_Reduce(&sum{n}, &val{n}, 1, MPI_INT, op, 0, MPI_COMM_WORLD);"
fini['MPI_Reduce'] = lambda n: ""

init['MPI_Allreduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Allreduce'] = lambda n: f"MPI_Allreduce(&sum{n}, &val{n}, 1, MPI_INT, op, MPI_COMM_WORLD);"
fini['MPI_Allreduce'] = lambda n: ""

init['MPI_Ireduce'] = lambda n: f"MPI_Request req{n}; MPI_Status sta{n}; int sum{n}, val{n} = 1;"
operation['MPI_Ireduce'] = lambda n: f"MPI_Ireduce(&sum{n}, &val{n}, 1, MPI_INT, op, 0, MPI_COMM_WORLD, &req{n}); MPI_Wait(&req{n},&sta{n});"
fini['MPI_Ireduce'] = lambda n: ""


for coll in collectives + icollectives:
    patterns = {}
    patterns = {'coll': coll}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
    patterns['collfeature'] = 'Yes' if coll in collectives else 'Lacking'
    patterns['icollfeature'] = 'Yes' if coll in icollectives else 'Lacking'
    patterns['coll'] = coll
    patterns['init'] = init[coll]("1")
    patterns['fini'] = fini[coll]("1")
    patterns['operation'] = operation[coll]("1")

    # Generate the incorrect matching
    replace = patterns
    replace['shortdesc'] = 'Collective @{coll}@ with an operator  mismatch'
    replace['longdesc'] = f'Odd ranks use MPI_SUM as the operator while even ranks use MPI_MAX'
    replace['outcome'] = 'ERROR: OperatorMatching'
    replace['errormsg'] = 'Collective operator mistmatch. @{coll}@ at @{filename}@:@{line:MBIERROR}@ has MPI_MAX or MPI_SUM as an operator.'
    replace['change_op'] = 'if (rank % 2)\n    op = MPI_MAX;'
    make_file(template, f'CollOpMatching_{coll}_nok.c', replace)

    # Generate the call with Op=MPI_OP_NULL
    replace = patterns
    replace['shortdesc'] = 'Collective @{coll}@ with an invalid operator '
    replace['longdesc'] = 'Collective @{coll}@ with an invalid operator ' 
    replace['outcome'] = 'ERROR: InvalidOperator'
    replace['errormsg'] = 'Invalid Operator. @{coll}@ at @{filename}@:@{line:MBIERROR}@ has MPI_OP_NULL as an operator.'
    replace['change_op'] = 'op = MPI_OP_NULL;'
    make_file(template, f'CollOpNull_{coll}_nok.c', replace)
