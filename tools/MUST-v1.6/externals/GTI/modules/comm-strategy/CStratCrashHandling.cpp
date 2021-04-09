/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file CStratCrashHandling.cpp
 *  Extension to communication strategies to handle crashes of the application.
 *
 *  This Extensions provides handlers for MPI-Errors and signals. By the use of
 * this handlers the tool processes can finish all analyses before the 
 * application gets stopped.
 *
 *
 * @author Joachim Protze
 * @date 20.03.2012
 * 
 */

#include <signal.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pnmpimod.h>
#include <unistd.h>
#include <assert.h>
#include <gtiConfig.h>
#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>

#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_ 
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */


#define GTI_SPLIT_MODULE_NAME "split_processes"

/* Switch between MPI-1.2 and MPI-2 errorhandler */
#ifdef HAVE_MPI_COMM_CREATE_ERRHANDLER
#define GTI_COMM_CREATE_ERRHANDLER(f,e) PMPI_Comm_create_errhandler(f,e)
#else
#define GTI_COMM_CREATE_ERRHANDLER(f,e) PMPI_Errhandler_create(f,e)
#endif

#ifdef HAVE_MPI_COMM_SET_ERRHANDLER
#define GTI_COMM_SET_ERRHANDLER(f,e) if (f != MPI_COMM_NULL && e != 0) PMPI_Comm_set_errhandler(f,e)
#else
#define GTI_COMM_SET_ERRHANDLER(f,e) if (f != MPI_COMM_NULL && e != 0) PMPI_Errhandler_set(f,e)
#endif

#ifdef GTI_STRAT_RAISE_PANIC
/**
 * Declaration of the panic function that the communication strategy should implement.
 * Use the macro mCOMM_STRATEGY_UP_RAISE_PANIC from GtiMacros.h for a default
 * implementation.
 */
void strategyRaisePanic (void);
#endif



#define CALLSTACK_SIZE 10

static void print_stack(void) {
    int i, nptrs;
    void *buf[CALLSTACK_SIZE + 1];
    char **strings;

    nptrs = backtrace(buf, CALLSTACK_SIZE);

    backtrace_symbols_fd(buf, nptrs, STDERR_FILENO);
}

MPI_Errhandler gtiMpiCommErrorhandler;
int gtiMpiCrashRank, gtiMpiCrashSize;

void myMpiErrHandler(MPI_Comm * comm, int * errCode, ...){
    printf("rank %i (of %i), pid %i caught MPI error nr %i\n", gtiMpiCrashRank, gtiMpiCrashSize, getpid(), *errCode);
    char error_string[BUFSIZ]; 
    int length_of_error_string;
    PMPI_Error_string(*errCode, error_string, &length_of_error_string);
    printf("%s\n", error_string);
    print_stack();

#ifdef GTI_STRAT_RAISE_PANIC
    strategyRaisePanic ();
#endif
    printf("Waiting up to 30 seconds for analyses to be finished.\n");
    sleep(30);
    exit(1);
}


void mySignalHandler(int signum){
    printf("rank %i (of %i), pid %i caught signal nr %i\n", gtiMpiCrashRank, gtiMpiCrashSize, getpid(), signum);
    if (signum==SIGINT || signum==SIGKILL)
        MPI_Abort(MPI_COMM_WORLD, signum+128);
    print_stack();
#ifdef GTI_STRAT_RAISE_PANIC
    strategyRaisePanic ();
#endif
    printf("Waiting up to 30 seconds for analyses to be finished.\n");
    sleep(30);
    exit(1);
}



void crashHandlingInit(){
    int err;
    MPI_Comm            this_set_comm;
    GTI_COMM_CREATE_ERRHANDLER(myMpiErrHandler, &gtiMpiCommErrorhandler);
    /**
     * @note we do not use GTI_COMM_SET_ERRHANDLER since the "if" in there
     * causes a segfault with Xcode 7.0.1 on OSX. The if is not needed so
     * we simply avoid it.
     */
    //GTI_COMM_SET_ERRHANDLER(MPI_COMM_SELF, gtiMpiCommErrorhandler);
#ifdef HAVE_MPI_COMM_SET_ERRHANDLER
    PMPI_Comm_set_errhandler(MPI_COMM_SELF,gtiMpiCommErrorhandler);
#else
    PMPI_Errhandler_set(MPI_COMM_SELF,gtiMpiCommErrorhandler);
#endif

    // === (2) Query for services of the split module ===
    PNMPI_modHandle_t handle;
    PNMPI_Service_descriptor_t service;
    PNMPI_Service_Fct_t fct;
#ifdef PNMPI_FIXED
    err = PNMPI_Service_GetModuleByName(GTI_SPLIT_MODULE_NAME, &handle);
#else
    char string[512];
    sprintf (string, "%s",GTI_SPLIT_MODULE_NAME);
    err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
    /**
     * @note we do not use GTI_COMM_SET_ERRHANDLER since the "if" in there
     * causes a segfault with Xcode 7.0.1 on OSX. The if is not needed so
     * we simply avoid it.
     */
    //GTI_COMM_SET_ERRHANDLER(MPI_COMM_WORLD, gtiMpiCommErrorhandler);
#ifdef HAVE_MPI_COMM_SET_ERRHANDLER
        PMPI_Comm_set_errhandler(MPI_COMM_WORLD,gtiMpiCommErrorhandler);
#else
        PMPI_Errhandler_set(MPI_COMM_WORLD,gtiMpiCommErrorhandler);
#endif
    if (err != PNMPI_SUCCESS)
    {
        PMPI_Comm_size(MPI_COMM_WORLD, &gtiMpiCrashSize);
        PMPI_Comm_rank(MPI_COMM_WORLD, &gtiMpiCrashRank);
    }
    else
    {
        err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetComm", "p", &service);
        assert (err == PNMPI_SUCCESS);
        ((int(*)(void*)) service.fct) (&this_set_comm);
        GTI_COMM_SET_ERRHANDLER(this_set_comm, gtiMpiCommErrorhandler);
        PMPI_Comm_size(this_set_comm, &gtiMpiCrashSize);
        PMPI_Comm_rank(this_set_comm, &gtiMpiCrashRank);
    }


    signal(SIGSEGV, mySignalHandler);
    signal(SIGINT, mySignalHandler);
    signal(SIGHUP, mySignalHandler);
    // SIGTERM should come from the outside, make sure to quit
//    signal(SIGTERM, mySignalHandler);
    signal(SIGABRT, mySignalHandler);
    // SIGTRAP is used by stackwalker
//    signal(SIGTRAP, mySignalHandler);
    signal(SIGQUIT, mySignalHandler);
    signal(SIGALRM, mySignalHandler);
    // SIGKILL is uncatchable
//     signal(SIGKILL, mySignalHandler);

//     int jmpflag = sigsetjmp(sigjumper, 1);
//     // try
//     if (jmpflag == 0)
//     {
//
//     }else{
//
//         pause();
//     }
}


_EXTERN_C_ int MPI_Init (int *pArgc, char ***pArgv)
{
    int ret, err;
    ret = PMPI_Init (pArgc, pArgv);

    crashHandlingInit();
    return ret;
}

_EXTERN_C_ int MPI_Init_thread (int *pArgc, char ***pArgv, int request, int *provided)
{
    int ret;
    ret = PMPI_Init_thread (pArgc, pArgv, request, provided);

    crashHandlingInit();

    return ret;
}

#ifdef HAVE_MPI_COMM_SPAWN

// MPI-2
_EXTERN_C_ int MPI_Comm_spawn(
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS 
  const 
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  char *command,
  char *argv[],
  int maxprocs,
  MPI_Info info,
  int root,
  MPI_Comm comm,
  MPI_Comm *intercomm,
  int array_of_errcodes[]
)
{
    int ret = PMPI_Comm_spawn(command, argv, maxprocs, info, root, comm, intercomm, array_of_errcodes);
    GTI_COMM_SET_ERRHANDLER(*intercomm, gtiMpiCommErrorhandler);
    return ret;
}

// MPI-2
_EXTERN_C_ int MPI_Comm_spawn_multiple(
  int count,
  char *array_of_commands[],
  char* *array_of_argv[],
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  int array_of_maxprocs[],
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  MPI_Info array_of_info[],
  int root,
  MPI_Comm comm,
  MPI_Comm *intercomm,
  int array_of_errcodes[]
)
{
    int ret = PMPI_Comm_spawn_multiple(count, array_of_commands, array_of_argv, array_of_maxprocs, array_of_info, root, comm, intercomm, array_of_errcodes);
    GTI_COMM_SET_ERRHANDLER(*intercomm, gtiMpiCommErrorhandler);
    return ret;
}

// MPI-2
_EXTERN_C_ int MPI_Comm_join(
  int fd,
  MPI_Comm *intercomm
)
{
    int ret = PMPI_Comm_join(fd, intercomm);
    GTI_COMM_SET_ERRHANDLER(*intercomm, gtiMpiCommErrorhandler);
    return ret;
}

// MPI-2
_EXTERN_C_ int MPI_Comm_accept(
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  char *port_name,
  MPI_Info info,
  int root,
  MPI_Comm comm,
  MPI_Comm *newcomm
)
{
    int ret = PMPI_Comm_accept(port_name, info, root, comm, newcomm);
    GTI_COMM_SET_ERRHANDLER(*newcomm, gtiMpiCommErrorhandler);
    return ret;
}

// MPI-2
_EXTERN_C_ int MPI_Comm_connect(
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  char *port_name,
  MPI_Info info,
  int root,
  MPI_Comm comm,
  MPI_Comm *newcomm
)
{
    int ret = PMPI_Comm_connect(port_name, info, root, comm, newcomm);
    GTI_COMM_SET_ERRHANDLER(*newcomm, gtiMpiCommErrorhandler);
    return ret;
}
#endif

_EXTERN_C_ int MPI_Comm_create(
  MPI_Comm comm,
  MPI_Group group,
  MPI_Comm *newcomm
)
{
    int ret = PMPI_Comm_create(comm, group, newcomm);
    GTI_COMM_SET_ERRHANDLER(*newcomm, gtiMpiCommErrorhandler);
    return ret;
}

_EXTERN_C_ int MPI_Comm_dup(
  MPI_Comm comm,
  MPI_Comm *newcomm
)
{
    int ret = PMPI_Comm_dup(comm, newcomm);
    GTI_COMM_SET_ERRHANDLER(*newcomm, gtiMpiCommErrorhandler);
    return ret;
}

_EXTERN_C_ int MPI_Comm_split(
  MPI_Comm comm,
  int color,
  int key,
  MPI_Comm *newcomm
)
{
    int ret = PMPI_Comm_split(comm, color, key, newcomm);
    GTI_COMM_SET_ERRHANDLER(*newcomm, gtiMpiCommErrorhandler);
    return ret;
}

_EXTERN_C_ int MPI_Cart_sub(
  MPI_Comm comm,
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  int *remain_dims,
  MPI_Comm *newcomm
)
{
    int ret = PMPI_Cart_sub(comm, remain_dims, newcomm);
    GTI_COMM_SET_ERRHANDLER(*newcomm, gtiMpiCommErrorhandler);
    return ret;
}


_EXTERN_C_ int MPI_Intercomm_create(
  MPI_Comm local_comm,
  int local_leader,
  MPI_Comm peer_comm,
  int remote_leader,
  int tag,
  MPI_Comm *newintercomm
)
{
    int ret = PMPI_Intercomm_create( local_comm, local_leader, peer_comm, remote_leader, tag, newintercomm );
    GTI_COMM_SET_ERRHANDLER(*newintercomm, gtiMpiCommErrorhandler);
    return ret;
}

_EXTERN_C_ int MPI_Intercomm_merge(
  MPI_Comm intercomm,
  int high,
  MPI_Comm *newintracomm
)
{
    int ret = PMPI_Intercomm_merge( intercomm, high, newintracomm );
    GTI_COMM_SET_ERRHANDLER(*newintracomm, gtiMpiCommErrorhandler);
    return ret;
}

_EXTERN_C_ int MPI_Graph_create(
  MPI_Comm comm_old,
  int nnodes,
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  int *index,
#ifndef HAVE_MPI_NO_CONST_CORRECTNESS
  const
#endif /*HAVE_MPI_NO_CONST_CORRECTNESS*/
  int *edges,
  int reorder,
  MPI_Comm *comm_graph
)
{
    int ret = PMPI_Graph_create( comm_old, nnodes, index, edges, reorder, comm_graph );
    GTI_COMM_SET_ERRHANDLER(*comm_graph, gtiMpiCommErrorhandler);
    return ret;
}

