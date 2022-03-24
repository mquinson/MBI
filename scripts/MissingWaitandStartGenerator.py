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
  P2P!basic: Lacking
  P2P!nonblocking: @{ip2pfeature}@
  P2P!persistent: @{persfeature}@
  COLL!basic: Lacking
  COLL!nonblocking: @{icollfeature}@
  COLL!persistent: @{cpersfeature}@
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


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int root = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  MPI_Comm newcom = MPI_COMM_WORLD;
  MPI_Datatype type = MPI_INT;
  MPI_Op op = MPI_SUM;
  int stag = 0, rtag = 0;
  int buff_size = 1;

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */

  int dest = (rank == nprocs - 1) ? (0) : (rank + 1);
  int src = (rank == 0) ? (nprocs - 1) : (rank - 1);

  @{init1}@
  @{init2}@

  @{operation1}@ /* MBIERROR */
  @{start1}@
  @{operation2}@
  @{start2}@

  @{fini1}@
  @{fini2}@

  @{free1}@
  @{free2}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


for s in isend + psend:
    for r in irecv + precv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
        patterns['persfeature'] = 'Yes' if s in psend or r in precv  else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv  else 'Lacking'
        patterns['icollfeature'] = 'Lacking'
        patterns['cpersfeature'] = 'Lacking'
        patterns['s'] = s
        patterns['r'] = r
        patterns['init1'] = init[s]("1")
        patterns['init2'] = init[r]("2")
        patterns['start1'] = start[s]("1")
        startPers = patterns['start1']
        patterns['start2'] = start[r]("2")
        patterns['operation1'] = operation[s]("1")
        patterns['operation2'] = operation[r]("2")
        patterns['fini1'] = fini[s]("1")
        wait = patterns['fini1']
        patterns['fini2'] = fini[r]("2")
        patterns['free1'] = free[s]("1")
        Reqfree = patterns['free1']
        patterns['free2'] = free[r]("2")

        # Generate the correct code
        replace = patterns
        replace['shortdesc'] = 'Correct matching'
        replace['longdesc'] = f'No error'
        replace['outcome'] = 'OK'
        replace['errormsg'] = 'OK'
        make_file(template, f'ReqLifecycle_{s}_{r}_ok.c', replace)

        # Generate the code with a missing wait
        replace = patterns
        replace['shortdesc'] = 'Missing wait'
        replace['longdesc'] = 'Missing Wait. @{s}@ at @{filename}@:@{line:MBIERROR}@ has no completion.'
        replace['outcome'] = 'ERROR: MissingWait'
        replace['errormsg'] = 'ERROR: MissingWait'
        replace['fini1'] =  ' /* MBIERROR MISSING: ' + wait + ' */'
        make_file(template, f'ReqLifecycle_MissingWait_{s}_{r}_nok.c', replace)

        if s in psend:
            # Generate the code with a missing start - persistent only
            replace = patterns
            replace['shortdesc'] = 'Missing start'
            replace['longdesc'] = 'Missing start. @{s}@ at @{filename}@:@{line:MBIERROR}@ has no start'
            replace['outcome'] = 'ERROR: MissingStart'
            replace['errormsg'] = 'ERROR: MissingStart'
            replace['fini1'] = fini[s]("1")
            replace['start1'] = ' /* MBIERROR MISSING: ' + startPers + ' */'
            make_file(template, f'ReqLifecycle_MissingStart_{s}_{r}_nok.c', replace)
            # Generate the code with a missing free - persistent only
            replace = patterns
            replace['shortdesc'] = 'Missing free'
            replace['longdesc'] = 'Missing free. @{s}@ at @{filename}@:@{line:MBIERROR}@ has no free'
            replace['outcome'] = 'ERROR: RequestLeak'
            replace['errormsg'] = 'ERROR: RequestLeak'
            replace['start1'] = start[s]("1")
            replace['free1'] = ' /* MBIERROR MISSING: ' + Reqfree + ' */'
            make_file(template, f'ResLeak_nofree_{s}_{r}_nok.c', replace)


# Collectives only
for c in pcoll + icoll + ibarrier:
    patterns = {}
    patterns = {'c': c}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
    patterns['persfeature'] = 'Lacking'
    patterns['ip2pfeature'] = 'Lacking'
    patterns['cpersfeature'] = 'Yes' if c in pcoll else 'Lacking'
    patterns['icollfeature'] = 'Yes' if c in icoll + ibarrier else 'Lacking'
    patterns['c'] = c
    patterns['init1'] = init[c]("1")
    patterns['operation1'] = operation[c]("1")
    patterns['start1'] = start[c]("1")
    patterns['fini1'] = fini[c]("1")
    patterns['free1'] = free[c]("1")
    opstart = patterns['start1']
    opwait = patterns['fini1']
    opfree = patterns['free1']
    patterns['init2'] = ""
    patterns['operation2'] = ""
    patterns['start2'] = ""
    patterns['fini2'] = ""
    patterns['free2'] = ""

    # Generate the code with a missing wait
    replace = patterns
    replace['shortdesc'] = 'Missing wait'
    replace['longdesc'] = 'Missing Wait. @{c}@ at @{filename}@:@{line:MBIERROR}@ has no completion'
    replace['outcome'] = 'ERROR: MissingWait'
    replace['errormsg'] = 'ERROR: MissingWait'
    replace['fini1'] = ' /* MBIERROR MISSING: ' + opwait + ' */'
    replace['free1'] = ' /* MISSING: ' + replace['free1'] + ' (to not free the buffer before an internal wait */'
    make_file(template, f'ReqLifecycle_MissingWait_{c}_nok.c', replace)

    if c in pcoll:
        # Generate the code with a missing start - persistent only
        replace = patterns
        replace['shortdesc'] = 'Missing start functio'
        replace['longdesc'] = 'Missing Start. @{c}@ at @{filename}@:@{line:MBIERROR}@ has no start'
        replace['outcome'] = 'ERROR: MissingStart'
        replace['errormsg'] = 'ERROR: MissingStart'
        replace['fini1'] = fini[c]("1")
        replace['start1'] = ' /* MBIERROR MISSING: ' + opstart + ' */'
        make_file(template, f'ReqLifecycle_MissingStart_{c}_nok.c', replace)

        # Generate the code with a resleak (no free) - persistent only
        replace = patterns
        replace['shortdesc'] = 'Missing free'
        replace['longdesc'] = 'Missing free. @{c}@ at @{filename}@:@{line:MBIERROR}@ has no free'
        replace['outcome'] = 'ERROR: RequestLeak'
        replace['errormsg'] = 'ERROR: RequestLeak'
        replace['start1'] = start[c]("1")
        replace['free1'] = ' /* MBIERROR MISSING: ' + opfree + ' */'
        make_file(template, f'ResLeak_nofree_{c}_nok.c', replace)
