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
	RMA: Lacking
END_MPI_FEATURES

BEGIN_MBI_TESTS
  $ mpirun -np 2 $zero_buffer ${EXE}
  | @{outcome1}@
  | @{errormsg1}@
  $ mpirun -np 2 $infty_buffer ${EXE}
  | @{outcome2}@
  | @{errormsg2}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
	int dest, src;
  int stag = 0, rtag = 0;
  int buff_size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

	MPI_Comm newcom = MPI_COMM_WORLD;
	MPI_Datatype type = MPI_INT;

  @{init1}@
  @{init2}@
	if (rank == 0) {
		src=1,dest=1;
  	@{operation1a}@ /* MBIERROR1 */
		@{fini1a}@
  	@{operation2a}@ 
		@{fini2a}@
	}else if (rank == 1) {
		src=0,dest=0;
  	@{operation1b}@ /* MBIERROR2 */
		@{fini1b}@
  	@{operation2b}@ 
		@{fini2b}@
	}

	@{free1}@
	@{free2}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

for s in send + isend:
    for r in recv + irecv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'  
        patterns['p2pfeature'] = 'Yes' if s in send or r in recv  else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv  else 'Lacking' 
        patterns['s'] = s 
        patterns['r'] = r 
        patterns['init1'] = init[s]("1") 
        patterns['init2'] = init[r]("2")
        patterns['fini1a'] = fini[s]("1") 
        patterns['fini1b'] = fini[s]("1") 
        patterns['fini2a'] = fini[r]("2") 
        patterns['fini2b'] = fini[r]("2") 
        patterns['free1'] = free[s]("1") 
        patterns['free2'] = free[r]("2") 
        patterns['operation1a'] = operation[s]("1") 
        patterns['operation2a'] = operation[r]("2") 
        patterns['operation1b'] = operation[s]("1") 
        patterns['operation2b'] = operation[r]("2") 

    		# Generate the incorrect matching depending on the buffering mode (send + recv)
        replace = patterns 
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ may not be matched' 
        replace['longdesc'] = 'Processes 0 and 1 both call @{s}@ and @{r}@. This results in a deadlock depending on the buffering mode' 
        replace['outcome1'] = 'ERROR: BufferingHazard' 
        replace['errormsg1'] = 'ERROR: BufferingHazard' 
        replace['outcome2'] = 'OK'
        replace['errormsg2'] = 'OK'
        make_file(template, f'P2PBuffering_{s}_{r}_{s}_{r}_nok.c', replace)
    		# Generate the incorrect matching depending on the buffering mode (recv + send)
        replace = patterns 
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ are not matched' 
        replace['longdesc'] = 'Processes 0 and 1 both call @{r}@ and @{s}@. This results in a deadlock'
        replace['outcome1'] = 'ERROR: CallMatching' 
        replace['errormsg1'] = 'ERROR: CallMatching' 
        replace['operation1a'] =  operation[r]("2")
        replace['fini1a'] =  fini[r]("2")
        replace['operation2a'] = operation[s]("1")
        replace['fini2a'] =  fini[s]("1")
        replace['operation1b'] =  operation[r]("2")
        replace['fini1b'] =  fini[r]("2")
        replace['operation2b'] = operation[s]("1")
        replace['fini2b'] =  fini[s]("1")
        make_file(template, f'P2PCallMatching_{r}_{s}_{r}_{s}_nok.c', replace)
    		# Generate the correct matching
        replace = patterns 
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ are correctly  matched' 
        replace['longdesc'] = 'Process 0 calls @{s}@ and process 1 calls @{r}@.' 
        replace['outcome1'] = 'OK' 
        replace['errormsg1'] = 'OK' 
        replace['fini1a'] =  fini[s]("1")
        replace['fini2a'] =  fini[r]("2")
        replace['operation1a'] =  operation[s]("1")
        replace['operation2a'] = operation[r]("2")
        make_file(template, f'P2PCallMatching_{s}_{r}_{r}_{s}_ok.c', replace)
