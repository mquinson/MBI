#! /usr/bin/python3
import sys
from generator_utils import *

template = """// @{generatedby}@
/* ///////////////////////// The MPI Bugs Initiative ////////////////////////

  Origin: @{origin}@

  Description: @{shortdesc}@
    @{longdesc}@


BEGIN_MPI_FEATURES
  P2P!basic: Lacking
  P2P!nonblocking: Lacking
  P2P!persistent: Lacking
  COLL!basic: Lacking
  COLL!nonblocking: Lacking
  COLL!persistent: Lacking
  COLL!tools: Lacking
  RMA: @{rmafeature}@
END_MPI_FEATURES

BEGIN_MBI_TESTS
  $ mpirun -np 2 ${EXE}
  | @{outcome}@
  | @{errormsg}@
END_MBI_TESTS
//////////////////////       End of MBI headers        /////////////////// */

#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10

int * buffer;

void get_win(MPI_Win *win) {
  @{bufferalloc}@

  MPI_Win_create(@{buffer}@, N * sizeof(int), 1, MPI_INFO_NULL, MPI_COMM_WORLD, win);

  return;
}

int main(int argc, char *argv[]) {
  int rank, numProcs;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d \\n", rank);

  if (numProcs < 2)
    printf("MBI ERROR: This test needs at least 2 processes to produce a bug!\\n");

  MPI_Win win;

  get_win(&win);

  MPI_Win_fence(0, win);

  if (rank == 0) {
    int localbuf[N] = {12345};
    MPI_Put(&localbuf, N, MPI_INT, 1, 0, N, MPI_INT, win);
  }

  MPI_Win_fence(0, win);

  MPI_Win_free(&win);

  @{bufferfree}@

  MPI_Finalize();
  printf("Rank %d finished normally\\n", rank);
  return 0;
}

"""


for b in ['stack', 'missing', 'null',  'malloc', 'bufferSize']:
    patterns = {}
    patterns = {'b': b}
    patterns['origin'] = "MPI-CorrBench"
    patterns['generatedby'] = f'DO NOT EDIT: this file was generated by {os.path.basename(sys.argv[0])}. DO NOT EDIT.'
    patterns['rmafeature'] = 'Yes'

    replace = patterns
    replace['shortdesc'] = 'Invalid buffer in window creation.'
    replace['longdesc'] = 'Invalid buffer in window creation.'
    replace['outcome'] = 'ERROR: InvalidBuffer'
    replace['errormsg'] = '@{b}@ at @{filename}@:@{line:MBIERROR}@ has an invalid buffer'
    replace['bufferfree'] = ''

    ok = 'nok'
    replace['buffer'] = 'buffer'

    if b == 'stack':
        replace['bufferalloc'] = 'int buffer[N]; /* MBIERROR1 */'
        replace['buffer'] = '&buffer'
        replace['longdesc'] = 'Use a stack oriented buffer in window creation, buffer on temporary stack memory.'
    elif b == 'missing':
        replace['bufferalloc'] = '/* MBIERROR1 */'
        replace['longdesc'] = 'Uninitialized buffer in window creation.'
    elif b == 'null':
        replace['bufferalloc'] = 'buffer = NULL; /* MBIERROR1 */'
        replace['longdesc'] = 'Use NULL buffer in window creation.'
    elif b == 'bufferSize':
        replace['bufferalloc'] = 'buffer = malloc((N/2) * sizeof(int)); /* MBIERROR1 */'
        replace['bufferfree'] = 'free(buffer);'
        replace['longdesc'] = 'Unmatched size of buffer in window creation.'
    else:
        replace['bufferalloc'] = 'buffer = malloc(N * sizeof(int));'
        replace['bufferfree'] = 'free(buffer);'
        replace['longdesc'] = 'Correct initialized buffer in window creation.'
        replace['outcome'] = 'OK'
        replace['errormsg'] = ''
        ok = 'ok'

    make_file(template, f'InvalidParam_WinBuffer_{b}_{ok}.c', replace)
