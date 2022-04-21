#! /usr/bin/python3
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: @{origin}@

  Description: @{shortdesc}@
    @{longdesc}@

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
  $ mpirun -np 4 $zero_buffer ${EXE}
  | @{outcome1}@
  | @{errormsg1}@
  $ mpirun -np 4 $infty_buffer ${EXE}
  | @{outcome1}@
  | @{errormsg1}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  int dest, src;
  int stag = 0, rtag = 0;
  int buff_size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (nprocs < 4)
    printf("MBI ERROR: This test needs at least 4 processes to produce a bug!\\n");

  MPI_Comm newcom = MPI_COMM_WORLD;
  MPI_Datatype type = MPI_INT;

  @{init1}@
  @{init2}@
  if (rank == 0) {
    src=@{src1}@,dest=@{dest1}@;
    @{operation1a}@ /* MBIERROR1 */
    @{fini1a}@
    @{operation2a}@
    @{fini2a}@
  }else if (rank == 1) {
    src=@{src2}@,dest=@{dest2}@;
    @{operation1b}@ /* MBIERROR2 */
    @{fini1b}@
    @{operation2b}@
    @{fini2b}@
  }else{
    src=@{src3}@,dest=@{dest3}@;
    @{operation1c}@
    @{fini1c}@
    @{operation2c}@
    @{fini2c}@
  }
  @{free1}@
  @{free2}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}
"""

for s in send + isend:
    for r in recv + irecv:
        patterns = {}
        patterns = {'s': s, 'r': r}
        patterns['origin'] = 'MBI'
        patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
        patterns['p2pfeature'] = 'Yes' if s in send or r in recv  else 'Lacking'
        patterns['ip2pfeature'] = 'Yes' if s in isend or r in irecv  else 'Lacking'
        patterns['s'] = s
        patterns['r'] = r
        patterns['src1'] = '1'
        patterns['dest1'] = '1'
        patterns['src2'] = '0'
        patterns['dest2'] = '0'
        patterns['src3'] = '0'
        patterns['dest3'] = '0'
        patterns['init1'] = init[s]("1")
        patterns['init2'] = init[r]("2")
        patterns['fini1a'] = fini[s]("1")
        patterns['fini1b'] = fini[s]("1")
        patterns['fini1c'] = ''
        patterns['fini2a'] = fini[r]("2")
        patterns['fini2b'] = fini[r]("2")
        patterns['fini2c'] = ''
        patterns['free1'] = free[s]("1")
        patterns['free2'] = free[r]("2")
        patterns['operation1a'] = operation[s]("1")
        patterns['operation2a'] = operation[r]("2")
        patterns['operation1b'] = operation[s]("1")
        patterns['operation2b'] = operation[r]("2")
        patterns['operation1c'] = ''
        patterns['operation2c'] = ''

        # Generate the incorrect matching depending on the buffering mode (send + recv)
        replace = patterns
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ may not be matched'
        replace['longdesc'] = 'Processes 0 and 1 both call @{s}@ and @{r}@. This results in a deadlock depending on the buffering mode'
        replace['outcome1'] = 'ERROR: BufferingHazard'
        replace['errormsg1'] = f'Buffering Hazard. Possible deadlock depending the buffer size of MPI implementation and system environment cause by two processes call {s} before {r}.'
        make_file(template, f'P2PBuffering_{s}_{r}_{s}_{r}_nok.c', replace)

        # Generate the incorrect matching with send message to the same process depending on the buffering mode (send + recv)
        replace = patterns.copy()
        replace['origin'] = 'RTED'
        replace['src1'] = '0'
        replace['dest1'] = '0'
        replace['src2'] = '1'
        replace['dest2'] = '1'
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ may not be matched'
        replace['longdesc'] = 'Processes 0 and 1 both call @{s}@ and @{r}@. This results in a deadlock depending on the buffering mode'
        replace['outcome1'] = 'ERROR: BufferingHazard'
        replace['errormsg1'] = f'Buffering Hazard. Possible deadlock depending the buffer size of MPI implementation and system environment cause Send message to the same process.'
        make_file(template, f'P2PBuffering_SameProcess_{s}_{r}_nok.c', replace)

        # Generate the incorrect matching with circular send message depending on the buffering mode (send + recv)
        replace = patterns.copy()
        replace['origin'] = 'RTED'
        replace['src1'] = '(nprocs - 1)'
        replace['dest1'] = '1'
        replace['src2'] = '0'
        replace['dest2'] = '2'
        replace['src3'] = '(rank - 1)'
        replace['dest3'] = '((rank + 1) % nprocs)'
        replace['fini1c'] = fini[r]("1")
        replace['fini2c'] = fini[r]("2")
        replace['operation1c'] = operation[s]("1") + ' /* MBIERROR3 */'
        replace['operation2c'] = operation[r]("2")
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ may not be matched'
        replace['longdesc'] = 'Processes 0 and 1 both call @{s}@ and @{r}@. This results in a deadlock depending on the buffering mode'
        replace['outcome1'] = 'ERROR: BufferingHazard'
        replace['errormsg1'] = f'Buffering Hazard. Possible deadlock depending the buffer size of MPI implementation and system environment cause circular send message.'
        make_file(template, f'P2PBuffering_Circular_{s}_{r}_nok.c', replace)

        # Generate the incorrect matching depending on the buffering mode (recv + send)
        replace = patterns
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ are not matched'
        replace['longdesc'] = 'Processes 0 and 1 both call @{r}@ and @{s}@. This results in a deadlock'
        replace['outcome1'] = 'ERROR: CallMatching'
        replace['errormsg1'] = 'ERROR: CallMatching'
        replace['operation1a'] =  operation[r]("2")
        replace['fini1a'] =  fini[r]("2")
        replace['operation2a'] = operation[s]("1")
        replace['fini2a'] =  fini[s]("1")
        replace['operation1b'] =  operation[r]("2")
        replace['fini1b'] =  fini[r]("2")
        replace['operation2b'] = operation[s]("1")
        replace['fini2b'] =  fini[s]("1")
        make_file(template, f'P2PCallMatching_{r}_{s}_{r}_{s}_nok.c', replace)

        # Generate the correct matching
        replace = patterns
        replace['shortdesc'] = 'Point to point @{s}@ and @{r}@ are correctly  matched'
        replace['longdesc'] = 'Process 0 calls @{s}@ and process 1 calls @{r}@.'
        replace['outcome1'] = 'OK'
        replace['errormsg1'] = 'OK'
        replace['fini1a'] =  fini[s]("1")
        replace['fini2a'] =  fini[r]("2")
        replace['operation1a'] =  operation[s]("1")
        replace['operation2a'] = operation[r]("2")
        make_file(template, f'P2PCallMatching_{s}_{r}_{r}_{s}_ok.c', replace)
