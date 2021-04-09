/// -*- c++ -*-
/// Wrappers for the PnMPI callpath module.  These take a stacktrace 
/// at each PnMPI call so that it can be exposed for other modules.
#include <mpi.h>
#ifdef PNMPI_ENABLE_THREAD_SAFETY
    #include <pthread.h>
#endif
#include "Callpath.h"
#include "CallpathRuntime.h"
#include "callpath_internal.h"
using namespace std;

/// Runtime used to take stack traces.
static CallpathRuntime *runtime = NULL;
CallpathRuntime * getCallpathRuntime()
{
    if( runtime == NULL )
    {
        runtime = new CallpathRuntime();
        runtime->set_chop_libc(true);
    }
    return runtime;
}

{{fn fn_name MPI_Init MPI_Init_thread}} {
  CallpathRuntime *rt = getCallpathRuntime();
  PNMPIMOD_Callpath_callpath = rt->doStackwalk();
  {{callfn}}
} {{endfn}}


{{fnall fn_name MPI_Init MPI_Init_thread MPI_Finalize MPI_Pcontrol MPI_Comm_Rank MPI_Comm_size MPI_Initialized}} {
  CallpathRuntime *rt = getCallpathRuntime();
  PNMPIMOD_Callpath_callpath = rt->doStackwalk();
  {{callfn}}
} {{endfnall}}


{{fn fn_name MPI_Finalize}} {
  CallpathRuntime *rt = getCallpathRuntime();
  PNMPIMOD_Callpath_callpath = rt->doStackwalk();
  {{callfn}}
  delete runtime;
} {{endfn}}
