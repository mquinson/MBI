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

#define ITERATIONS 100
#define PARAM_PER_ITERATION 3
#define PARAM_LOST_PER_ITERATION 1

void myOp(int *invec, int *inoutvec, int *len, MPI_Datatype *dtype) {
  for (int i = 0; i < *len; i++)
    inoutvec[i] += invec[i];
}

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
	int i=1, j=1, size=1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

	@{change_size}@
  @{init}@
	@{loop}@	
  @{operation}@ 
	@{cond}@	
	@{fini}@
	@{end}@	

	@{free}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

callparameter = ['MPI_Op_create', 'MPI_Comm_group', 'MPI_Comm_dup', 'MPI_Type_contiguous', 'MPI_Comm_create', 'MPI_Group_excl', 'MPI_Comm_split']

init = {}
operation = {}
fini = {}
error = {}
free = {}

init['MPI_Op_create'] = lambda n: 'MPI_Op op[size];'
operation['MPI_Op_create'] = lambda n: 'MPI_Op_create((MPI_User_function *)myOp, 0, &op[j]);'
error['MPI_Op_create'] = 'OperatorLeak'
fini['MPI_Op_create'] = lambda n: "MPI_Op_free(&op[j]);"
free['MPI_Op_create'] = lambda n: ""

init['MPI_Comm_group'] = lambda n: 'MPI_Group grp[size];'
operation['MPI_Comm_group'] = lambda n: 'MPI_Comm_group(MPI_COMM_WORLD, &grp[j]);'
error['MPI_Comm_group'] = 'GroupLeak'
fini['MPI_Comm_group'] = lambda n: "MPI_Group_free(&grp[j]);"
free['MPI_Comm_group'] = lambda n: "" 

init['MPI_Group_excl'] = lambda n: 'MPI_Group worldgroup, grp[size];\n MPI_Comm_group(MPI_COMM_WORLD, &worldgroup);'
operation['MPI_Group_excl'] = lambda n: 'MPI_Group_excl(worldgroup, 1, &rank, &grp[j]);' 
error['MPI_Group_excl'] = 'GroupLeak'
fini['MPI_Group_excl'] = lambda n: "MPI_Group_free(&grp[j]);"
free['MPI_Group_excl'] = lambda n: "MPI_Group_free(&worldgroup);"

init['MPI_Comm_create'] = lambda n: 'MPI_Comm com[size]; MPI_Group grp[size];'
operation['MPI_Comm_create'] = lambda n: 'MPI_Comm_group(MPI_COMM_WORLD, &grp[j]);\n MPI_Comm_create(MPI_COMM_WORLD, grp[j], &com[j]);\n MPI_Group_free(&grp[j]);'
error['MPI_Comm_create'] = 'CommunicatorLeak'
fini['MPI_Comm_create'] = lambda n: "MPI_Comm_free(&com[j]);"
free['MPI_Comm_create'] = lambda n: ""

init['MPI_Comm_dup'] = lambda n: f'MPI_Comm com[size];'
operation['MPI_Comm_dup'] = lambda n: 'MPI_Comm_dup(MPI_COMM_WORLD, &com[j]);'
error['MPI_Comm_dup'] = 'CommunicatorLeak'
fini['MPI_Comm_dup'] = lambda n: "MPI_Comm_free(&com[j]);"
free['MPI_Comm_dup'] = lambda n: "" 

init['MPI_Comm_split'] = lambda n: f'MPI_Comm com[size]; int color = rank % 2; int key = 1;'
operation['MPI_Comm_split'] = lambda n: 'MPI_Comm_split(MPI_COMM_WORLD,color,key, &com[j]);'
error['MPI_Comm_split'] = 'CommunicatorLeak'
fini['MPI_Comm_split'] = lambda n: "MPI_Comm_free(&com[j]);"
free['MPI_Comm_split'] = lambda n: "" 

init['MPI_Type_contiguous'] = lambda n: 'MPI_Datatype type[size];'
operation['MPI_Type_contiguous'] = lambda n: 'MPI_Type_contiguous(2, MPI_DOUBLE, &type[j]);'
error['MPI_Type_contiguous'] = 'TypeLeak'
fini['MPI_Type_contiguous'] = lambda n: "MPI_Type_free(&type[j]);"
free['MPI_Type_contiguous'] = lambda n: "" 

# Generate code with one collective
for call in callparameter:
    patterns = {}
    patterns = {'call': call}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
    patterns['toolfeature'] = 'Yes' 
    patterns['call'] = call
    patterns['operation'] = operation[call]("1")
    patterns['init'] = init[call]("1")
    patterns['fini'] = fini[call]("1")
    patterns['free'] = free[call]("1")
    missing = patterns['fini']
    patterns['loop'] = ''
    patterns['cond'] = ''
    patterns['change_size'] = ''
    patterns['end'] = ''

    # Generate the correct code
    replace = patterns
    replace['shortdesc'] = '@{call}@ is correctly used' 
    replace['longdesc'] = f'{call} correctly used' 
    replace['outcome'] = 'OK'
    replace['errormsg'] = ''
    make_file(template, f'CallParamCorrect_{call}.c', replace)

    # Generate the resleak
    replace = patterns
    replace['shortdesc'] = '@{call}@ has no free'
    replace['longdesc'] = '@{call}@ has no free'
    replace['outcome'] = f'ERROR: {error[call]}'
    replace['errormsg'] = 'Resleak. @{call}@ at @{filename}@:@{line:MBIERROR}@ has no free.'
    replace['fini'] = ' /* MBIERROR MISSING: ' + missing + ' */'
    make_file(template, f'Resleak_{call}_nok.c', replace)

    # Generate multiple resleak
    replace = patterns
    replace['shortdesc'] = '@{call}@ lacks several free'
    replace['longdesc'] = '@{call}@ lacks several free'
    replace['outcome'] = f'ERROR: {error[call]}'
    replace['errormsg'] = 'Resleak. @{call}@ at @{filename}@:@{line:MBIERROR}@ lacks several free.'
    replace['change_size'] = 'size=PARAM_PER_ITERATION;'
    replace['loop'] = 'for (i = 0; i < ITERATIONS; i++) {\n		for (j = 0; j < PARAM_PER_ITERATION; j++) {'
    replace['cond'] = '			if (j < PARAM_PER_ITERATION - PARAM_LOST_PER_ITERATION) {'
    replace['fini'] = fini[call]("1") + ' /* MBIERROR */'
    replace['end'] = '			}\n 		}\n 	}'
    make_file(template, f'Resleak_multiple_{call}_nok.c', replace)
