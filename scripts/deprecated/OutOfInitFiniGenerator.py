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
	COLL!Tools: Lacking
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
  @{init1}@
  @{operation1}@ 
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug.\\n");

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  @{init2}@
  @{fini1}@
  
  MPI_Finalize();

  @{operation2}@ /* MBIERROR */
  @{fini2}@

  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

collectives = ['MPI_Allgather', 'MPI_Allgatherv', 'MPI_Allreduce', 'MPI_Alltoall', 'MPI_Alltoallv', 'MPI_Barrier', 'MPI_Bcast', 'MPI_Gather', 'MPI_Reduce', 'MPI_Scatter']
icollectives = ['MPI_Ibarrier', 'MPI_Ireduce']

init = {}
fini = {}
operation = {}

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

init['MPI_Ibarrier'] = lambda n: f"MPI_Request req{n};MPI_Status sta{n};"
operation['MPI_Ibarrier'] = lambda n: f"MPI_Ibarrier(MPI_COMM_WORLD,&req{n});MPI_Wait(&req{n},&sta{n});"
fini['MPI_Ibarrier'] = lambda n: ""

init['MPI_Bcast'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Bcast'] = lambda n: f'MPI_Bcast(buf{n}, buff_size, MPI_INT, 0, MPI_COMM_WORLD);'
fini['MPI_Bcast'] = lambda n: ""

init['MPI_Reduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Reduce'] = lambda n: f"MPI_Reduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);"
fini['MPI_Reduce'] = lambda n: ""

init['MPI_Ireduce'] = lambda n: f"MPI_Request req{n}; MPI_Status sta{n}; int sum{n}, val{n} = 1;"
operation['MPI_Ireduce'] = lambda n: f"MPI_Ireduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD, &req{n}); MPI_Wait(&req{n},&sta{n});"
fini['MPI_Ireduce'] = lambda n: ""

init['MPI_Scatter'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Scatter'] = lambda n: f"MPI_Scatter(&buf{n}, 1, MPI_INT, &val{n}, 1, MPI_INT, 0, MPI_COMM_WORLD);"
fini['MPI_Scatter'] = lambda n: ""

init['MPI_Gather'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Gather'] = lambda n: f"MPI_Gather(&val{n}, 1, MPI_INT, buf{n},1, MPI_INT, 0, MPI_COMM_WORLD);"
fini['MPI_Gather'] = lambda n: ""

for coll in collectives + icollectives:
    patterns = {}
    patterns = {'coll': coll} 
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
    patterns['collfeature'] = 'Yes' if coll in collectives else 'Lacking'
    patterns['icollfeature'] = 'Yes' if coll in icollectives else 'Lacking'
    patterns['coll'] = coll
    patterns['init1'] = init[coll]("1")
    patterns['init2'] = init[coll]("2")
    patterns['fini1'] = fini[coll]("1")
    patterns['fini2'] = fini[coll]("2")
    patterns['operation1'] = operation[coll]("1")
    patterns['operation2'] = operation[coll]("2")

    # Generate the incorrect code
    replace = patterns
    replace['shortdesc'] = 'Collective outside of parallel region'
    replace['longdesc'] = f'Collective {coll} called outside of the MPI parallel region'
    replace['outcome'] = 'ERROR: OutOfInitFini'
    replace['errormsg'] = 'Collective {coll} at @{filename}@:@{line:MBIERROR}@ is called after MPI_Finalize'
    replace['init1'] = ""
    replace['fini1'] = ""
    replace['operation1'] = ""
    make_file(template, f'CallAfterFinalize_{coll}_nok.c', replace)
