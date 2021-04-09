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
 * @file ThreadSanity.h
 *       @see MUST::ThreadSanity.
 *
 *  @date 31.01.2014
 *  @author Felix Muenchhalfen
 */

#include "ModuleBase.h"
#include "I_ThreadSanity.h"
#include "I_CreateMessage.h"
#include "I_InitLocationId.h"
#include "I_InitParallelId.h"
#include "I_LocationAnalysis.h"
#include "I_OMPTIntegration.h"
#include "I_ParallelIdAnalysis.h"
#include "MustEnums.h"
#include <string>
#include <vector>

#ifdef GTI_FOUND_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

#include <omp.h>
#ifdef GTI_OMPT_FOUND
#include <ompt.h>
#endif

#ifndef ThreadSanity_H
#define ThreadSanity_H

using namespace gti;

namespace must {
    /* BEGIN HELPER */
#ifdef GTI_OMPT_FOUND
    class ParallelTeam;

    /**
     * Structure to hold information about a certain thread inside
     * a parallel team
     */
    struct TeamThread {
        uint64_t barrier;
        bool isinbarrier;
        ParallelTeam *team;
        std::vector<ompt_wait_id_t> locks;
        std::vector<ompt_wait_id_t> nest_locks;

        TeamThread(ParallelTeam *a_team) : team(a_team), isinbarrier(false), barrier(0) {
        }
        
        /**
         * Finds an iterator to an aquired lock
         */
        std::vector<ompt_wait_id_t>::iterator FindLockInTeamThread( ompt_wait_id_t wait_id );
        /**
         * Finds an iterator to an aquired nested lock
         */
        std::vector<ompt_wait_id_t>::iterator FindNestLockInTeamThread( ompt_wait_id_t wait_id );
    };

    /**
     * Structure to hold information about a parallel team and it's threads
     */
    struct ParallelTeam {
        ompt_parallel_id_t id;
        uint32_t teamsize;
        std::vector<TeamThread*> threads;

        ParallelTeam(ompt_parallel_id_t a_id, uint32_t a_teamsize) : id(a_id), teamsize(a_teamsize) {
            for (int i = 0; i < teamsize; i++)
                threads.push_back(new TeamThread(this));
        }

        ~ParallelTeam() {
            for (int i = 0; i < teamsize; i++)
                delete threads[i];
        }
    };

    struct ParallelApplication {
        std::vector<ompt_wait_id_t> initialized_locks;
        std::vector<ompt_wait_id_t> initialized_nest_locks;

        ParallelApplication() {
        };

        ~ParallelApplication() {
        };
    };
    /* END HELPER */
#endif

    /**
     * Implementation for I_ThreadSanity.
     */
    class ThreadSanity : public gti::ModuleBase<ThreadSanity, I_ThreadSanity> {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        ThreadSanity(const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~ThreadSanity(void);

        /* ANALYSIS */
        gti::GTI_ANALYSIS_RETURN notifyInitThreaded(
                MustParallelId pId,
                MustLocationId lId,
                int provided);

#ifdef GTI_OMPT_FOUND
        /* ANALYSIS */
        gti::GTI_ANALYSIS_RETURN notifyParallelBegin(
                ompt_task_id_t parent_task_id,
                ompt_frame_t *parent_task_frame,
                ompt_parallel_id_t parallel_id,
                uint32_t requested_team_size,
                void *parallel_function);

        gti::GTI_ANALYSIS_RETURN notifyParallelEnd(
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id);

        gti::GTI_ANALYSIS_RETURN notifyBarrierBegin(
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id);

        gti::GTI_ANALYSIS_RETURN notifyBarrierEnd(
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id);

        gti::GTI_ANALYSIS_RETURN notifyBarrierWaitBegin(
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id);

        gti::GTI_ANALYSIS_RETURN notifyBarrierWaitEnd(
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id);
        
        gti::GTI_ANALYSIS_RETURN notifyInitLock(
                ompt_wait_id_t wait_id);

        gti::GTI_ANALYSIS_RETURN notifyInitNestLock (
                ompt_wait_id_t wait_id);
        
        bool CheckLockInitialized(ompt_wait_id_t wait_id);
        bool CheckNestLockInitialized(ompt_wait_id_t wait_id);
        
        gti::GTI_ANALYSIS_RETURN notifyAcquiredLock (
                ompt_wait_id_t wait_id);

        gti::GTI_ANALYSIS_RETURN notifyWaitLock (
                        ompt_wait_id_t wait_id);

        gti::GTI_ANALYSIS_RETURN notifyAcquiredNestLock (
                        ompt_wait_id_t wait_id);

        gti::GTI_ANALYSIS_RETURN notifyWaitNestLock (
                        ompt_wait_id_t wait_id);

#endif

        /* ANALYSIS */
        gti::GTI_ANALYSIS_RETURN enterMPICall(MustParallelId pId, MustLocationId lId);
        /* ANALYSIS */
        gti::GTI_ANALYSIS_RETURN leaveMPICall(MustParallelId pId, MustLocationId lId);
    protected:
        /**
         * Logger
         */
        I_CreateMessage* myLogger;
        /**
         * I_InitLocationId interface to generated location id's when necessary
         */
        I_InitLocationId* myLIdInit;
        /**
         * I_InitParallelId interface to generated parallel id's when necessary
         */
        I_InitParallelId* myPIdInit;
        /**
         * I_OMPTIntegration interface to functionality of OMPT
         */
        I_OMPTIntegration* myOMPTIntegration;
        /**
         * I_ParallelIdAnalysis interface to resolve info's from parallel id's
         */
        I_ParallelIdAnalysis* myPIdAnalysis;
        
    private:
#ifdef GTI_OMPT_FOUND
        /**
         * Helper function to find first address outside the OMP library
         * necessary to compare barrier entry points against each other
         */
        static unsigned long long findFirstAddrOutsideOMPLib(char *name, const int maxlen);
        /**
         * Finds or generated a parallel team for parallel-id 'parallelid'
         */
        static ParallelTeam *findTeamForParallelId(ompt_parallel_id_t parallelid);
        /**
         * Finds or generated a team thread for ParallelTeam 'team'
         */
        TeamThread *findThreadInTeam(ParallelTeam *team, uint32_t threadid);
        
        /**
         * ParallelTeam vector
         */
        static std::vector<ParallelTeam*> teams;
        /**
         * ParallelApplication structure
         */
        static ParallelApplication application;
        /**
         * Helperfunction to send error messages
         */
        void IssueErrorMessage( char *msg, char *typestr, MustMessageIdNames messagetype );
#endif
        /**
         * lock to synchronize concurrent accesses to the ParallelTeam('s)
         */
        static pthread_mutex_t mod_lock;
        
        static int MPI_thread_id;
        static int MPI_counter;
        static int MPIlevel_provided;
    };
} /*namespace MUST*/

#endif /*ThreadSanity_H*/
