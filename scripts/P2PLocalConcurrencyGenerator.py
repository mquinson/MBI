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
	P2P!persistent: @{persfeature}@
	P2P!probe: Lacking
	COLL!basic: Lacking
	COLL!nonblocking: Lacking
	COLL!persistent: Lacking
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
#include <stdlib.h>

#define buff_size 1

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
  	@{operation1}@ 
		@{write1}@ /* MBIERROR1 */ 
		@{fini1}@
	}else if (rank == 1){
  	@{operation2}@
		@{write2}@ /* MBIERROR2 */
		@{fini2}@
	}

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

send = ['MPI_Send'] 
recv = ['MPI_Recv'] 
isend = ['MPI_Isend']  
psend = ['MPI_Send_init']  
irecv = ['MPI_Irecv']  
precv = ['MPI_Recv_init']  

init = {}
operation = {}
fini = {}
write= {}

init['MPI_Send'] = lambda n: f'int buf{n}=rank;'
operation['MPI_Send'] = lambda n: f'MPI_Send(&buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD);'
fini['MPI_Send'] = lambda n: ""
write['MPI_Send'] = lambda n: ""

init['MPI_Recv'] = lambda n: f'int buf{n};MPI_Status sta{n};'
operation['MPI_Recv'] = lambda n: f'MPI_Recv(&buf{n}, buff_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &sta{n});'
fini['MPI_Recv'] = lambda n: ""
write['MPI_Recv'] = lambda n: ""

init['MPI_Isend'] = lambda n: f'int buf{n}=rank;MPI_Request req{n};'
operation['MPI_Isend'] = lambda n: f'MPI_Isend(&buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req{n});'
fini['MPI_Isend'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'
write['MPI_Isend'] = lambda n: f'buf{n}=4;'

init['MPI_Irecv'] = lambda n: f'int buf{n};MPI_Request req{n};'
operation['MPI_Irecv'] = lambda n: f'MPI_Irecv(&buf{n}, buff_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &req{n});'
fini['MPI_Irecv'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'
write['MPI_Irecv'] = lambda n: f'buf{n}++;' 

init['MPI_Send_init'] = lambda n: f'int buf{n}=rank; MPI_Request req{n};'
operation['MPI_Send_init'] = lambda n: f'MPI_Send_init(&buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req{n});\n 		MPI_Start(&req{n});' 
fini['MPI_Send_init'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);\n 		MPI_Request_free(&req{n}); '
write['MPI_Send_init'] = lambda n: f'buf{n}=4;' 

init['MPI_Recv_init'] = lambda n: f'int buf{n}; MPI_Request req{n};'
operation['MPI_Recv_init'] = lambda n: f'MPI_Recv_init(&buf{n}, buff_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &req{n});\n 		MPI_Start(&req{n});'
fini['MPI_Recv_init'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);\n 		MPI_Request_free(&req{n});'
write['MPI_Recv_init'] = lambda n: f'buf{n}++;' 

for s in send + isend + psend:
    for r in irecv + precv + recv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
        patterns['p2pfeature'] = 'Yes' if s in send else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if r in irecv else 'Lacking'
        patterns['persfeature'] = 'Yes' if r in precv else 'Lacking'
        patterns['s'] = s
        patterns['r'] = r
        patterns['init1'] = init[s]("1")
        patterns['init2'] = init[r]("2")
        patterns['fini1'] = fini[s]("1")
        patterns['fini2'] = fini[r]("2")
        patterns['operation1'] = operation[s]("1")
        patterns['operation2'] = operation[r]("2")
        patterns['write1'] = write[s]("1")
        patterns['write2'] = write[r]("2")

        # Generate a message race
        if s in send and r in irecv + precv:
            replace = patterns 
            replace['shortdesc'] = ' Local Concurrency with a P2P'
            replace['longdesc'] = f'The message buffer in {r} is modified before the call has been completed.'
            replace['outcome'] = 'ERROR: LocalConcurrency' 
            replace['errormsg'] = 'Local Concurrency with a P2P. The receive buffer in @{r}@ is modified at @{filename}@:@{line:MBIERROR2}@ whereas there is no guarantee the message has been received.'
            make_file(template, f'P2PLocalConcurrency_{r}_{s}_nok.c', replace)
        if s in isend + psend and r in recv:
            replace = patterns 
            replace['shortdesc'] = ' Local Concurrency with a P2P'
            replace['longdesc'] = f'The message buffer in {s} is modified before the call has been completed.'
            replace['outcome'] = 'ERROR: LocalConcurrency' 
            replace['errormsg'] = 'Local Concurrency with a P2P. The send buffer in @{s}@ is modified at @{filename}@:@{line:MBIERROR1}@ whereas there is no guarantee the message has been sent.'
            make_file(template, f'P2PLocalConcurrency_{r}_{s}_nok.c', replace)
        if s in isend + psend and r in irecv + precv:
            replace = patterns 
            replace['shortdesc'] = ' Local Concurrency with a P2P'
            replace['longdesc'] = f'The message buffer in {s} and {r} are modified before the calls have completed.'
            replace['outcome'] = 'ERROR: LocalConcurrency' 
            replace['errormsg'] = 'Local Concurrency with a P2P. The message buffers in @{s}@ and @{r}@ are modified at @{filename}@:@{line:MBIERROR1}@ and @{filename}@:@{line:MBIERROR2}@ whereas there is no guarantee the calls have been completed.'
            make_file(template, f'P2PLocalConcurrency_{r}_{s}_nok.c', replace)
