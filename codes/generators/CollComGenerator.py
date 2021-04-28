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

	MPI_Comm newcom;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  MPI_Comm_split(MPI_COMM_WORLD, 0, nprocs - rank, &newcom);

  @{change_com}@

	int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  @{init}@
  @{operation}@ 
	@{fini}@

	if(newcom != MPI_COMM_NULL)
		MPI_Comm_free(&newcom);

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

collectives = ['MPI_Bcast', 'MPI_Barrier', 'MPI_Reduce', 'MPI_Gather', 'MPI_Scatter', 'MPI_Scan', 'MPI_Exscan', 'MPI_Allgather', 'MPI_Allreduce']
icollectives = ['MPI_Ibarrier']  # 'ibarrier', 'ireduce']

init = {}
operation = {}
fini = {}

init['MPI_Bcast'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Bcast'] = lambda n: f'MPI_Bcast(buf{n}, buff_size, MPI_INT, 0, newcom);'
fini['MPI_Bcast'] = lambda n: ""

init['MPI_Barrier'] = lambda n: ""
operation['MPI_Barrier'] = lambda n: 'MPI_Barrier(newcom);'
fini['MPI_Barrier'] = lambda n: ""

init['MPI_Ibarrier'] = lambda n: f"MPI_Request req{n};MPI_Status stat{n};"
operation['MPI_Ibarrier'] = lambda n: f'MPI_Ibarrier(newcom, &req{n});'
fini['MPI_Ibarrier'] = lambda n: f"MPI_Wait(&req{n}, &stat{n});"

init['MPI_Reduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Reduce'] = lambda n: f"MPI_Reduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, newcom);"
fini['MPI_Reduce'] = lambda n: ""

init['MPI_Gather'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Gather'] = lambda n: f"MPI_Gather(&val{n}, 1, MPI_INT, buf{n},1, MPI_INT, 0, newcom);"
fini['MPI_Gather'] = lambda n: ""

init['MPI_Scatter'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Scatter'] = lambda n: f"MPI_Scatter(&buf{n}, 1, MPI_INT, &val{n}, 1, MPI_INT, 0, newcom);"
fini['MPI_Scatter'] = lambda n: ""

init['MPI_Allreduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Allreduce'] = lambda n: f"MPI_Allreduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, newcom);"
fini['MPI_Allreduce'] = lambda n: ""

init['MPI_Scan'] = lambda n: f"int outbuf{n}[buff_size], inbuf{n}[buff_size];"
operation['MPI_Scan'] = lambda n: f"MPI_Scan(&outbuf{n}, inbuf{n}, buff_size, MPI_INT, MPI_SUM, newcom);"
fini['MPI_Scan'] = lambda n: ""

init['MPI_Exscan'] = lambda n: f"int outbuf{n}[buff_size], inbuf{n}[buff_size];"
operation['MPI_Exscan'] = lambda n: f"MPI_Exscan(&outbuf{n}, inbuf{n}, buff_size, MPI_INT, MPI_SUM, newcom);"
fini['MPI_Exscan'] = lambda n: ""

init['MPI_Allgather'] = lambda n: f"int *rbuf{n} = malloc(dbs);"
operation['MPI_Allgather'] = lambda n: f"MPI_Allgather(&rank, 1, MPI_INT, rbuf{n}, 1, MPI_INT, newcom);"
fini['MPI_Allgather'] = lambda n: f"free(rbuf{n});"

init['MPI_Allreduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Allreduce'] = lambda n: f"MPI_Allreduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, newcom);"
fini['MPI_Allreduce'] = lambda n: ""

# Generate code with one collective
for coll in collectives + icollectives:
    patterns = {}
    patterns = {'coll': coll}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
    patterns['collfeature'] = 'Correct' if coll in collectives else 'Lacking'
    patterns['icollfeature'] = 'Correct' if coll in icollectives else 'Lacking'
    patterns['coll'] = coll
    patterns['init'] = init[coll]("1")
    patterns['fini'] = fini[coll]("1")
    patterns['operation'] = operation[coll]("1")

    # Generate the correct code => to remove?
    replace = patterns
    replace['shortdesc'] = 'Collective @{coll}@ with correct arguments'
    replace['longdesc'] = f'All ranks in newcom call {coll} with correct arguments'
    replace['outcome'] = 'OK'
    replace['errormsg'] = ''
    replace['change_com'] = '/* No error injected here */'
    make_file(template, f'CollCorrect_{coll}.c', replace)

    # Generate the incorrect matching
    replace = patterns
    replace['shortdesc'] = 'Collective @{coll}@ with a communicator mismatch'
    replace['longdesc'] = f'Odd ranks call the collective on newcom while even ranks call the collective on MPI_COMM_WORLD'
    replace['outcome'] = 'ERROR: CommunicatorMatching'
    replace['errormsg'] = 'Communicator mistmatch in collectives. @{coll}@ at @{filename}@:@{line:MBIERROR}@ has newcom or MPI_COMM_WORLD as a communicator.'
    replace['change_com'] = 'if (rank % 2)\n    newcom = MPI_COMM_WORLD; /* MBIERROR */'
    make_file(template, f'CollComMatching_{coll}_nok.c', replace)

    # Generate the coll with newcom=MPI_COMM_NULL
    replace = patterns
    replace['shortdesc'] = f'Collective @{coll}@ with newcom=MPI_COMM_NULL' 
    replace['longdesc'] = f'Collective @{coll}@ with newcom=MPI_COMM_NULL'
    replace['outcome'] = 'ERROR: InvalidCommunicator'
    replace['errormsg'] = 'Invalid communicator. @{coll}@ at @{filename}@:@{line:MBIERROR}@ has MPI_COMM_NULL as a communicator.'
    replace['change_com'] = 'newcom = MPI_COMM_NULL; /* MBIERROR */'
    make_file(template, f'CollComNull_{coll}_nok.c', replace)
