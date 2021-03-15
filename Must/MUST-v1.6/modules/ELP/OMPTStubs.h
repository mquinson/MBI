/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#ifndef OMPTSTUBS_H
#define	OMPTSTUBS_H

            /* MANDATORY EVENTS */
            static void OMPT_Event_Parallel_Begin( 
                                    ompt_task_id_t parent_task_id,
                                    ompt_frame_t *parent_task_frame,
                                    ompt_parallel_id_t parallel_id,
                                    uint32_t requested_team_size,
                                    void *parallel_function
                                );
            
            static void OMPT_Event_Parallel_End(
                                    ompt_parallel_id_t parallel_id,
                                    ompt_task_id_t task_id
                                );
            
            static void OMPT_Event_Task_Begin(
                                    ompt_task_id_t parent_task_id,    /* id of parent task            */
                                    ompt_frame_t *parent_task_frame,  /* frame data for parent task   */
                                    ompt_task_id_t  new_task_id,      /* id of created task           */
                                    void *task_function               /* pointer to outlined function */
                                ) {}
            
            static void OMPT_Event_Task_End(
                                    ompt_task_id_t parent_task_id,    /* id of parent task            */
                                    ompt_frame_t *parent_task_frame,  /* frame data for parent task   */
                                    ompt_task_id_t  new_task_id,      /* id of created task           */
                                    void *task_function               /* pointer to outlined function */
                                ) {}
            
            static void OMPT_Event_Thread_Begin(
                                    ompt_thread_type_t thread_type,   /* type of thread               */
                                    ompt_thread_id_t thread_id        /* ID of thread                 */
                                ) {}
            
            static void OMPT_Event_Thread_End(
                                    ompt_thread_type_t thread_type,   /* type of thread               */
                                    ompt_thread_id_t thread_id        /* ID of thread                 */
                                ) {}
            
            static void OMPT_Event_Control(
                                    uint64_t command,                 /* command of control call      */
                                    uint64_t modifier                 /* modifier of control call     */
                                ) {}
            
            static void OMPT_Event_Runtime_Shutdown(
                                    ompt_parallel_id_t parallel_id, 
                                    ompt_task_id_t task_id
                                ) {} 
            
            /* OPTIONAL EVENTS */
            
            static void OMPT_Event_Idle_Begin(
                                    ompt_thread_id_t thread_id        /* ID of thread                 */
                                ) {}
            
            static void OMPT_Event_Idle_End(
                                    ompt_thread_id_t thread_id        /* ID of thread                 */
                                ) {}
            
            static void OMPT_Event_Wait_Barrier_Begin(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                );
            
            static void OMPT_Event_Wait_Barrier_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                );

// Not yet implemented in OMPT  
//            static void OMPT_Event_Wait_Taskwait_Begin(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Wait_Taskwait_End(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Wait_Taskgroup_Begin(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Wait_Taskgroup_End(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
            
            static void OMPT_Event_Release_Lock(
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}
            
            static void OMPT_Event_Release_Nest_Lock_Last(
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}
            
            static void OMPT_Event_Release_Critical(
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}
            
            static void OMPT_Event_Release_Atomic(
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}
            
            static void OMPT_Event_Implicit_Task_Begin(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Implicit_Task_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}

// Not yet implemented in OMPT
//            static void OMPT_Event_Initial_Task_Begin(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Initial_Task_End(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Task_Switch(
//                                    ompt_task_id_t suspended_task_id, /* tool data for suspended task */
//                                    ompt_task_id_t resumed_task_id    /* tool data for resumed task   */
//                                ) {}
            
            static void OMPT_Event_Loop_Begin(
                                    ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                                    ompt_task_id_t parent_task_id,    /* id of parent task            */
                                    void *workshare_function          /* pointer to outlined function */
                                ) {}
            
            static void OMPT_Event_Loop_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Sections_Begin(
                                    ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                                    ompt_task_id_t parent_task_id,    /* id of parent task            */
                                    void *workshare_function          /* pointer to outlined function */
                                ) {}
            
            static void OMPT_Event_Sections_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Single_In_Block_Begin(
                                    ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                                    ompt_task_id_t parent_task_id,    /* id of parent task            */
                                    void *workshare_function          /* pointer to outlined function */
                                ) {}
            
            static void OMPT_Event_Single_In_Block_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Single_In_Others_Begin(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Single_In_Others_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}

// Not yet implemented in OMPT
//            static void OMPT_Event_Workshare_Begin(
//                                    ompt_parallel_id_t parallel_id,   /* id of parallel region        */
//                                    ompt_task_id_t parent_task_id,    /* id of parent task            */
//                                    void *workshare_function          /* pointer to outlined function */
//                                ) {}
//            
//            static void OMPT_Event_Workshare_End(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
            
            static void OMPT_Event_Master_Begin(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Master_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                ) {}
            
            static void OMPT_Event_Barrier_Begin(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                );
            
            static void OMPT_Event_Barrier_End(
                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                                    ompt_task_id_t task_id             /* id of task                  */
                                );

// Not yet implemented in OMPT
//            static void OMPT_Event_Taskwait_Begin(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Taskwait_End(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Taskgroup_Begin(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
//            
//            static void OMPT_Event_Taskgroup_End(
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
            
            static void OMPT_Event_Release_Nest_Lock_Prev( 
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}

// Not yet implemented in OMPT            
//            static void OMPT_Event_Wait_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Wait_Nest_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Wait_Critical( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
            
            static void OMPT_Event_Wait_Atomic( 
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}
            
            static void OMPT_Event_Wait_Ordered( 
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}

// Not yet implemented in OMPT
//            static void OMPT_Event_Acquired_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Acquired_Nest_Lock_First( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Acquired_Nest_Lock_Next( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Acquired_Critical( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
            
            static void OMPT_Event_Acquired_Atomic( 
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}
            
            static void OMPT_Event_Acquired_Ordered( 
                                    ompt_wait_id_t wait_id            /* wait id                      */
                                ) {}

// Not yet implemented in OMPT
//            static void OMPT_Event_Init_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Init_Nest_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Destroy_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Destroy_Nest_Lock( 
//                                    ompt_wait_id_t wait_id            /* wait id                      */
//                                ) {}
//            
//            static void OMPT_Event_Event_Flush( 
//                                    ompt_parallel_id_t parallel_id,    /* id of parallel region       */
//                                    ompt_task_id_t task_id             /* id of task                  */
//                                ) {}
            
            /* OpenMP 4.0 Events */

#endif	/* OMPTSTUBS_H */

