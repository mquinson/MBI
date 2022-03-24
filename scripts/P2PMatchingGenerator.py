#! /usr/bin/python3
import os
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: MBI

  Description: @{shortdesc}@
    @{longdesc}@

  Version of MPI: Conforms to MPI 1.1, does not require MPI 2 implementation

BEGIN_MPI_FEATURES
  P2P!basic: @{p2pfeature}@
  P2P!nonblocking: @{ip2pfeature}@
  P2P!persistent: @{persfeature}0@
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
  int its_raining = 0;
  int src=0, dest=1;
  int stag=0, rtag=0;

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
    @{operation1}@ /* MBIERROR1 */
    @{fini1}@
  }else if (@{change_cond}@){
    @{operation2}@ /* MBIERROR2 */
    @{fini2}@
  }

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


for p in send + ssend + bsend + recv + irecv + isend:
    patterns = {}
    patterns = {'p': p}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
    patterns['p2pfeature'] = 'Yes' if p in send + bsend + ssend + recv else 'Lacking'
    patterns['ip2pfeature'] = 'Yes' if p in isend + irecv else 'Lacking'
    patterns['persfeature'] = 'Lacking'
    #patterns['persfeature'] = 'Yes' if p in psend + precv else 'Lacking'
    patterns['p'] = p
    patterns['init1'] = init[p]("1")
    patterns['init2'] = '' #init[p2]("2")
    patterns['fini1'] = fini[p]("1")
    patterns['fini2'] = '' #fini[p2]("2")
    patterns['operation1'] = operation[p]("1")
    patterns['operation2'] = '' #operation[p2]("2")
    patterns['change_cond'] = 'rank == 1'

    # Generate the incorrect matching with one call
    replace = patterns
    replace['shortdesc'] = 'Point to point @{p}@ is not matched'
    replace['longdesc'] = 'Process 0 calls @{p}@ and is not matched'
    replace['outcome'] = 'ERROR: CallMatching'
    replace['errormsg'] = 'P2P mistmatch. @{p}@ at @{filename}@:@{line:MBIERROR1}@ is not matched.'
    make_file(template, f'CallOrdering_{p}_nok.c', replace)

    # Generate the incorrect matching with two calls
    replace = patterns
    replace['shortdesc'] = 'Both point to point @{p}@ are not matched'
    replace['longdesc'] = 'Processes 0 and 1 both call @{p}@ which are not matched'
    replace['outcome'] = 'ERROR: CallMatching'
    replace['errormsg'] = 'P2P mismatch. @{p}@ at @{filename}@:@{line:MBIERROR1}@ and @{p}@ at @{filename}@:@{line:MBIERROR2}@ are not matched.'
    replace['operation2'] = operation[p]("1")
    replace['fini2'] = fini[p]("1")
    make_file(template, f'CallOrdering_{p}_{p}_nok.c', replace)

for s in send + isend + ssend + bsend:
    for r in recv + irecv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
        patterns['p2pfeature'] = 'Yes' if s in send or r in recv else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv else 'Lacking'
        patterns['s'] = s
        patterns['r'] = r
        patterns['init1'] = init[s]("1")
        patterns['init2'] = init[r]("2")
        patterns['fini1'] = fini[s]("1")
        patterns['fini2'] = fini[r]("2")
        patterns['operation1'] = operation[s]("1")
        patterns['operation2'] = operation[r]("2")
        patterns['change_cond'] = '(rank == 1) && (its_raining)'

        # Generate the incorrect matching because of the conditional
        replace = patterns
        replace['shortdesc'] = 'Point to point @{r}@ is never called.'
        replace['longdesc'] = 'Point to point @{r}@ is never executed. Process 1 calls MPI_Finalize and causes a deadlock.'
        replace['outcome'] = 'ERROR: CallMatching'
        replace['errormsg'] = 'P2P mistmatch. @{r}@ at @{filename}@:@{line:MBIERROR2}@ is never called because of the conditional (@{change_cond}@).'
        replace['operation1'] =  operation[s]("1")
        replace['operation2'] = operation[r]("2")
        make_file(template, f'CallOrdering_{r}_{s}_nok.c', replace)

