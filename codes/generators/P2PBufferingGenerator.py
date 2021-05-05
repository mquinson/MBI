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
  	@{operation1a}@ /* MBIERROR1 */
  	@{operation2a}@ 
	}if (rank == 1) {
  	@{operation1b}@ /* MBIERROR2 */
  	@{operation2b}@ 
	}
	@{fini1}@
	@{fini2}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

send = ['MPI_Send'] 
isend = ['MPI_Isend'] 
recv = ['MPI_Recv'] 
irecv = ['MPI_Irecv']  

init = {}
operation = {}
fini = {}

init['MPI_Send'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Send'] = lambda n: f'MPI_Send(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD);'
fini['MPI_Send'] = lambda n: ""

init['MPI_Recv'] = lambda n: f'int buf{n}[buff_size]; MPI_Status sta{n};'
operation['MPI_Recv'] = lambda n: f'MPI_Recv(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &sta{n});'
fini['MPI_Recv'] = lambda n: ""

init['MPI_Isend'] = lambda n: f'int buf{n}[buff_size]; MPI_Request req{n};'
operation['MPI_Isend'] = lambda n: f'MPI_Isend(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req{n});\n 		MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'
fini['MPI_Isend'] = lambda n: ""

init['MPI_Irecv'] = lambda n: f'int buf{n}[buff_size]; MPI_Request req{n};'
operation['MPI_Irecv'] = lambda n: f'MPI_Irecv(buf{n}, buff_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req{n});\n 		MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'
fini['MPI_Irecv'] = lambda n: ""

for p1 in send + isend:
    for p2 in recv + irecv:
        patterns = {}
        patterns = {'p1': p1, 'p2': p2}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'  
        patterns['p2pfeature'] = 'Correct' if p1 in send + recv  else 'Lacking'
        patterns['ip2pfeature'] = 'Correct' if p2 in isend + irecv  else 'Lacking' 
        patterns['p1'] = p1 
        patterns['p2'] = p2 
        patterns['init1'] = init[p1]("1") 
        patterns['init2'] = init[p2]("2")
        patterns['fini1'] = fini[p1]("1") 
        patterns['fini2'] = fini[p2]("2") 
        patterns['operation1a'] = operation[p1]("1") 
        patterns['operation2a'] = operation[p2]("2") 
        patterns['operation1b'] = operation[p1]("1") 
        patterns['operation2b'] = operation[p2]("2") 

    		# Generate the incorrect matching depending on the buffering mode (send + recv)
        replace = patterns 
        replace['shortdesc'] = 'Point to point @{p1}@ and @{p2}@ may not be matched' 
        replace['longdesc'] = f'Processes 0 and 1 both call @{p1}@ and @{p2}@. This results in a deadlock depending on the buffering mode' 
        replace['outcome'] = 'ERROR: BufferingHazard' 
        replace['errormsg'] = 'ERROR: BufferingHazard' 
        make_file(template, f'P2PBuffering_{p1}_{p2}_nok.c', replace)
    		# Generate the incorrect matching depending on the buffering mode (recv + send)
        replace = patterns 
        replace['shortdesc'] = 'Point to point @{p1}@ and @{p2}@ may not be matched' 
        replace['longdesc'] = f'Processes 0 and 1 both call @{p1}@ and @{p2}@. This results in a deadlock depending on the buffering mode' 
        replace['outcome'] = 'ERROR: BufferingHazard' 
        replace['errormsg'] = 'ERROR: BufferingHazard' 
        replace['operation1a'] =  operation[p2]("2")
        replace['operation2a'] = operation[p1]("1")
        replace['operation1b'] =  operation[p2]("2")
        replace['operation2b'] = operation[p1]("1")
        make_file(template, f'P2PBuffering_{p2}_{p1}_nok.c', replace)
    		# Generate the correct matching 
        replace = patterns 
        replace['shortdesc'] = 'Point to point @{p1}@ and @{p2}@ may not be matched' 
        replace['longdesc'] = f'Processes 0 and 1 both call @{p1}@ and @{p2}@. This results in a deadlock depending on the buffering mode' 
        replace['outcome'] = 'OK' 
        replace['errormsg'] = 'OK' 
        replace['operation1a'] =  operation[p1]("1")
        replace['operation2a'] = operation[p2]("2")
        make_file(template, f'P2PCallMatching_{p1}_{p2}_ok.c', replace)
