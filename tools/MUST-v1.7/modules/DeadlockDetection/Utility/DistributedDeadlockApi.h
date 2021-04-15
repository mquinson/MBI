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
 * @file DistributedDeadlockApi.h
 * 		P call definition for MUST distributed deadlock detection API calls.
 *
 * @author Tobias Hilbrich
 * @date 20.02.2012
 */

#ifndef DISTRIBUTED_DEADLOCK_API_H
#define DISTRIBUTED_DEADLOCK_API_H

#include "I_RequestTrack.h"
#include "I_CommTrack.h"
#include "MustTypes.h"

//== Function to pass a send to another DP2PMatch instance on the same layer
inline int PpassSendForMatching (
        MustParallelId pId,
        MustLocationId lId,
        int dest,
        int tag,
        MustCommType comm,
        MustDatatypeType type,
        int count,
        int mode,
        MustLTimeStamp lTS)  {return 0;}

typedef int (*passSendForMatchingP) (
		MustParallelId pId,
		MustLocationId lId,
		int dest,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
		int mode,
		MustLTimeStamp lTS,
		int toLevel /*IMPLICIT argument*/);

//== Function to pass an isend to another DP2PMatch instance on the same layer
inline int PpassIsendForMatching (
        MustParallelId pId,
        MustLocationId lId,
        int dest,
        int tag,
        MustCommType comm,
        MustDatatypeType type,
        int count,
        int mode,
        MustRequestType request,
        MustLTimeStamp lTS)  {return 0;}

typedef int (*passIsendForMatchingP) (
        MustParallelId pId,
        MustLocationId lId,
        int dest,
        int tag,
        MustCommType comm,
        MustDatatypeType type,
        int count,
        int mode,
        MustRequestType request,
        MustLTimeStamp lTS,
        int toLevel /*IMPLICIT argument*/);

//== Function to pass a start of a persistent send-request to another DP2PMatch instance on the same layer
inline int PpassSendStartForMatching (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request,
        MustLTimeStamp lTS)  {return 0;}

typedef int (*passSendStartForMatchingP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request,
        MustLTimeStamp lTS,
        int toLevel /*IMPLICIT argument*/);

//== Function to tell collective comm reductions whether any ancestor is tacking care of
//      checking complex MPI collective type matching with intra-layer communication
inline int PdCollMatchAncestorHasIntra (
        int ancestorHas)  {return 0;}

typedef int (*dCollMatchAncestorHasIntraP) (
        int ancestorHas);

//== Function to pass a collective operations type match info to another DCollectiveMatch instance on the same layer
inline int PpassTypeMatchInfo (
        MustParallelId pId,
        MustLocationId lId,
        MustRemoteIdType commRId,
        MustRemoteIdType typeRId,
        int numCounts,
        int* counts, /*-1 for unused values, e.g. if the comm of this op is only a subgroup of MPI_COMM_WORLD*/
        int firstRank, /*Rank in comm world*/
        int collectiveNumber,
        int collId)  {return 0;}

typedef int (*passTypeMatchInfoP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRemoteIdType commRId,
        MustRemoteIdType typeRId,
        int numCounts,
        int* counts, /*-1 for unused values, e.g. if the comm of this op is only a subgroup of MPI_COMM_WORLD*/
        int firstRank, /*Rank in comm world*/
        int collectiveNumber,
        int collId,
        int toLevel /*IMPLICIT argument*/);

//== Function to pass a collective operations type match info to another DCollectiveMatch instance on the same layer.
//      Same as passTypeMatchInfo, but with multiple types (for MPI_Alltoallw).
inline int PpassTypeMatchInfoTypes (
        MustParallelId pId,
        MustLocationId lId,
        MustRemoteIdType commRId,
        int numCountsAndTypes,
        MustRemoteIdType* typeRIds,
        int* counts, /*-1 for unused values, e.g. if the comm of this op is only a subgroup of MPI_COMM_WORLD*/
        int firstRank, /*Rank in comm world*/
        int collectiveNumber,
        int collId)  {return 0;}

typedef int (*passTypeMatchInfoTypesP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRemoteIdType commRId,
        int numCountsAndTypes,
        MustRemoteIdType* typeRIds,
        int* counts, /*-1 for unused values, e.g. if the comm of this op is only a subgroup of MPI_COMM_WORLD*/
        int firstRank, /*Rank in comm world*/
        int collectiveNumber,
        int collId,
        int toLevel /*IMPLICIT argument*/);

//== Function to send a CollectiveActive request towards the root
/**
 * @see PgenerateCollectiveActiveAcknowledge
 */
inline int PgenerateCollectiveActiveRequest (
        int isIntercomm,
        unsigned long long contextId,
        int collCommType,
        int localGroupSize,
        int remoteGroupSize,
        int numTasks // counter for event aggregation
    )  {return 0;}

typedef int (*generateCollectiveActiveRequestP) (
        int isIntercomm,
        unsigned long long contextId,
        int collCommType,
        int localGroupSize,
        int remoteGroupSize,
        int numTasks // counter for event aggregation
    );

//== Function to send a CollectiveActiveAcknowledge back to the DWaitState nodes
/**
 * @todo this API is rather crude, background is we can't send a communicator
 *            handle down, since it may differ on each task. As a result,
 *            we would need to make a per ancestor node specific send, rather than
 *            a broadcast. We use the arguments that the I_Comm implementation
 *            uses to compare a communicator, which is equal on all ranks and nodes.
 *            However, if we change the I_Comm implementation we need to adapt this
 *            API too.
 */
inline int PgenerateCollectiveActiveAcknowledge (
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    )  {return 0;}

typedef int (*generateCollectiveActiveAcknowledgeP) (
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    );

//== Function to send a ReceiveActiveRequest via intra-layer (DWaitState)
inline int PgenerateReceiveActiveRequest (
        int sendRank,
        MustLTimeStamp sendLTS,
        MustLTimeStamp receiveLTS
    )  {return 0;}

typedef int (*generateReceiveActiveRequestP) (
        int sendRank,
        MustLTimeStamp sendLTS,
        MustLTimeStamp receiveLTS,
        int toLevel /*IMPLICIT argument*/
    );

//== Function to send a ReceiveActiveAcknowledge via intra-layer (DWaitState)
inline int PgenerateReceiveActiveAcknowledge (
        int receiveRank,
        MustLTimeStamp receiveLTS
    )  {return 0;}

typedef int (*generateReceiveActiveAcknowledgeP) (
        int receiveRank,
        MustLTimeStamp receiveLTS,
        int toLevel /*IMPLICIT argument*/
    );

//== Broadcasted by DWaitStateWfgMgr to request that all DWaitState modules shall generate a consistent state
inline int PrequestConsistentState (void)  {return 0;}
typedef int (*requestConsistentStateP) (void);

//== Sent upwards by DWaitState when a consistent state is ready
inline int PacknowledgeConsistentState (int numHeads)  {return 0;}
typedef int (*acknowledgeConsistentStateP) (int numHeads);

//== Ping-pongs for DWaitState intra communication
inline int PpingDWaitState (
        int fromNode, //Issuer node id
        int pingsRemaining //number of ping-pongs to be done when the pong for this ping arrives
        )  {return 0;}
typedef int (*pingDWaitStateP) (
        int fromNode,
        int pingsRemaining,
        int toLevel /*IMPLICIT argument*/);

inline int PpongDWaitState (
        int fromNode, //Issuer node id
        int pingsRemaining //number of ping-pongs to be done when the pong for this ping arrives
        )  {return 0;}
typedef int (*pongDWaitStateP) (
        int fromNode,
        int pingsRemaining,
        int toLevel /*IMPLICIT argument*/);

//== Function to request wait-for infos from all DWaitState implementations
inline int PrequestWaitForInfos (void)  {return 0;}
typedef int (*requestWaitForInfosP) (void);

//== Function acknowledge requestWaitForInfos for an empty head
inline int PprovideWaitForInfosEmpty (
        int worldRank
    )  {return 0;}

typedef int (*provideWaitForInfosEmptyP) (
        int worldRank
    );

//== Function to pass wait-for information of a rank or a ranks sub node to the root
inline int PprovideWaitForInfosSingle (
        int worldRank, /*originating MPI_COMM_WORLD rank*/
        MustParallelId pId, /* pId to name the node*/
        MustLocationId lId,
        int subId, /*sub id (if given world rank used provideWaitForInfosMixed), or  -1 if this is for the rank directly*/
        int count, /*number of wait-for dependencies*/
        int type, /*must::ArcType (AND or OR)*/
        int* toRanks, /*count sized list of MPI_COMM_WORLD target ranks*/
        MustParallelId *labelPIds, /*optional pId+lId pair for each wait-for, size of count; use 0+0 for NO pair*/
        MustLocationId *labelLId,
        int labelsSize, /*string length of all concatenated labels */
        char *labels /*all labels concatenated and separated with a '\n'*/
    )  {return 0;}

typedef int (*provideWaitForInfosSingleP) (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int subId,
        int count,
        int type,
        int* toRanks,
        MustParallelId *labelPIds,
        MustLocationId *labelLId,
        int labelsSize,
        char *labels
    );

//== Function to pass wait-for information of a rank with sub nodes to the root (Main node for the rank uses this information, sub-nodes are specified with provideWaitForInfosSingle)
inline int PprovideWaitForInfosMixed (
        int worldRank, /*originating MPI_COMM_WORLD rank*/
        MustParallelId pId, /* pId to name the node*/
        MustLocationId lId,
        int numSubs, /*number of subIds*/
        int type, /*must::ArcType (AND or OR)*/
        MustParallelId *labelPIds, /*optional pId+lId pair for each wait-for, size of numSubs; use 0+0 for NO pair*/
        MustLocationId *labelLId,
        int labelsSize, /*string length of all concatenated labels */
        char *labels /*all labels concatenated and separated with a '\n'*/
        ) {return 0;}

typedef int (*provideWaitForInfosMixedP) (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int numSubs,
        int type,
        MustParallelId *labelPIds,
        MustLocationId *labelLId,
        int labelsSize,
        char *labels
    );

//== Function to pass wait-for information of a rank that currently is in a collective
inline int PprovideWaitForInfosColl (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int collType,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    )  {return 0;}

typedef int (*provideWaitForInfosCollP) (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int collType,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    );

//== Function to pass background information on the matching information of an non blocking collective
inline int PprovideWaitForNbcBackground (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int waveNumInColl,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    )  {return 0;}

typedef int (*provideWaitForNbcBackgroundP) (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int waveNumInColl,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    );

//== Function to pass wait-for information of a rank that currently is in a non-blocking collective
inline int PprovideWaitForInfosNbcColl (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int subId, /*sub id (if given world rank used provideWaitForInfosMixed), or  -1 if this is for the rank directly*/
        int waveNumInColl,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    )  {return 0;}

typedef int (*provideWaitForInfosNbcCollP) (
        int worldRank,
        MustParallelId pId,
        MustLocationId lId,
        int subId,
        int waveNumInColl,
        int isIntercomm,
        unsigned long long contextId,
        int localGroupSize,
        int remoteGroupSize
    );


#endif /*DISTRIBUTED_DEADLOCK_API_H*/
