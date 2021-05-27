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
	COLL!tools: Lacking
	RMA: Lacking
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

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

	int recv_buffer=-1;
	int send_buffer=rank;

  @{init1}@
  @{init2}@

  if (rank == 0) {
    for (int i = 0; i < nprocs - 1; i++) {
  		@{operation1}@ /* MBIERROR */
			@{fini1}@
    }
	if (recv_buffer != 3) {
      printf("The last received message is not 3 but %d!\\n", recv_buffer);
      fflush(stdout);
      abort();
    }
  }else{
  	@{operation2}@
		@{fini2}@
  }


  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

p2p = ['MPI_Send', 'MPI_Recv'] 
send = ['MPI_Send'] 
recv = ['MPI_Recv'] 
ip2p = ['MPI_Isend', 'MPI_Irecv']  
isend = ['MPI_Isend']  
irecv = ['MPI_Irecv']  

init = {}
operation = {}
fini = {}

init['MPI_Send'] = lambda n: ""
operation['MPI_Send'] = lambda n: f'MPI_Send(&send_buffer, 1, MPI_INT, 0, 42, MPI_COMM_WORLD);'
fini['MPI_Send'] = lambda n: ""

init['MPI_Recv'] = lambda n: f'MPI_Status sta{n};'
operation['MPI_Recv'] = lambda n: f'MPI_Recv(&recv_buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &sta{n});'
fini['MPI_Recv'] = lambda n: ""

init['MPI_Isend'] = lambda n: f'MPI_Request req{n};'
operation['MPI_Isend'] = lambda n: f'MPI_Isend(&send_buffer, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &req{n});'
fini['MPI_Isend'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'

init['MPI_Irecv'] = lambda n: f'MPI_Request req{n};'
operation['MPI_Irecv'] = lambda n: f'MPI_Irecv(&recv_buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req{n});'
fini['MPI_Irecv'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'


for s in send + isend:
    for r in recv + irecv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
        patterns['p2pfeature'] = 'Yes' if s in send or r in recv else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv else 'Lacking'
        patterns['s'] = s
        patterns['r'] = r
        patterns['init2'] = init[s]("2")
        patterns['init1'] = init[r]("1")
        patterns['fini2'] = fini[s]("2")
        patterns['fini1'] = fini[r]("1")
        patterns['operation2'] = operation[s]("2")
        patterns['operation1'] = operation[r]("1")

        # Generate the incorrect matching
        replace = patterns 
        replace['shortdesc'] = 'The message ordering is non-deterministic.'
        replace['longdesc'] = f'The code assumes a fixed order in the reception of messages while the message ordering is non-deterministic.' 
        replace['outcome'] = 'ERROR: MessageRace' 
        replace['errormsg'] = 'P2P message race which can cause a deadlock. @{r}@ at @{filename}@:@{line:MBIERROR}@ is called with ANY_SRC.'
        make_file(template, f'P2PMessageRace_{r}_{s}_nok.c', replace)

