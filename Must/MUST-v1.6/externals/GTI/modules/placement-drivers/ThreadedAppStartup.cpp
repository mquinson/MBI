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
 * @file ThreadedAppStartup.cpp
 * Starts the Tool thread which then executes an instance of
 * ThreadedAppPlace
 *
 *
 * @author Felix Münchhalfen
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>

#include <pnmpimod.h>
#include <dlfcn.h>
#include "ThreadedAppStartup.h"
#include "GtiMacros.h"

static pthread_t m_Thread;
static pthread_once_t thread_starter = PTHREAD_ONCE_INIT;
static int ThreadPlaceInitialized = 0;

void start_worker_thread(void)
{
    static int inited=0;
    if (inited)
        return;
    inited++;
    pthread_create( &(m_Thread), NULL, threadProc, NULL );
}

void start() {
    pthread_once( &thread_starter, start_worker_thread );
}

void finish() {
    static int finished=-1;
    finished++;
    if(finished!=1)
        return;
    pthread_join( m_Thread, NULL );
}

    extern "C" {
    void __attribute__((weak)) AnnotateIgnoreWritesBegin(const char *file, int line){}
    void __attribute__((weak)) AnnotateIgnoreWritesEnd(const char *file, int line){}
    }
    # define TsanIgnoreWritesBegin() AnnotateIgnoreWritesBegin(__FILE__, __LINE__)
    # define TsanIgnoreWritesEnd() AnnotateIgnoreWritesEnd(__FILE__, __LINE__)

void *threadProc( void *thread_data )
{
    // execute the tool thread in stack 1
    // initialize libcProtMpiSplited.so and then libthreadedAppPlace.so !
    int stack, ret;
    ret = PNMPI_Service_GetStackByName("level_1", &stack);
    assert(ret == PNMPI_SUCCESS);
    TsanIgnoreWritesBegin();    
    ret = XMPI_Init_thread_NewStack(stack, NULL, NULL, 0, NULL);
    assert(ret == MPI_SUCCESS);
    TsanIgnoreWritesEnd();    
//    assert(XMPI_Finalize_NewStack(stack) == MPI_SUCCESS);
    return 0;
}

//=============================
// MPI_Init Wrapper
//=============================
extern "C" int MPI_Init (int* argc, char*** argv)
{
    //Startup the place
    start_worker_thread ();
    
    int err = PMPI_Init (argc, argv);

    return err;
}

//=============================
// MPI_Init_thread Wrapper
//=============================
extern "C" int MPI_Init_thread (int* argc, char*** argv, int required, int* provided)
{
    //Startup the place
    start_worker_thread ();
    
    int err = PMPI_Init_thread (argc, argv, required, provided);

    return err;
}

extern "C" int MPI_Finalize ()
{

    finish();
    int err = PMPI_Finalize ();
    return err;
}


/*EOF*/
