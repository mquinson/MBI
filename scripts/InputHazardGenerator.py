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
  P2P!persistent: Lacking
  COLL!basic: @{collfeature}@
  COLL!nonblocking: @{icollfeature}@
  COLL!persistent: Lacking
  COLL!tools: Lacking
  RMA: Lacking
END_MPI_FEATURES

BEGIN_MBI_TESTS
  $ mpirun -np 2 ${EXE} 1
  | @{outcome}@
  | @{errormsg}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  MPI_Status sta;
  int src,dest;
  int i=0;
  int root = 0;
  int stag=0, rtag=0;
  int buff_size = N;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  if (argc < 2)
    printf("MBI ERROR: This test needs at least 1 argument to produce a bug!\\n");

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  MPI_Comm newcom = MPI_COMM_WORLD;
  MPI_Datatype type = MPI_INT;
  MPI_Op op = MPI_SUM;

  int n = atoi(argv[1]);
  int buffer[N] = {42};

  @{init1}@
  @{init2}@

  if (rank == 0) {
    dest=1, src=1;
    if ((n % 2) == 0) { @{errorcond}@
      @{operation1b}@
      @{fini1b}@
    } else {
      @{operation1a}@
      @{fini1a}@
    }
  } else @{addcond}@ {
    dest=0, src=0;
    @{operation2}@
    @{fini2}@
  }

  @{free1}@
  @{free2}@

  MPI_Finalize();

  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

# P2P
for s in send + isend:
    for r in recv + irecv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
        patterns['p2pfeature'] = 'Yes' if s in send or r in recv else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv else 'Lacking'
        patterns['collfeature'] = 'Lacking'
        patterns['icollfeature'] = 'Lacking'
        patterns['s'] = s
        patterns['r'] = r

        patterns['init1'] = init[s]("1")
        patterns['operation1a'] = operation[s]("1").replace("buf1", "buffer")
        patterns['operation1b'] = operation[s]("1").replace("buf1", "buffer")
        patterns['fini1a'] = fini[s]("1")
        patterns['fini1b'] = fini[s]("1")
        patterns['free1'] = free[s]("1")

        patterns['init2'] = init[r]("2")
        patterns['operation2'] = operation[r]("2").replace("buf2", "buffer")
        patterns['fini2'] = fini[r]("2")
        patterns['free2'] = free[r]("2")

        patterns['errorcond'] = ''
        patterns['addcond'] = 'if (rank == 1)'

        # Generate a correct matching
        replace = patterns
        replace['shortdesc'] = 'Correct call ordering.'
        replace['longdesc'] = 'Correct call ordering.'
        replace['outcome'] = 'OK'
        replace['errormsg'] = 'OK'
        make_file(template, f'InputHazardCallOrdering_{r}_{s}_ok.c', replace)

        # Generate the incorrect matching
        replace = patterns
        replace['shortdesc'] = 'Missing Send function.'
        replace['longdesc'] = 'Missing Send function call for a path depending to input, a deadlock is created.'
        replace['outcome'] = 'ERROR: CallMatching'
        replace['errormsg'] = 'P2P mistmatch. Missing @{r}@ at @{filename}@:@{line:MBIERROR}@.'
        replace['errorcond'] = '/* MBIERROR */'
        replace['operation1b'] = ''
        replace['fini1b'] = ''
        make_file(template, f'InputHazardCallOrdering_{r}_{s}_nok.c', replace)

# COLLECTIVE
for c in coll:
    patterns = {}
    patterns = {'c': c}
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
    patterns['p2pfeature'] = 'Lacking'
    patterns['ip2pfeature'] = 'Lacking'
    patterns['collfeature'] = 'Yes' if c in coll else 'Lacking'
    patterns['icollfeature'] = 'Yes' if c in icoll else 'Lacking'
    patterns['c'] = c

    patterns['init1'] = init[c]("1")
    patterns['operation1a'] = operation[c]("1")
    patterns['operation1b'] = operation[c]("1")
    patterns['fini1a'] = fini[c]("1")
    patterns['fini1b'] = fini[c]("1")
    patterns['free1'] = free[c]("1")

    patterns['init2'] = init[c]("2")
    patterns['operation2'] = operation[c]("2")
    patterns['fini2'] = fini[c]("2")
    patterns['free2'] = free[c]("2")

    patterns['errorcond'] = ''
    patterns['addcond'] = ''

    # Generate a correct matching
    replace = patterns
    replace['shortdesc'] = 'Correct call ordering.'
    replace['longdesc'] = 'Correct call ordering.'
    replace['outcome'] = 'OK'
    replace['errormsg'] = 'OK'
    make_file(template, f'InputHazardCallOrdering_{c}_ok.c', replace)

    # Generate the incorrect matching
    replace = patterns
    replace['shortdesc'] = 'Missing collective function call.'
    replace['longdesc'] = 'Missing collective function call for a path depending to input, a deadlock is created.'
    replace['outcome'] = 'ERROR: CallMatching'
    replace['errormsg'] = 'P2P mistmatch. Missing @{c}@ at @{filename}@:@{line:MBIERROR}@.'
    replace['errorcond'] = '/* MBIERROR */'
    replace['operation1b'] = ''
    replace['fini1b'] = ''
    make_file(template, f'InputHazardCallOrdering_{c}_nok.c', replace)
