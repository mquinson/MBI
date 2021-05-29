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

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug.\\n");

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  @{init1}@
  @{init2}@
  int root = 0;

  if (@{change_cond}@) {
    @{operation1a}@ /* MBIERROR1 */
    @{operation2a}@
  } else {
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

collectives = ['MPI_Allgather', 'MPI_Allgatherv', 'MPI_Allreduce', 'MPI_Alltoall', 'MPI_Alltoallv', 'MPI_Barrier', 'MPI_Bcast', 'MPI_Gather', 'MPI_Reduce', 'MPI_Scatter']
icollectives = ['MPI_Ibarrier', 'MPI_Ireduce', 'MPI_Ibcast']

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

init['MPI_Ibarrier'] = lambda n: f'MPI_Request req{n}; MPI_Status sta{n};'
operation['MPI_Ibarrier'] = lambda n: f'MPI_Ibarrier(MPI_COMM_WORLD,&req{n});MPI_Wait(&req{n},&sta{n});'
fini['MPI_Ibarrier'] = lambda n: f'if (req{n} != MPI_REQUEST_NULL)  MPI_Request_free(&req{n});'

init['MPI_Bcast'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Bcast'] = lambda n: f'MPI_Bcast(buf{n}, buff_size, MPI_INT, 0, MPI_COMM_WORLD);'
fini['MPI_Bcast'] = lambda n: ""

init['MPI_Ibcast'] = lambda n: f'int buf{n}[buff_size];MPI_Request req{n};MPI_Status sta{n};'
operation['MPI_Ibcast'] = lambda n: f'MPI_Ibcast(buf{n}, buff_size, MPI_INT, 0, MPI_COMM_WORLD,&req{n});MPI_Wait(&req{n},&sta{n});'
fini['MPI_Ibcast'] = lambda n: f'if (req{n} != MPI_REQUEST_NULL)	MPI_Request_free(&req{n});'

init['MPI_Reduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Reduce'] = lambda n: f"MPI_Reduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);"
fini['MPI_Reduce'] = lambda n: ""

init['MPI_Ireduce'] = lambda n: f"MPI_Request req{n}; MPI_Status sta{n}; int sum{n}, val{n} = 1;"
operation['MPI_Ireduce'] = lambda n: f"MPI_Ireduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD, &req{n}); MPI_Wait(&req{n},&sta{n});"
fini['MPI_Ireduce'] = lambda n: f'if (req{n} != MPI_REQUEST_NULL) MPI_Request_free(&req{n});'

init['MPI_Scatter'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Scatter'] = lambda n: f"MPI_Scatter(&buf{n}, 1, MPI_INT, &val{n}, 1, MPI_INT, root, MPI_COMM_WORLD);"
fini['MPI_Scatter'] = lambda n: ""

init['MPI_Gather'] = lambda n: f"int val{n}, buf{n}[buff_size];"
operation['MPI_Gather'] = lambda n: f"MPI_Gather(&val{n}, 1, MPI_INT, buf{n},1, MPI_INT, root, MPI_COMM_WORLD);"
fini['MPI_Gather'] = lambda n: ""

for coll1 in collectives + icollectives:
    for coll2 in collectives + icollectives:
        patterns = {}
        patterns = {'coll1': coll1, 'coll2': coll2}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {sys.argv[0]}. DO NOT EDIT.'
        patterns['collfeature'] = 'Yes' if coll1 in collectives or coll2 in collectives else 'Lacking'
        patterns['icollfeature'] = 'Yes' if coll1 in icollectives or coll2 in icollectives else 'Lacking'
        patterns['coll1'] = coll1
        patterns['coll2'] = coll2
        patterns['init1'] = init[coll1]("1")
        patterns['init2'] = init[coll2]("2")
        patterns['fini1'] = fini[coll1]("1")
        patterns['fini2'] = fini[coll2]("2")
        patterns['operation1a'] = operation[coll1]("1")
        patterns['operation1b'] = operation[coll1]("1")
        patterns['operation2a'] = operation[coll2]("2")
        patterns['operation2b'] = operation[coll2]("2")
        patterns['change_cond'] = 'rank % 2'

        if coll1 == coll2:
            # Generate the correct code using the same collective twice
            replace = patterns
            replace['shortdesc'] = 'Correct collective ordering'
            replace['longdesc'] = f'All ranks call {coll1} twice'
            replace['outcome'] = 'OK'
            replace['errormsg'] = ''
            make_file(template, f'CollCorrect_{coll1}_{coll2}.c', replace)
            # Generate the correct code using the collective once
            replace = patterns
            replace['shortdesc'] = 'Correct collective ordering'
            replace['longdesc'] = f'All ranks call {coll1} once'
            replace['outcome'] = 'OK'
            replace['errormsg'] = ''
            replace['init2'] = ''
            replace['operation2a'] = ''
            replace['operation2b'] = ''
            replace['fini2'] = ''
            make_file(template, f'CollCorrect_{coll1}.c', replace)
        else:
            # Generate the correct ordering with two different collectives
            replace = patterns
            replace['shortdesc'] = 'Correct collective ordering'
            replace['longdesc'] = f'All ranks call {coll1} and then {coll2}'
            replace['outcome'] = 'OK'
            replace['errormsg'] = ''
            make_file(template, f'CollCorrect_{coll1}_{coll2}.c', replace)
            # Generate the incorrect ordering with two different collectives
            replace = patterns
            replace['shortdesc'] = 'Incorrect collective ordering'
            replace['longdesc'] = f'Odd ranks call {coll1} and then {coll2} while even ranks call these collectives in the other order'
            replace['outcome'] = 'ERROR: CallMatching'
            replace['errormsg'] = 'Collective mistmatch. @{coll1}@ at @{filename}@:@{line:MBIERROR1}@ is matched with @{coll2}@ line @{filename}@:@{line:MBIERROR2}@.'
            replace['operation1b'] = operation[coll2]("2")  # Inversion
            replace['operation2b'] = operation[coll1]("1")
            make_file(template, f'CollCallOrder_{coll1}_{coll2}_nok.c', replace)
            # Generate the incorrect ordering with one collective
            replace = patterns
            replace['shortdesc'] = 'Incorrect collective ordering'
            replace['longdesc'] = f'Odd ranks call {coll1} while even ranks do not call any collective'
            replace['outcome'] = 'ERROR: CallMatching'
            replace['errormsg'] = 'Collective mistmatch. @{coll1}@ at @{filename}@:@{line:MBIERROR1}@ is not matched.'
            replace['operation1b'] = ''  # Remove functions
            replace['operation2b'] = ''
            replace['operation2a'] = ''
            replace['fini2'] = ''
            make_file(template, f'CollCallOrder_{coll1}_none_nok.c', replace)
            # Generate a correct ordering with a conditional not depending on ranks
            replace = patterns
            replace['shortdesc'] = 'Correct collective ordering'
            replace['longdesc'] = f'All ranks call {coll1} and then {coll2} or inversely'
            replace['outcome'] = 'OK'
            replace['errormsg'] = ''
            replace['change_cond'] = 'nprocs<256'
            replace['operation2b'] = '' # Remove functions
            replace['operation2a'] = ''
            replace['fini2'] = ''
            make_file(template, f'CollCallOrder_{coll1}_none_ok.c', replace)
