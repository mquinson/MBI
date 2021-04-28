#! /usr/bin/python3
import sys
from generator_utils import make_file

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

BEGIN_MPI_FEATURES
	P2P!basic: @{p2pfeature}@ 
	P2P!nonblocking: @{ip2pfeature}@
	P2P!persistent: Lacking
	P2P!probe: Lacking
	COLL!basic: Lacking
	COLL!nonblocking: Lacking
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

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  @{init1}@
  @{init2}@
	if (rank == 0) {
  	@{operation1}@ /* MBIERROR1 */
	}if (rank == 1) {
  	@{operation2}@ /* MBIERROR2 */
	}
	@{fini1}@
	@{fini2}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

p2p = ['MPI_Send', 'MPI_Recv'] 
ip2p = ['MPI_Isend']  

init = {}
operation = {}
fini = {}

init['MPI_Send'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Send'] = lambda n: f'MPI_Send(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD);'
fini['MPI_Send'] = lambda n: ""

init['MPI_Recv'] = lambda n: f'int buf{n}[buff_size]; MPI_Status sta;'
operation['MPI_Recv'] = lambda n: f'MPI_Recv(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &sta);'
fini['MPI_Recv'] = lambda n: ""

init['MPI_Isend'] = lambda n: f'int buf{n}[buff_size]; MPI_Request req{n};'
operation['MPI_Isend'] = lambda n: f'MPI_Isend(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req{n});'
fini['MPI_Isend'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'

for p1 in p2p + ip2p:
    patterns = {}
    patterns = {'p1': p1}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
    patterns['p2pfeature'] = 'Correct' if p1 in p2p else 'Lacking'
    patterns['ip2pfeature'] = 'Correct' if p1 in ip2p else 'Lacking'
    patterns['p1'] = p1
    patterns['init1'] = init[p1]("1")
    patterns['init2'] = '' #init[p2]("2")
    patterns['fini1'] = fini[p1]("1")
    patterns['fini2'] = '' #fini[p2]("2")
    patterns['operation1'] = operation[p1]("1")
    patterns['operation2'] = '' #operation[p2]("2")

    # Generate the incorrect matching
    replace = patterns
    replace['shortdesc'] = 'Point to point @{p1}@ is not matched' 
    replace['longdesc'] = f'Process 0 calls @{p1}@ and is not matched'
    replace['outcome'] = 'ERROR: CallMatching'
    replace['errormsg'] = 'P2P mistmatch. @{p1}@ at @{filename}@:@{line:MBIERROR}@ is not matched.'
    make_file(template, f'P2PCallMatching_{p1}_nok.c', replace)

