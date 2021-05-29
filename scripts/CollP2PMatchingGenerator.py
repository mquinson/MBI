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
	COLL!basic: @{collfeature}@ 
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

#define buff_size 128

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int dest, src;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  
  @{init1}@
  @{init2}@
  @{init3}@
	if (rank == 0) {
		dest=1;src=1;
  	@{operation3}@ /* MBIERROR1 */
		@{fini3}@
  	@{operation1}@ 
		@{fini1}@
	}else if (rank==1) {
		dest=0;src=0;
  	@{operation2}@ /* MBIERROR2 */
		@{fini2}@
  	@{operation3}@ 
		@{fini3}@
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
coll = ['MPI_Allgather', 'MPI_Allgatherv', 'MPI_Allreduce', 'MPI_Alltoall', 'MPI_Alltoallv', 'MPI_Barrier', 'MPI_Bcast', 'MPI_Gather', 'MPI_Reduce', 'MPI_Scatter']

init = {}
operation = {}
fini = {}

## P2P

init['MPI_Send'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Send'] = lambda n: f'MPI_Send(buf{n}, buff_size, MPI_INT, dest, 0, MPI_COMM_WORLD);'
fini['MPI_Send'] = lambda n: ""

init['MPI_Recv'] = lambda n: f'int buf{n}[buff_size]; MPI_Status sta{n};'
operation['MPI_Recv'] = lambda n: f'MPI_Recv(buf{n}, buff_size, MPI_INT, src, 0, MPI_COMM_WORLD, &sta{n});'
fini['MPI_Recv'] = lambda n: ""

init['MPI_Isend'] = lambda n: f'int buf{n}[buff_size]; MPI_Request req{n};'
operation['MPI_Isend'] = lambda n: f'MPI_Isend(buf{n}, buff_size, MPI_INT, dest, 0, MPI_COMM_WORLD, &req{n});'
fini['MPI_Isend'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'

init['MPI_Irecv'] = lambda n: f'int buf{n}[buff_size]; MPI_Request req{n};'
operation['MPI_Irecv'] = lambda n: f'MPI_Irecv(buf{n}, buff_size, MPI_INT, src, 0, MPI_COMM_WORLD, &req{n});'
fini['MPI_Irecv'] = lambda n: f'MPI_Wait(&req{n}, MPI_STATUS_IGNORE);'

## COLL

init['MPI_Allgather'] = lambda n: f"int *rbuf{n} = malloc(dbs);"
operation['MPI_Allgather'] = lambda n: f"MPI_Allgather(&rank, 1, MPI_INT, rbuf{n}, 1, MPI_INT, MPI_COMM_WORLD);"
fini['MPI_Allgather'] = lambda n: f"free(rbuf{n});"

init['MPI_Allgatherv'] = lambda n: (f"int *rbuf{n} = malloc(dbs), *rcounts{n}=malloc(dbs),  *displs{n}=malloc(dbs);\n" 
  +  "  for (int i = 0; i < nprocs; i++) {\n"
  + f"    rcounts{n}[i] = 1;\n"
  + f"    displs{n}[i] = 2 * (nprocs - (i + 1));\n"
  +  "  }")
operation['MPI_Allgatherv'] = lambda n: f"MPI_Allgatherv(&rank, 1, MPI_INT, rbuf{n}, rcounts{n}, displs{n}, MPI_INT, MPI_COMM_WORLD);"
fini['MPI_Allgatherv'] = lambda n: f"free(rbuf{n});free(rcounts{n});free(displs{n});"

init['MPI_Allreduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Allreduce'] = lambda n: f"MPI_Allreduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);"
fini['MPI_Allreduce'] = lambda n: ""

init['MPI_Alltoall'] = lambda n: f"int *sbuf{n} = malloc(dbs), *rbuf{n} = malloc(dbs);"
operation['MPI_Alltoall'] = lambda n: f"MPI_Alltoall(sbuf{n}, 1, MPI_INT, rbuf{n}, 1, MPI_INT, MPI_COMM_WORLD);"
fini['MPI_Alltoall'] = lambda n: f"free(sbuf{n});free(rbuf{n});"

init['MPI_Alltoallv'] = lambda n: (f"int *sbuf{n}=malloc(dbs*2), *rbuf{n}=malloc(dbs*2), *scounts{n}=malloc(dbs), *rcounts{n}=malloc(dbs), *sdispls{n}=malloc(dbs), *rdispls{n}=malloc(dbs);\n"
  +  "  for (int i = 0; i < nprocs; i++) {\n"
  + f"    scounts{n}[i] = 2;\n"
  + f"    rcounts{n}[i] = 2;\n"
  + f"    sdispls{n}[i] = (nprocs - (i + 1)) * 2;\n"
  + f"    rdispls{n}[i] = i * 2;\n"
  +  "  }")
operation['MPI_Alltoallv'] = lambda n: f"MPI_Alltoallv(sbuf{n}, scounts{n}, sdispls{n}, MPI_INT, rbuf{n}, rcounts{n}, rdispls{n}, MPI_INT, MPI_COMM_WORLD);"
fini['MPI_Alltoallv'] = lambda n: f"free(sbuf{n});free(rbuf{n});free(scounts{n});free(rcounts{n});free(sdispls{n});free(rdispls{n});"

init['MPI_Barrier'] = lambda n: ""
operation['MPI_Barrier'] = lambda n: 'MPI_Barrier(MPI_COMM_WORLD);'
fini['MPI_Barrier'] = lambda n: ""

init['MPI_Bcast'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Bcast'] = lambda n: f'MPI_Bcast(buf{n}, buff_size, MPI_INT, 0, MPI_COMM_WORLD);'
fini['MPI_Bcast'] = lambda n: ""

init['MPI_Reduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Reduce'] = lambda n: f"MPI_Reduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);"
fini['MPI_Reduce'] = lambda n: ""

init['MPI_Scatter'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Scatter'] = lambda n: f"MPI_Scatter(&buf{n}, 1, MPI_INT, &val{n}, 1, MPI_INT, 0, MPI_COMM_WORLD);"
fini['MPI_Scatter'] = lambda n: ""

init['MPI_Gather'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Gather'] = lambda n: f"MPI_Gather(&val{n}, 1, MPI_INT, buf{n},1, MPI_INT, 0, MPI_COMM_WORLD);"
fini['MPI_Gather'] = lambda n: ""

for s in send + isend:
    for r in recv + irecv:
        for c in coll:
            patterns = {}
            patterns = {'s': s, 'r': r, 'c': c}
            patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
            patterns['p2pfeature'] = 'Yes' if s in send or r in recv else 'Lacking'
            patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv else 'Lacking'
            patterns['collfeature'] = 'Yes' if c in coll else 'Lacking'
            patterns['s'] = s
            patterns['r'] = r
            patterns['c'] = c
            patterns['init1'] = init[s]("1")
            patterns['init2'] = init[r]("2")
            patterns['init3'] = init[c]("3")
            patterns['fini1'] = fini[s]("1")
            patterns['fini2'] = fini[r]("2")
            patterns['fini3'] = fini[c]("3")
            patterns['operation1'] = operation[s]("1")
            patterns['operation2'] = operation[r]("2")
            patterns['operation3'] = operation[c]("3")

            # Generate the incorrect matching because of the conditional
            replace = patterns 
            replace['shortdesc'] = 'Point to point & collective mismatch'
            replace['longdesc'] = 'Point to point @{r}@ is matched with @{c}@ which causes a deadlock.' 
            replace['outcome'] = 'ERROR: CallMatching' 
            replace['errormsg'] = 'P2P & Collective mistmatch. @{r}@ at @{filename}@:@{line:MBIERROR2}@ is matched with @{c}@ at @{filename}@:@{line:MBIERROR1}@ wich causes a deadlock.'
            make_file(template, f'CollP2PCallMatching_{r}_{s}_{c}_nok.c', replace)

            # Generate the incorrect code depending on buffering
            replace = patterns 
            replace['shortdesc'] = 'Point to point & collective mismatch'
            replace['longdesc'] = 'Point to point @{s}@ is matched with @{c}@ which causes a deadlock depending on the buffering mode.' 
            replace['outcome'] = 'ERROR: BufferingHazard' 
            replace['errormsg'] = 'P2P & Collective mistmatch. @{s}@ at @{filename}@:@{line:MBIERROR2}@ is matched with @{c}@ at @{filename}@:@{line:MBIERROR1}@ wich causes a deadlock.'
            replace['init1'] = init[s]("1") 
            replace['init2'] = init[r]("2") 
            replace['operation1'] = operation[r]("2")
            replace['operation2'] = operation[s]("1")
            replace['fini1'] = fini[r]("2") 
            replace['fini2'] = fini[s]("1") 
            make_file(template, f'CollP2PBuffering_{r}_{s}_{c}_nok.c', replace)

