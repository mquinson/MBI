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

#define N 10

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int dest, src;
  int i=0;
  int root = 0;
  int stag = 0, rtag = 0;
  int buff_size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 3)
    printf("MBI ERROR: This test needs at least 3 processes to produce a bug!\\n");

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  MPI_Comm newcom = MPI_COMM_WORLD;
  MPI_Datatype type = MPI_INT;
  MPI_Op op = MPI_SUM;

  @{init0}@
  @{init1a}@
  @{init1b}@
  @{init2}@

  if (rank == 0) {
    dest = 1; stag = 1;
    @{operation0}@
    @{fini0}@
  } else if (rank == 1) {
    src = MPI_ANY_SOURCE;
    rtag = @{xtag}@;
    @{operation1a}@
    @{fini1a}@
    rtag = @{ytag}@;
    @{operation1b}@ @{tagerror}@
    @{fini1b}@
  } else if (rank == 2) {
    dest = 1; stag = 2;
    @{operation2}@
    @{fini2}@
  }

  @{free0}@
  @{free1a}@
  @{free1b}@
  @{free2}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


for s in send:
    for r in recv:
        for x in ['MPI_ANY_TAG', '1', '2']:
            for y in ['MPI_ANY_TAG', '1', '2']:
                patterns = {}
                patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
                patterns['p2pfeature'] = 'Yes' if s in send or r in recv else 'Lacking'
                patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv else 'Lacking'
                patterns['s'] = s
                patterns['r'] = r
                patterns['x'] = x
                patterns['y'] = y

                patterns['xtag'] = x
                patterns['ytag'] = y

                patterns['init0'] = init[s]("0")
                patterns['operation0'] = operation[s]("0")
                patterns['fini0'] = fini[s]("0")
                patterns['free0'] = free[s]("0")

                patterns['init1a'] = init[r]("1a")
                patterns['init1b'] = init[r]("1b")
                patterns['operation1a'] = operation[r]("1a")
                patterns['operation1b'] = operation[r]("1b")
                patterns['fini1a'] = fini[r]("1a")
                patterns['fini1b'] = fini[r]("1b")
                patterns['free1a'] = free[r]("1a")
                patterns['free1b'] = free[r]("1b")

                patterns['init2'] = init[s]("2")
                patterns['operation2'] = operation[s]("2")
                patterns['fini2'] = fini[s]("2")
                patterns['free2'] = free[s]("2")
                patterns['tagerror'] = '/* MBIERROR */'

                # To be correct, this benchmark must be use wildcard
                # on second recv, or the recv tag must be different.
                #
                # |-----+-----+----+----|
                # | x\y | ANY | 1  | 2  |
                # |-----+-----+----+----|
                # | ANY | OK  |  - |  - |
                # |   1 | OK  |  - | OK |
                # |   2 | OK  | OK |  - |
                # |-----+-----+----+----|

                if y == 'MPI_ANY_TAG' or (x != 'MPI_ANY_TAG' and x != y):
                    # Generate the correct matching because of the conditional
                    replace = patterns
                    replace['shortdesc'] = 'Message race'
                    replace['longdesc'] = 'Correct code without message race.'
                    replace['outcome'] = 'OK'
                    replace['errormsg'] = 'OK'
                    replace['tagerror'] = ''
                    make_file(template, f'MessageRace_tag_{x}_{y}_{s}_{r}_ok.c', replace)
                else:
                    # Generate the incorrect matching because of the conditional
                    replace = patterns
                    replace['shortdesc'] = 'Message race'
                    replace['longdesc'] = 'Message race in @{r}@ with @{s}@.'
                    replace['outcome'] = 'ERROR: MessageRace'
                    replace['errormsg'] = 'Message race. The use of wildcard receive calls @{r}@ at @{filename}@:@{line:MBIERROR}@ and incorrect tag matching.'
                    make_file(template, f'MessageRace_tag_{x}_{y}_{s}_{r}_nok.c', replace)
