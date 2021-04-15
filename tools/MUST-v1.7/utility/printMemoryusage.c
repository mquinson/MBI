#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#ifndef PMPIPREFIX
#define PMPIPREFIX PMPI
#endif

#define PMPIZE_T(f,p) p ## f
#define PMPIZE_H(f,p) PMPIZE_T(f,p)
#define PMPIZE(f) PMPIZE_H(f,PMPIPREFIX)

int PMPIZE(_Finalize)();
int MPI_Finalize() {
  int rank, print_rank=1, all=0;
  char* print_str = getenv("MUST_PRINT_RANK");
  if (print_str!=NULL)
  {
    if (strcmp("all", print_str)==0)
      all=1;
    else
      print_rank=atoi(print_str);
  }
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (all || rank == print_rank)
  {
    struct rusage end;
    getrusage(RUSAGE_SELF, &end);
    printf("%i: MAX RSS[KBytes] during execution: %ld\n", rank, end.ru_maxrss);
  }
  return PMPIZE(_Finalize)();
}
