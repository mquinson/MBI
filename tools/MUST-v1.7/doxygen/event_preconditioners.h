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
 * @page EventPreconditioner Preconditioners for MPI Events
 *
 * @section EventPreconditionerOverview Overview
 *
 * The Preconditioners are:
 * 
 * - CollectiveCondition: for splitting the collective communication calls to send and recv calls.
 *
 * - RequestCondition: analyse the various test and wait calls to generate events just for finished requests.
 * 
 * 
 * 
 * @section CollectiveCondition CollectiveCondition
 * 
 * @subsection CollectiveConditionApi Summary
 * 
 * Summary of the API-Functions:
 * <table border>
 *   <tr>
 *     <th>Variant</th>
 *     <th>send</th>
 *     <th>op_send</th>
 *     <th>recv</th>
 *     <th>op_recv</th>
 *     <th>explanation</th>
 *   </tr><tr>
 *     <td>(single)</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>o</td>
 *     <td>communicate with single partner</td>
 *   </tr><tr>
 *     <td>_n</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>o</td>
 *     <td>X</td>
 *     <td>communicate same stuff to multiple partners</td>
 *   </tr><tr>
 *     <td>_buffers</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>o</td>
 *     <td>communicate to multiple partners using various buffers</td>
 *   </tr><tr>
 *     <td>_counts</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>X</td>
 *     <td>o</td>
 *     <td>communicate to multiple partners using various buffers and counts</td>
 *   </tr><tr>
 *     <td>_types</td>
 *     <td>X</td>
 *     <td>o</td>
 *     <td>X</td>
 *     <td>o</td>
 *     <td>communicate to multiple partners using various buffers, counts and types</td>
 *   </tr>
 * </table>
 * 
 *
 * @subsection CollectiveConditionApiExample Example
 * Example for a generated Op_Send_n event
 * @param pId parallel context.
 * @param lId location id of context.
 * @param coll enum representing the former call.
 * @param buffer address of transfer buffer.
 * @param count number of repetitions.
 * @param type of transfered data
 * @param op operation executed while reducing
 * @param commsize size of the communication group
 * @param comm used communicator
 * \code
inline int PMust_Coll_Op_Send_n (
        MustParallelId pId,
        MustLocationId lId
        MustCollCommType coll,
        void *buffer,
        int count,
        MustDatatypeType type,
        MustOperationType op,
        int commsize,
        MustCommType comm
        )  {return 0;}
 * \endcode
 * 
 * 
 * @subsection CollectiveConditionApiMappings Mappings
 * Each collective communication event is conditionally mapped to a send and a receive event.
 * One-to-all and all-to-one communications generate the to-all event just on the root rank.
 * The events are considered to be reducible, when the values of coll, (count*type) (, op) and comm match semantically.
 * <table border>
 *   <tr>
 *     <th>MPI-call</th>
 *     <th>send on</th>
 *     <th>MUST-send</th>
 *     <th>recv on</th>
 *     <th>MUST-recv</th>
 *   </tr><tr>
 *     <td>MPI_Gather</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send</td>
 *     <td>root</td>
 *     <td>Must_Coll_Recv_buffers</td>
 *   </tr><tr>
 *     <td>MPI_Gatherv</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send</td>
 *     <td>root</td>
 *     <td>Must_Coll_Recv_counts</td>
 *   </tr><tr>
 *     <td>MPI_Reduce</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Send</td>
 *     <td>root</td>
 *     <td>Must_Coll_Op_Recv_n</td>
 *   </tr><tr>
 *     <td>MPI_Bcast</td>
 *     <td>root</td>
 *     <td>Must_Coll_Send_n</td>
 *     <td>all-but-root</td>
 *     <td>Must_Coll_Recv</td>
 *   </tr><tr>
 *     <td>MPI_Scatter</td>
 *     <td>root</td>
 *     <td>Must_Coll_Send_buffers</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv</td>
 *   </tr><tr>
 *     <td>MPI_Scatterv</td>
 *     <td>root</td>
 *     <td>Must_Coll_Send_counts</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv</td>
 *   </tr><tr>
 *     <td>MPI_Allgather</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send_n</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv_buffers</td>
 *   </tr><tr>
 *     <td>MPI_Allgatherv</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send_n</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv_counts</td>
 *   </tr><tr>
 *     <td>MPI_Alltoall</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send_buffers</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv_buffers</td>
 *   </tr><tr>
 *     <td>MPI_Alltoallv</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send_counts</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv_counts</td>
 *   </tr><tr>
 *     <td>MPI_Alltoallw</td>
 *     <td>all</td>
 *     <td>Must_Coll_Send_types</td>
 *     <td>all</td>
 *     <td>Must_Coll_Recv_types</td>
 *   </tr><tr>
 *     <td>MPI_Allreduce</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Send_n</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Recv_n</td>
 *   </tr><tr>
 *     <td>MPI_Reduce_scatter</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Send_counts</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Recv_n</td>
 *   </tr><tr>
 *     <td>MPI_Reduce_scatter_block</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Send_buffers</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Recv_n</td>
 *   </tr><tr>
 *     <td>MPI_Scan</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Send_n</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Recv_n</td>
 *   </tr><tr>
 *     <td>MPI_Exscan</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Send_n</td>
 *     <td>all</td>
 *     <td>Must_Coll_Op_Recv_n</td>
 *   </tr>
 * </table>
 * 
 * 
 * 
 * 
 * 
 * @section RequestCondition RequestCondition
 * The Preconditioner for Requestcompletion is triggered by MPI_Test(|_all|_any|_some) and MPI_Wait(|_all|_any|_some) calls.
 * If any request is completed, an event is generated, that holds just the finished request(s).
 * <table border>
 *   <tr>
 *     <th>MPI-call</th>
 *     <th>conditionally</th>
 *     <th>generated event</th>
 *   </tr><tr>
 *     <td>MPI_Wait</td>
 *     <td>no</td>
 *     <td>propagateRequestRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Wait_some</td>
 *     <td>no</td>
 *     <td>propagateRequestRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Wait_any</td>
 *     <td>no</td>
 *     <td>propagateRequestsRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Wait_all</td>
 *     <td>no</td>
 *     <td>propagateRequestsRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Test</td>
 *     <td>yes</td>
 *     <td>propagateRequestRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Test_some</td>
 *     <td>yes</td>
 *     <td>propagateRequestRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Test_any</td>
 *     <td>yes</td>
 *     <td>propagateRequestsRealComplete</td>
 *   </tr><tr>
 *     <td>MPI_Test_all</td>
 *     <td>yes</td>
 *     <td>propagateRequestsRealComplete</td>
 *   </tr>
 * </table>
 * 
 */
