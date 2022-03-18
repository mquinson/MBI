#! /usr/bin/python3
import os
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

	Version of MPI: Conforms to MPI 2, requires MPI 3 implementation (for lock_all/unlock_all epochs)

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

#define N 1

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
	MPI_Win win;
  int winbuf[100] = {0};

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

	MPI_Datatype type = MPI_INT;
	int target = 1;

  MPI_Win_create(&winbuf, 100 * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

	@{init1}@ 

  @{epoch}@

	if (rank == 0 || rank == 2) {
  	@{operation1}@ /* MBIERROR1 */
	}

  @{finEpoch}@

  MPI_Win_free(&win);

  MPI_Finalize();
  return 0;
}
"""


for e in epoch:
    for p1 in get + put: 
        patterns = {}
        patterns = {'e': e, 'p1': p1}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'  
        patterns['rmafeature'] = 'Yes'
        patterns['p1'] = p1 
        patterns['e'] = e 
        patterns['epoch'] = epoch[e]("1") 
        patterns['finEpoch'] = finEpoch[e]("1") 
        patterns['init1'] = init[p1]("1") 
        patterns['operation1'] = operation[p1]("1") 

    		# Generate a data race (Get + Get/load/store/Put)
        replace = patterns 
        replace['shortdesc'] = 'Global Concurrency error.' 
        replace['longdesc'] = 'Global Concurrency error. Both processes 0 and 2 access the window in process 1 with @{p1}@'
        replace['outcome'] = 'ERROR: GlobalConcurrency' 
        replace['errormsg'] = 'Global Concurrency error. @{p1}@ at @{filename}@:@{line:MBIERROR1}@ conflicts in process 1'
        make_file(template, f'GlobalConcurrency_rr_{e}_{p1}_nok.c', replace)

