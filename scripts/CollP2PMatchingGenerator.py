#! /usr/bin/python3
import os
import sys
import generator_utils as gen

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


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int dest, src;
  int root = 0;
  int stag = 0, rtag = 0;
  int buff_size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  int dbs = sizeof(int)*nprocs; /* Size of the dynamic buffers for alltoall and friends */
  MPI_Comm newcom = MPI_COMM_WORLD;
  MPI_Datatype type = MPI_INT;
  MPI_Op op = MPI_SUM;

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

  @{free1}@
  @{free2}@
  @{free3}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""


for s in gen.send + gen.isend:
    for r in gen.recv + gen.irecv:
        for c in gen.coll:
            patterns = {}
            patterns = {'s': s, 'r': r, 'c': c}
            patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
            patterns['p2pfeature'] = 'Yes' if s in gen.send or r in gen.recv else 'Lacking'
            patterns['ip2pfeature'] = 'Yes' if s in gen.isend or r in gen.irecv else 'Lacking'
            patterns['collfeature'] = 'Yes' if c in gen.coll else 'Lacking'
            patterns['s'] = s
            patterns['r'] = r
            patterns['c'] = c
            patterns['init1'] = gen.init[s]("1")
            patterns['init2'] = gen.init[r]("2")
            patterns['init3'] = gen.init[c]("3")
            patterns['fini1'] = gen.fini[s]("1")
            patterns['fini2'] = gen.fini[r]("2")
            patterns['fini3'] = gen.fini[c]("3")
            patterns['free1'] = gen.free[s]("1")
            patterns['free2'] = gen.free[r]("2")
            patterns['free3'] = gen.free[c]("3")
            patterns['operation1'] = gen.operation[s]("1")
            patterns['operation2'] = gen.operation[r]("2")
            patterns['operation3'] = gen.operation[c]("3")

            # Generate the incorrect matching because of the conditional
            replace = patterns.copy()
            replace['shortdesc'] = 'Point to point & collective mismatch'
            replace['longdesc'] = 'The @{s}@ corresponding to the @{r}@ is after @{c}@, while @{r}@ is before @{c}@. This is a deadlock.'
            replace['outcome'] = 'ERROR: CallMatching'
            replace['errormsg'] = 'P2P & Collective mistmatch. @{r}@ at @{filename}@:@{line:MBIERROR2}@ is matched with @{c}@ at @{filename}@:@{line:MBIERROR1}@ wich causes a deadlock.'
            gen.make_file(template, f'CallOrdering_{r}_{s}_{c}_nok.c', replace)

            # Generate the incorrect code depending on buffering
            #  replace = patterns.copy()
            #  replace['shortdesc'] = 'Point to point & collective mismatch'
            #  replace['longdesc'] = 'Point to point @{s}@ is matched with @{c}@ which causes a deadlock depending on the buffering mode.'
            #  replace['outcome'] = 'ERROR: BufferingHazard'
            #  replace['errormsg'] = 'P2P & Collective mistmatch. @{s}@ at @{filename}@:@{line:MBIERROR2}@ is matched with @{c}@ at @{filename}@:@{line:MBIERROR1}@ wich causes a deadlock.'
            #  replace['init1'] = gen.init[s]("1")
            #  replace['init2'] = gen.init[r]("2")
            #  replace['operation1'] = gen.operation[r]("2")
            #  replace['operation2'] = gen.operation[s]("1")
            #  replace['fini1'] = gen.fini[r]("2")
            #  replace['fini2'] = gen.fini[s]("1")
            #  gen.make_file(template, f'CollP2PBuffering_{r}_{s}_{c}_nok.c', replace)
