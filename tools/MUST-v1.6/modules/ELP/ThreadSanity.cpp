/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file ThreadSanity.cpp
 *       @see MUST::ThreadSanity.
 *
 *  @date 31.01.2014
 *  @author Felix Muenchhalfen
 */

#include "GtiMacros.h"
#include "ThreadSanity.h"
#include "MustEnums.h"
#include "I_InitParallelId.h"
#include "I_InitLocationId.h"

#include <sstream>
#include <iostream>
#include <dlfcn.h>

#define ELPDEBUG

using namespace must;

mGET_INSTANCE_FUNCTION(ThreadSanity);
mFREE_INSTANCE_FUNCTION(ThreadSanity);
mPNMPI_REGISTRATIONPOINT_FUNCTION(ThreadSanity);


// Must be static as each thread has it's own instance
// of this module but we need to have a common counter.
int ThreadSanity::MPI_thread_id = 0;
int ThreadSanity::MPI_counter = 0;
int ThreadSanity::MPIlevel_provided = -1;
pthread_mutex_t ThreadSanity::mod_lock = PTHREAD_MUTEX_INITIALIZER;

struct ELP_mem_access {
    unsigned long address;
    unsigned char stride;
    bool write;

    bool isReadAccess() {
        return !write;
    }

    bool isWriteAccess() {
        return write;
    }

    unsigned long getAddress() {
        return address;
    }

    unsigned char getStride() {
        return stride;
    }
};

//=============================
// Constructor
//=============================

ThreadSanity::ThreadSanity(const char* instanceName)
: gti::ModuleBase<ThreadSanity, I_ThreadSanity> (instanceName) {
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances();

    //handle sub modules
#define NUM_MODULES 5
    if (subModInstances.size() < NUM_MODULES) {
        std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
        assert(0);
    }
    if (subModInstances.size() > NUM_MODULES) {
        for (std::vector<I_Module*>::size_type i = NUM_MODULES; i < subModInstances.size(); i++)
            destroySubModuleInstance(subModInstances[i]);
    }

    myLogger = (I_CreateMessage*) subModInstances[0];
    myLIdInit = (I_InitLocationId*) subModInstances[1];
    myPIdInit = (I_InitParallelId*) subModInstances[2];
    myOMPTIntegration = (I_OMPTIntegration*) subModInstances[3];
    myPIdAnalysis = (I_ParallelIdAnalysis*) subModInstances[4];

    //Initialize module data
}

//=============================
// Destructor
//=============================

ThreadSanity::~ThreadSanity() {
    if (myLogger != NULL)
        destroySubModuleInstance(myLogger);
    myLogger = NULL;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyInitThreaded(
        MustParallelId pId,
        MustLocationId lId,
        int provided) {
#ifdef ELP_MODIFICATIONS
    pthread_mutex_lock(&mod_lock);
    MPIlevel_provided = provided;
    ParallelInfo pi = myPIdAnalysis->getInfoForId(pId);
    // Set the initializing thread as the main thread
    MPI_thread_id = pi.threadid;
    pthread_mutex_unlock(&mod_lock);
#endif
    
    /* ... */

    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::enterMPICall(
        MustParallelId pId,
        MustLocationId lId) {
    int val;
#ifdef _OPENMP
#pragma omp critical
    val = ++MPI_counter;
#else
    // This intrinsic is at least available in GCC, ICC and XL compiler. 
    val = __sync_add_and_fetch(&MPI_counter, 1);
#endif
#ifdef ELP_MODIFICATIONS
    unsigned int threadid;
    ParallelInfo pi = myPIdAnalysis->getInfoForId(pId);
    pthread_mutex_lock(&mod_lock);
    // If no main thread is yet set, set the first thread as the main thread.
    if( MPI_thread_id == 0 )
    {
        MPI_thread_id = pi.threadid;
    }
    threadid = MPI_thread_id;
    pthread_mutex_unlock(&mod_lock);
#endif
    
    if (MPIlevel_provided != MPI_THREAD_MULTIPLE && val > 1) {
        std::stringstream stream;
        stream
                << "Multiple threads call MPI functions simultaneously while you are not " <<
                "using MPI_THREAD_MULTIPLE. Current thread level is: ";

        if (MPIlevel_provided == -1)
            stream << "undefined";
        else if (MPIlevel_provided == MPI_THREAD_SINGLE)
            stream << "MPI_THREAD_SINGLE";
        else if (MPIlevel_provided == MPI_THREAD_FUNNELED)
            stream << "MPI_THREAD_FUNNELED";
        else if (MPIlevel_provided == MPI_THREAD_SERIALIZED)
            stream << "MPI_THREAD_SERIALIZED";
        else if (MPIlevel_provided == MPI_THREAD_MULTIPLE)
            stream << "MPI_THREAD_MULTIPLE";

        myLogger->createMessage(
                        MUST_ERROR_MPI_MULTIPLE_THREADS,
                        pId,
                        lId,
                        MustErrorMessage,
                        stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
#ifdef ELP_MODIFICATIONS
    else if( MPIlevel_provided == MPI_THREAD_FUNNELED && threadid != pi.threadid )
    {
        std::stringstream stream;
        stream
                << "You are using MPI_THREAD_FUNNELED and other threads than the main thread are making calls to MPI functions.";
        myLogger->createMessage(
                        MUST_ERROR_MPI_MULTIPLE_THREADS,
                        pId,
                        lId,
                        MustErrorMessage,
                        stream.str()
                );
    }
#endif
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::leaveMPICall(
        MustParallelId pId,
        MustLocationId lId) {
#ifdef _OPENMP
#pragma omp atomic
    MPI_counter--;
#else
    // This intrinsic is at least available in GCC, ICC and XL compiler. 
    __sync_fetch_and_sub(&MPI_counter, 1);
#endif
    return gti::GTI_ANALYSIS_SUCCESS;
}

// BELOW ALL DEPENDS ON OMPT being available

#ifdef GTI_OMPT_FOUND
std::vector<ParallelTeam*> ThreadSanity::teams;
ParallelApplication ThreadSanity::application;

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyParallelBegin(
        ompt_task_id_t parent_task_id,
        ompt_frame_t *parent_task_frame,
        ompt_parallel_id_t parallel_id,
        uint32_t requested_team_size,
        void *parallel_function) {
    /* ... */

    pthread_mutex_lock(&mod_lock);
    bool exists = (findTeamForParallelId(parallel_id) != NULL);
    if (!exists) {
#ifdef ELPDEBUG
        printf("MUST: New parallel team of size %i spawned!\n", requested_team_size);
#endif
        teams.push_back(new ParallelTeam(parallel_id, requested_team_size));
    }
    pthread_mutex_unlock(&mod_lock);
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyParallelEnd(
        ompt_parallel_id_t parallel_id,
        ompt_task_id_t task_id
        ) {
    pthread_mutex_lock(&mod_lock);
#ifdef ELPDEBUG
    printf("MUST: Parallel team ends! (ParReg: %li)\n", parallel_id);
#endif
    pthread_mutex_unlock(&mod_lock);
    return gti::GTI_ANALYSIS_SUCCESS;
}

#if defined(GTI_FOUND_LIBUNWIND)
/* jfm: This is hacky, i know,
 * but up to now my best approach
 * for distinguishing two different
 * barriers or code locations
 * (the intel omp runtime has no 
 * identifier for barriers!)
 * It only works if the function is forcibly
 * inlined.
 */

#ifdef __GNUC__

__attribute__((always_inline))
unsigned long long ThreadSanity::findFirstAddrOutsideOMPLib(char *name, const int maxlen) {
    unw_cursor_t cursor;
    unw_context_t uc;
    unw_word_t ip;

    Dl_info omp, peek;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    // resolve omp base addr here
    dladdr((void*) &omp_get_num_threads, &omp);

    // IMPORTANT: This function assumes it is called
    // from a module function which is called
    // out of the OMP runtime library
    while (unw_step(&cursor) > 0) {
        // find the first addr on the stack which is IN the omp runtime library
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        dladdr((void*) ip, &peek);
        if (omp.dli_fbase == peek.dli_fbase)
            break;
    }

    // then search for the first return addr which
    // lies below on the stack and is NOT in the omp runtime library
    while (unw_step(&cursor) > 0) {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        if (dladdr((void*) ip, &peek) && (peek.dli_fbase != omp.dli_fbase)) {
            unw_word_t crap;
            int error = unw_get_proc_name(&cursor, name, maxlen, &crap);
            return (unsigned long) ip;
        }
    }

    return 0;
}
#else

unsigned long long ThreadSanity::findFirstAddrOutsideOMPLib(char *name, const int maxlen) {
    return 0;
}
#endif
#else

unsigned long long ThreadSanity::findFirstAddrOutsideOMPLib(char *name, const int maxlen) {
    return 0;
}
#endif

ParallelTeam *ThreadSanity::findTeamForParallelId(ompt_parallel_id_t parallelid) {
    for (int i = 0; i < teams.size(); i++)
        if (teams[i]->id == parallelid) {
            ParallelTeam *ret = teams[i];
            return ret;
        }
    return NULL;
}

TeamThread *ThreadSanity::findThreadInTeam(ParallelTeam *team, uint32_t threadid) {
    /*for (int i = 0; i < team->threads.size(); i++)
        if (i == threadid) {
            return team->threads[i];
        }*/
    
    // Add threads until we reach the requested thread-index
    for(int i = team->threads.size(); i <= threadid; i++)
        team->threads.push_back(new TeamThread(team));
    
    return team->threads[threadid];
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyBarrierBegin(
        ompt_parallel_id_t parallel_id, /* id of parallel region       */
        ompt_task_id_t task_id /* id of task                  */
        ) {
    char name[200];
    name[0] = 0;
    unsigned long long barrier_position = findFirstAddrOutsideOMPLib(name, 200);
#ifdef ELPDEBUG
    printf("BARRIER: Parallel-Id: %li, Task-Id: %li, Code position: %lx, Proc name: %s\n", parallel_id, task_id, barrier_position, name);
#endif
    pthread_mutex_lock(&mod_lock);
    uint32_t tid = omp_get_thread_num();
    ParallelTeam *team = findTeamForParallelId(parallel_id);
    if (team) {
        TeamThread *thread = findThreadInTeam(team, tid);
        thread->isinbarrier = true;
        thread->barrier = barrier_position;
        for (int i = 0; i < team->threads.size(); i++) {
            if (tid == i)
                continue;
            if (team->threads[i]->isinbarrier && team->threads[i]->barrier != barrier_position) {
                
                char msg[1024];
                sprintf(msg, "Error: Thread %i of %i (Parallelid: %li) passes a different barrier than other threads of the same team!", tid, team->teamsize, team->id);                    
                IssueErrorMessage(msg, name, MUST_ERROR_OPENMP);
                break;
            }
        }
#ifdef ELPDEBUG
        printf("MUST: Marked thread %i of parallel team %li as in-barrier!\n", tid, parallel_id);
#endif
    }
    pthread_mutex_unlock(&mod_lock);
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyBarrierEnd(
        ompt_parallel_id_t parallel_id, /* id of parallel region       */
        ompt_task_id_t task_id /* id of task                  */
        ) {
#ifdef ELPDEBUG
    char name[200];
    name[0] = 0;
    unsigned long long barrier_position = findFirstAddrOutsideOMPLib(name, 200);
    printf("BARRIER-END: Parallel-Id: %li, Task-Id: %li, Code position: %lx, Proc name: %s\n", parallel_id, task_id, barrier_position, name);
#endif
    pthread_mutex_lock(&mod_lock);
    uint32_t tid = omp_get_thread_num();
    ParallelTeam *team = findTeamForParallelId(parallel_id);
    if (team) {
        TeamThread *thread = findThreadInTeam(team, tid);
        thread->isinbarrier = false;
#ifdef ELPDEBUG
        printf("MUST: Marked thread %i of parallel team %li as out-of-barrier!\n", tid, parallel_id);
#endif
    }
    pthread_mutex_unlock(&mod_lock);
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyBarrierWaitBegin(
        ompt_parallel_id_t parallel_id, /* id of parallel region       */
        ompt_task_id_t task_id /* id of task                  */
        ) {
#ifdef ELPDEBUG
    char name[200];
    name[0] = 0;
    unsigned long long barrier_position = findFirstAddrOutsideOMPLib(name, 200);
    printf("BARRIER-WAIT: Parallel-Id: %li, Task-Id: %li, Code position: %lx, Proc name: %s\n", parallel_id, task_id, barrier_position, name);
#endif
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyBarrierWaitEnd(
        ompt_parallel_id_t parallel_id, /* id of parallel region       */
        ompt_task_id_t task_id /* id of task                  */
        ) {
#ifdef ELPDEBUG
    char name[200];
    name[0] = 0;
    unsigned long long barrier_position = findFirstAddrOutsideOMPLib(name, 200);
    printf("BARRIER-WAIT-END: Parallel-Id: %li, Task-Id: %li, Code position: %lx, Proc name: %s\n", parallel_id, task_id, barrier_position, name);
#endif

    pthread_mutex_lock(&mod_lock);
    uint32_t tid = omp_get_thread_num();
    ParallelTeam *team = findTeamForParallelId(parallel_id);
    if (team) {
        TeamThread *thread = findThreadInTeam(team, tid);
        thread->isinbarrier = false;
#ifdef ELPDEBUG
        printf("MUST: Marked thread %i of parallel team %li as out-of-barrier!\n", tid, parallel_id);
#endif
    }
    pthread_mutex_unlock(&mod_lock);
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyInitLock (
                ompt_wait_id_t wait_id)
{
    bool found = CheckLockInitialized(wait_id);
    printf("OMPT: LOCK INITIALIZED!\n");
    if(!found)
    {
        pthread_mutex_lock(&mod_lock);
        application.initialized_locks.push_back(wait_id);
        pthread_mutex_unlock(&mod_lock);
    }
    else
    {
        char msg[1024];
        sprintf(msg, "Error: Trying to initialize a lock again. Lock %lx is already initialized.", wait_id);
        IssueErrorMessage(msg, "lock_doubleinit", MUST_ERROR_OPENMP);
    }
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyInitNestLock (
                ompt_wait_id_t wait_id)
{
    bool found = CheckNestLockInitialized(wait_id);
    
    if(!found)
    {
        pthread_mutex_lock(&mod_lock);
        application.initialized_nest_locks.push_back(wait_id);
        pthread_mutex_unlock(&mod_lock);
    }
    else
    {
        char msg[1024];
        sprintf(msg, "Error: Trying to initialize a nested lock again. Nested Lock %lx is already initialized.", wait_id);
        IssueErrorMessage(msg, "nested_lock_doubleinit", MUST_ERROR_OPENMP);
    }
    return gti::GTI_ANALYSIS_SUCCESS;
}

bool ThreadSanity::CheckLockInitialized( ompt_wait_id_t wait_id )
{
    pthread_mutex_lock(&mod_lock);
    bool found = false;
    for( int i = 0; i < application.initialized_locks.size(); i++ )
    {
        if( application.initialized_locks[i] == wait_id )
        {
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&mod_lock);
    return found;
}

bool ThreadSanity::CheckNestLockInitialized( ompt_wait_id_t wait_id )
{
    pthread_mutex_lock(&mod_lock);
    bool found = false;
    for( int i = 0; i < application.initialized_nest_locks.size(); i++ )
    {
        if( application.initialized_nest_locks[i] == wait_id )
        {
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&mod_lock);
    return found;
}

void ThreadSanity::IssueErrorMessage( char *msg, char *typestr, MustMessageIdNames messagetype )
{
    ThreadSanity *meself = (ThreadSanity*) getInstance("");
    if (meself != NULL && meself->myPIdInit != NULL && meself->myLIdInit != NULL) {
        MustParallelId pId;
        MustLocationId lId;
        
        // WORK IN PROGRESS (jfm))
        meself->myPIdInit->init(&pId);
        meself->myLIdInit->init(&lId, typestr, 999);
        meself->myLogger->createMessage(
                messagetype,
                pId,
                lId,
                MustErrorMessage,
                msg);
    }
}

std::vector<ompt_wait_id_t>::iterator TeamThread::FindLockInTeamThread( ompt_wait_id_t wait_id )
{
    for( int i = 0; i < locks.size(); i++)
    {
        if( locks[i] == wait_id )
        {
            return locks.begin()+i;
        }
    }
    return nest_locks.end();
}

std::vector<ompt_wait_id_t>::iterator TeamThread::FindNestLockInTeamThread( ompt_wait_id_t wait_id )
{
    for( int i = 0; i < nest_locks.size(); i++)
    {
        if( nest_locks[i] == wait_id )
        {
            return nest_locks.begin()+i;
        }
    }
    return nest_locks.end();
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyAcquiredLock (
                ompt_wait_id_t wait_id)
{
    printf("ACQUIRED LOCK!\n");
    bool found = CheckLockInitialized(wait_id);
    if(!found)
    {
        char msg[1024];
        sprintf(msg, "Error: Lock %lx was not initialized prior to usage.", wait_id);
        IssueErrorMessage(msg, "lock_not_initialized", MUST_ERROR_OPENMP);
    }
    else
    {
        /*ompt_parallel_id_t parallel_id = ompt_get_parallel_id(0);
        uint32_t tid = omp_get_thread_num();
        pthread_mutex_lock(&mod_lock);
        ParallelTeam *team = findTeamForParallelId(parallel_id);
        std::vector<ompt_wait_id_t>::iterator lock = team->threads[tid]->FindLockInTeamThread(wait_id)
        pthread_mutex_unlock(&mod_lock);
        if( lock == NULL )
            team->threads[tid]->locks.push_back(wait_id);
        else
        {
            // should never happen
            char msg[1024];
            sprintf(msg, "Error: Lock %lx was double-locked.", wait_id);
            IssueErrorMessage(msg, "lock_double_lock", MUST_ERROR_OPENMP);
        }*/
    }
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyWaitLock (
                ompt_wait_id_t wait_id)
{
    bool found = CheckLockInitialized(wait_id);
    if(!found)
    {
        char msg[1024];
        sprintf(msg, "Error: Lock %lx was not initialized prior to usage.", wait_id);
        IssueErrorMessage(msg, "lock_not_initialized", MUST_ERROR_OPENMP);
    }
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyAcquiredNestLock (
                ompt_wait_id_t wait_id)
{
    bool found = CheckNestLockInitialized(wait_id);
    if(!found)
    {
        char msg[1024];
        sprintf(msg, "Error: Nested Lock %lx was not initialized prior to usage.", wait_id);
        IssueErrorMessage(msg, "lock_not_initialized", MUST_ERROR_OPENMP);
    }
    else
    {

    }
    return gti::GTI_ANALYSIS_SUCCESS;
}

gti::GTI_ANALYSIS_RETURN ThreadSanity::notifyWaitNestLock (
                ompt_wait_id_t wait_id)
{
    bool found = CheckNestLockInitialized(wait_id);
    if(!found)
    {
        char msg[1024];
        sprintf(msg, "Error: Nested Lock %lx was not initialized prior to usage.", wait_id);
        IssueErrorMessage(msg, "lock_not_initialized", MUST_ERROR_OPENMP);
    }
    return gti::GTI_ANALYSIS_SUCCESS;
}

#endif
/*EOF*/
