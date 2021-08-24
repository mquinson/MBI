#! /usr/bin/python3
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

BEGIN_MPI_FEATURES
	P2P!basic: @{p2pfeature}@
	P2P!nonblocking: @{ip2pfeature}@
	P2P!persistent: Lacking
	COLL!basic: Lacking
	COLL!nonblocking: Lacking
	COLL!persistent: Lacking
	COLL!tools: Lacking
	RMA: @{rmafeature}@
END_MPI_FEATURES

BEGIN_MBI_TESTS
  $ mpirun -np 4 ${EXE}
  | @{outcome}@
  | @{errormsg}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
	MPI_Win win;
  int W; // Window buffer
  int NUM_ELEMT=1;
	int buff_size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 4)
    printf("MBI ERROR: This test needs at least 4 processes to produce a bug!\\n");

	MPI_Comm newcom = MPI_COMM_WORLD;
	MPI_Datatype type = MPI_INT;
	int stag=0, rtag=0;
  W = nprocs;

  MPI_Win_create(&W, NUM_ELEMT * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);


	@{init1}@
	@{init2}@
	@{init3}@

	if (rank == 0) {
		int target=1;
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 1, 0, win);
 		@{operation1}@ /* MBIERROR1 */
		MPI_Win_unlock(1, win);
	}else if (rank == 2){
		int dest=1;
 		@{operation2}@ 
		@{fini2}@
	}else if (rank == 1){
		int src=2;
 		@{operation3}@ /* MBIERROR2 */
		@{fini3}@
	}

  MPI_Win_free(&win);

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


for p in put + get: 
    for s in send + isend:
         for r in recv + irecv:
             patterns = {}
             patterns = {'p': p, 's': s, 'r': r}
             patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'  
             patterns['rmafeature'] = 'Yes'
             patterns['p2pfeature'] = 'Yes' if s in send or r in recv  else 'Lacking'
             patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv  else 'Lacking'
             patterns['p'] = p 
             patterns['s'] = s 
             patterns['r'] = r 
             patterns['init1'] = init[p]("1") 
             patterns['init2'] = init[s]("2") 
             patterns['init3'] = init[r]("3") 
             patterns['fini2'] = fini[s]("2") 
             patterns['fini3'] = fini[r]("3") 
             patterns['operation1'] = operation[p]("1") #put or get
             patterns['operation2'] = operation[s]("2") #send
             patterns['operation3'] = operation[r]("3") #recv
             
             replace = patterns 
             replace['shortdesc'] = 'Global Concurrency error.' 
             replace['longdesc'] = 'Global Concurrency error. Concurrent access of variable W by @{p}@ and @{r}@'
             replace['outcome'] = 'ERROR: GlobalConcurrency' 
             replace['errormsg'] = 'Global Concurrency error. @{p}@ at @{filename}@:@{line:MBIERROR1}@ accesses the window of process 1. Process 1 receives data from process 2 and uses variable W. W in process 1 is then nondeterministic.'
             make_file(template, f'GlobalConcurrency_{p}_{s}_{r}_nok.c', replace)
