#! /usr/bin/python3
import sys
from generator_utils import make_file

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

BEGIN_MPI_FEATURES
  P2P:   Lacking
  iP2P:  Lacking
  PERS:  Lacking
  COLL:  @{collfeature}@
  iCOLL: @{icollfeature}@
  TOPO:  Lacking
  RMA:   Lacking
  PROB:  Lacking
  COM:   Lacking
  GRP:   Lacking
  DATA:  Lacking
  OP:    Lacking
END_MPI_FEATURES

BEGIN_ERROR_LABELS
  deadlock:  ???
  numstab:   never
  mpierr:    never
  resleak:   never
  datarace:  never
  various:   never
END_ERROR_LABELS

BEGIN_MBI_TESTS
  $ mpirun -np 2 ${EXE}
  | @{outcome}@
  | @{errormsg}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stdio.h>

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
  @{change_root}@

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

collectives = ['MPI_Allreduce', 'MPI_Alltoall', 'MPI_Barrier', 'MPI_Bcast', 'MPI_Gather', 'MPI_Reduce', 'MPI_Scatter']
icollectives = []  # 'ibarrier', 'ireduce', 'iallreduce']

init = {}
fini = {}
operation = {}

init['MPI_Alltoall'] = lambda n: f"int *sbuf{n} = malloc(dbs), *rbuf{n} = malloc(dbs);"
operation['MPI_Alltoall'] = lambda n: f"MPI_Alltoall(sbuf{n}, 1, MPI_INT, rbuf{n}, root, MPI_INT, MPI_COMM_WORLD);"
fini['MPI_Alltoall'] = lambda n: f"free(sbuf{n});free(rbuf{n});"

init['MPI_Alltoallv'] = lambda n: f"int *sbuf{n}=malloc(dbs*2), *rbuf{n}=malloc(dbs*2), *scounts{n}=malloc(dbs), *rcounts{n}=malloc(dbs), *sdispls{n}=malloc(dbs), *rdispls{n}=malloc(dbs);"
operation['MPI_Alltoallv'] = lambda n: f"MPI_Alltoallv(sbuf{n}, scounts{n}, sdispls{n}, MPI_INT, rbuf{n}, rcounts{n}, rdispls{n}, MPI_INT, MPI_COMM_WORLD);"
fini['MPI_Alltoallv'] = lambda n: f"free(sbuf{n});free(rbuf{n});free(scounts{n});free(rcounts{n});free(sdispl{n});free(rdispls{n});"

init['MPI_Barrier'] = lambda n: ""
operation['MPI_Barrier'] = lambda n: 'MPI_Barrier(MPI_COMM_WORLD);'
fini['MPI_Barrier'] = lambda n: ""

init['MPI_Bcast'] = lambda n: f'int buf{n}[buff_size];'
operation['MPI_Bcast'] = lambda n: f'MPI_Bcast(buf{n}, buff_size, MPI_INT, 0, MPI_COMM_WORLD);'
fini['MPI_Bcast'] = lambda n: ""

init['MPI_Reduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Reduce'] = lambda n: f"MPI_Reduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);"
fini['MPI_Reduce'] = lambda n: ""

init['MPI_Allreduce'] = lambda n: f"int sum{n}, val{n} = 1;"
operation['MPI_Allreduce'] = lambda n: f"MPI_Allreduce(&sum{n}, &val{n}, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);"
fini['MPI_Allreduce'] = lambda n: ""

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
        patterns['collfeature'] = 'Correct' if coll1 in collectives or coll2 in collectives else 'Lacking'
        patterns['icollfeature'] = 'Correct' if coll1 in icollectives or coll2 in icollectives else 'Lacking'
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
        patterns['change_root'] = ''

        if coll1 == coll2:
            # Generate the correct code using the collective twice
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
            make_file(template, f'CollCorrect_{coll1}.c', replace)
            # Generate an incorrect root matching
            replace['change_cond'] = '1'
            replace['operation1b'] = '/* nothing to do in this case */'
            replace['shortdesc'] = 'Collective @{coll1}@ with a root mismatch'
            replace['longdesc'] = f'Odd ranks use 0 as a root while even ranks use 1 as a root'
            replace['outcome'] = 'ERROR: RootMismatch'
            replace['errormsg'] = 'Collective root mistmatch. @{coll1}@ at @{filename}@:@{line:MBIERROR}@ has 0 or 1 as a root.'
            replace['change_root'] = 'if (rank % 2)\n    root = 1;'
            make_file(template, f'CollRootMatching_{coll1}_nok.c', replace)
            # Generate the call with root=-1
            replace['shortdesc'] = f'Collective {coll1} with root = -1'
            replace['longdesc'] = f'Collective {coll1} with root = -1'
            replace['outcome'] = 'ERROR: InvalidRoot'
            replace['errormsg'] = 'Invalid collective root.  @{coll1}@ at @{filename}@:@{line:MBIERROR}@ has -1 as a root while communicator MPI_COMM_WORLD requires ranks in range 0 to 1.'
            replace['change_root'] = 'root = -1;'
            make_file(template, f'CollRootNeg_{coll1}_nok.c', replace)
            # Generate the call with root=2
            replace['shortdesc'] = f'Collective {coll1} with root out of the communicator'
            replace['longdesc'] = f'Collective {coll1} with root = 2 (there is only 2 ranks)'
            replace['outcome'] = 'ERROR: InvalidRoot'
            replace['errormsg'] = 'Invalid collective root.  @{coll1}@ at @{filename}@:@{line:MBIERROR}@ has 2 as a root while communicator MPI_COMM_WORLD requires ranks in range 0 to 1.'
            replace['change_root'] = 'root = nprocs;'
            make_file(template, f'CollRootTooLarge_{coll1}_nok.c', replace)

        else:
            # Generate the correct ordering
            replace = patterns
            replace['shortdesc'] = 'Correct collective ordering'
            replace['longdesc'] = f'All ranks call {coll1} and then {coll2}'
            replace['outcome'] = 'OK'
            replace['errormsg'] = ''
            make_file(template, f'CollCorrect_{coll1}_{coll2}.c', replace)
            # Generate the incorrect ordering
            replace = patterns
            replace['shortdesc'] = 'Incorrect collective ordering'
            replace['longdesc'] = f'Odd ranks call {coll1} and then {coll2} while even ranks call these collectives in the other order'
            replace['outcome'] = 'ERROR: CollectiveOrdering'
            replace['errormsg'] = 'Collective mistmatch. @{coll1}@ at @{filename}@:@{line:MBIERROR1}@ is matched with @{coll2}@ line @{filename}@:@{line:MBIERROR2}@.'
            replace['operation1b'] = operation[coll2]("1")  # Inversion
            replace['operation2b'] = operation[coll1]("2")
            make_file(template, f'CollCallOrder_{coll1}_{coll2}.c', replace)
            # Generate the incorrect ordering using one collective
            replace = patterns
            replace['shortdesc'] = 'Incorrect collective ordering'
            replace['longdesc'] = f'Odd ranks call {coll1} while even ranks do not call any collective'
            replace['outcome'] = 'ERROR: CollectiveOrdering'
            replace['errormsg'] = 'Collective mistmatch. @{coll1}@ at @{filename}@:@{line:MBIERROR1}@ is not matched.'
            replace['operation1b'] = ''  # Remove functions
            replace['operation2b'] = ''
            replace['operation2a'] = ''
            make_file(template, f'CollCallOrder_{coll1}_none_nok.c', replace)
            # Generate a correct ordering with a conditional not depending on ranks
            replace = patterns
            replace['shortdesc'] = 'Correct collective ordering'
            replace['longdesc'] = f'All ranks call {coll1} and then {coll2} or inversely'
            replace['outcome'] = 'OK'
            replace['errormsg'] = ''
            replace['change_cond'] = 'nprocs<56'
            make_file(template, f'CollCallOrder2_{coll1}_none_ok.c', replace)
