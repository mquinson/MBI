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
 * @file CollectiveCondition.cpp
 *       @see MUST::CollectiveCondition.
 *
 *  @date 06.06.2011
 *  @author Joachim Protze
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "CollectiveConditionApi.h"

#include "CollectiveCondition.h"

using namespace must;

mGET_INSTANCE_FUNCTION(CollectiveCondition)
mFREE_INSTANCE_FUNCTION(CollectiveCondition)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CollectiveCondition)


int CollectiveCondition::pId2Rank(
        MustParallelId pId)
{
    return myPIdMod->getInfoForId(pId).rank;
}

//=============================
// Constructor
//=============================
CollectiveCondition::CollectiveCondition (const char* instanceName)
    : ModuleBase<CollectiveCondition, I_CollectiveCondition> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODS 2
    if (subModInstances.size() < NUM_SUB_MODS)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_SUB_MODS)
    {
        for (std::vector<I_Module*>::size_type i = NUM_SUB_MODS; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myComMod = (I_CommTrack*) subModInstances[1];

    //Initialize module data
    getWrapperFunction ("Must_Coll_No_Transfer", (GTI_Fct_t*)&myPNoTransfer);
    getWrapperFunction ("Must_Coll_Send", (GTI_Fct_t*)&myPSend);
    getWrapperFunction ("Must_Coll_Op_Send", (GTI_Fct_t*)&myPOpSend);
    getWrapperFunction ("Must_Coll_Send_n", (GTI_Fct_t*)&myPSendN);
    getWrapperFunction ("Must_Coll_Op_Send_n", (GTI_Fct_t*)&myPOpSendN);
    getWrapperFunction ("Must_Coll_Send_buffers", (GTI_Fct_t*)&myPSendBuffers);
    getWrapperFunction ("Must_Coll_Op_Send_buffers", (GTI_Fct_t*)&myPOpSendBuffers);
    getWrapperFunction ("Must_Coll_Send_counts", (GTI_Fct_t*)&myPSendCounts);
    getWrapperFunction ("Must_Coll_Op_Send_counts", (GTI_Fct_t*)&myPOpSendCounts);
    getWrapperFunction ("Must_Coll_Send_types", (GTI_Fct_t*)&myPSendTypes);
    getWrapperFunction ("Must_Coll_Recv", (GTI_Fct_t*)&myPRecv);
    getWrapperFunction ("Must_Coll_Recv_n", (GTI_Fct_t*)&myPRecvN);
    getWrapperFunction ("Must_Coll_Op_Recv_n", (GTI_Fct_t*)&myPOpRecvN);
    getWrapperFunction ("Must_Coll_Recv_buffers", (GTI_Fct_t*)&myPRecvBuffers);
    getWrapperFunction ("Must_Coll_Recv_counts", (GTI_Fct_t*)&myPRecvCounts);
    getWrapperFunction ("Must_Coll_Recv_types", (GTI_Fct_t*)&myPRecvTypes);
    getWrapperFunction ("Must_Coll_Send_Recv", (GTI_Fct_t*)&myPSendRecv);
}

//=============================
// Destructor
//=============================
CollectiveCondition::~CollectiveCondition ()
{
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myComMod)
        destroySubModuleInstance ((I_Module*) myComMod);
    myComMod = NULL;
}

//=============================
// handler for MPI_Gather
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::gather (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType recvtype,
        int root,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    if (!commInfo->getGroup()->translate(root, &worldroot))
	    return GTI_ANALYSIS_SUCCESS;

    if (myPSend)
    {
        (*myPSend) (
                pId,
                lId,
                (int)MUST_COLL_GATHER,
                sendbuf,
                sendcount,
                sendtype,
                worldroot,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (pId2Rank(pId) == worldroot)
    {
        if (myPRecvBuffers)
        {
            (*myPRecvBuffers) (
                    pId,
                    lId,
                    (int)MUST_COLL_GATHER,
                    recvbuf,
                    recvcount,
                    recvtype,
                    commsize,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }

        if (myPSendRecv)
        {
            int rcount = recvcount * commsize;
            (*myPSendRecv) (
                    pId,
                    lId,
                    (int)MUST_COLL_GATHER,
                    sendbuf,
                    NULL,
                    0,
                    &sendcount,
                    1,
                    &sendtype,
                    1,
                    recvbuf,
                    NULL,
                    0,
                    &rcount,
                    1,
                    &recvtype,
                    1,
                    hasRequest,
                    request
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Gatherv
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::gatherv (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        const int recvcounts[],
        const int displs[],
        MustDatatypeType recvtype,
        int root,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull() )
			return GTI_ANALYSIS_SUCCESS;

    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    if (!commInfo->getGroup()->translate(root, &worldroot))
	    return GTI_ANALYSIS_SUCCESS;

    if (myPSend)
    {
        (*myPSend) (
                pId,
                lId,
                (int)MUST_COLL_GATHERV,
                sendbuf,
                sendcount,
                sendtype,
                worldroot,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (pId2Rank(pId) == worldroot)
    {
    		//skip event creation if integrity failures are present
    		if (recvcounts == NULL || displs == NULL)
	    		return GTI_ANALYSIS_SUCCESS;

        if (myPRecvCounts)
        {
            (*myPRecvCounts) (
                    pId,
                    lId,
                    (int)MUST_COLL_GATHERV,
                    recvbuf,
                    displs,
                    recvcounts,
                    recvtype,
                    commsize,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }
        if (myPSendRecv)
        {
            (*myPSendRecv) (
                    pId,
                    lId,
                    (int)MUST_COLL_GATHERV,
                    sendbuf,
                    NULL,
                    0,
                    &sendcount,
                    1,
                    &sendtype,
                    1,
                    recvbuf,
                    displs,
                    commsize,
                    recvcounts,
                    commsize,
                    &recvtype,
                    1,
                    hasRequest,
                    request
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Reduce
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::reduce (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustAddressType recvbuf,
        int count,
        MustDatatypeType datatype,
        MustOpType op,
        int root,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    if (!commInfo->getGroup()->translate(root, &worldroot))
	    return GTI_ANALYSIS_SUCCESS;

    if (myPOpSend)
    {
        (*myPOpSend) (
                pId,
                lId,
                (int)MUST_COLL_REDUCE,
                sendbuf,
                count,
                datatype,
                op,
                worldroot,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (pId2Rank(pId) == worldroot)
    {
        if (myPOpRecvN)
        {
            (*myPOpRecvN) (
                    pId,
                    lId,
                    (int)MUST_COLL_REDUCE,
                    recvbuf,
                    count,
                    datatype,
                    op,
                    commsize,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }
        if (myPSendRecv)
        {
            (*myPSendRecv) (
                    pId,
                    lId,
                    (int)MUST_COLL_REDUCE,
                    sendbuf,
                    NULL,
                    0,
                    &count,
                    1,
                    &datatype,
                    1,
                    recvbuf,
                    NULL,
                    0,
                    &count,
                    1,
                    &datatype,
                    1,
                    hasRequest,
                    request
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Bcast
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::bcast (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType buffer,
        int count,
        MustDatatypeType datatype,
        int root,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    if (!commInfo->getGroup()->translate(root, &worldroot))
	    return GTI_ANALYSIS_SUCCESS;

    if (pId2Rank(pId) == worldroot)
    {
        if (myPSendN)
        {
            (*myPSendN) (
                    pId,
                    lId,
                    (int)MUST_COLL_BCAST,
                    buffer,
                    count,
                    datatype,
                    commsize,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }
    }
    else
    {
        if (myPRecv)
        {
            (*myPRecv) (
                    pId,
                    lId,
                    (int)MUST_COLL_BCAST,
                    buffer,
                    count,
                    datatype,
                    worldroot,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }
    }


    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Scatter
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::scatter (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType recvtype,
        int root,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    if (!commInfo->getGroup()->translate(root, &worldroot))
	    return GTI_ANALYSIS_SUCCESS;

    if (pId2Rank(pId) == worldroot)
    {
        if (myPSendBuffers)
        {
            (*myPSendBuffers) (
                    pId,
                    lId,
                    (int)MUST_COLL_SCATTER,
                    sendbuf,
                    sendcount,
                    sendtype,
                    commsize,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }
        if (myPSendRecv)
        {
            int scount = sendcount * commsize;
            (*myPSendRecv) (
                    pId,
                    lId,
                    (int)MUST_COLL_SCATTER,
                    sendbuf,
                    NULL,
                    0,
                    &scount,
                    1,
                    &sendtype,
                    1,
                    recvbuf,
                    NULL,
                    0,
                    &recvcount,
                    1,
                    &recvtype,
                    1,
                    hasRequest,
                    request
                );
        }
    }

    if (myPRecv)
    {
        (*myPRecv) (
                pId,
                lId,
                (int)MUST_COLL_SCATTER,
                recvbuf,
                recvcount,
                recvtype,
                worldroot,
                comm,
                1,
                hasRequest,
                request
            );
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Scatterv
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::scatterv (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        const int sendcounts[],
        const int displs[],
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType recvtype,
        int root,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    if (!commInfo->getGroup()->translate(root, &worldroot))
	    return GTI_ANALYSIS_SUCCESS;

    if (pId2Rank(pId) == worldroot)
    {
        if (myPSendCounts)
        {
            (*myPSendCounts) (
                    pId,
                    lId,
                    (int)MUST_COLL_SCATTERV,
                    sendbuf,
                    displs,
                    sendcounts,
                    sendtype,
                    commsize,
                    comm,
                    1,
                    hasRequest,
                    request
                );
        }
        if (myPSendRecv)
        {
            (*myPSendRecv) (
                    pId,
                    lId,
                    (int)MUST_COLL_SCATTERV,
                    sendbuf,
                    displs,
                    commsize,
                    sendcounts,
                    commsize,
                    &sendtype,
                    1,
                    recvbuf,
                    NULL,
                    0,
                    &recvcount,
                    1,
                    &recvtype,
                    1,
                    hasRequest,
                    request
                );
        }
    }

    if (myPRecv)
    {
        (*myPRecv) (
                pId,
                lId,
                (int)MUST_COLL_SCATTERV,
                recvbuf,
                recvcount,
                recvtype,
                worldroot,
                comm,
                1,
                hasRequest,
                request
            );
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Allgather
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::allgather (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType recvtype,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize();

    if (myPSendN)
    {
        (*myPSendN) (
                pId,
                lId,
                (int)MUST_COLL_ALLGATHER,
                sendbuf,
                sendcount,
                sendtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPRecvBuffers)
    {
        (*myPRecvBuffers) (
                pId,
                lId,
                (int)MUST_COLL_ALLGATHER,
                recvbuf,
                recvcount,
                recvtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }
    if (myPSendRecv)
    {
        int rcount = recvcount * commsize;
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLGATHER,
                sendbuf,
                NULL,
                0,
                &sendcount,
                1,
                &sendtype,
                1,
                recvbuf,
                NULL,
                0,
                &rcount,
                1,
                &recvtype,
                1,
                hasRequest,
                request
            );
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Allgatherv
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::allgatherv (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        const int recvcounts[],
        const int displs[],
        MustDatatypeType recvtype,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize();

    if (myPSendN)
    {
        (*myPSendN) (
                pId,
                lId,
                (int)MUST_COLL_ALLGATHERV,
                sendbuf,
                sendcount,
                sendtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPRecvCounts)
    {
        (*myPRecvCounts) (
                pId,
                lId,
                (int)MUST_COLL_ALLGATHERV,
                recvbuf,
                displs,
                recvcounts,
                recvtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLGATHERV,
                sendbuf,
                NULL,
                0,
                &sendcount,
                1,
                &sendtype,
                1,
                recvbuf,
                displs,
                commsize,
                recvcounts,
                commsize,
                &recvtype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Alltoall
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::alltoall (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType recvtype,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize();

    if (myPSendBuffers)
    {
        (*myPSendBuffers) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALL,
                sendbuf,
                sendcount,
                sendtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPRecvBuffers)
    {
        (*myPRecvBuffers) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALL,
                recvbuf,
                recvcount,
                recvtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        int scount = sendcount * commsize, rcount = recvcount * commsize;
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALL,
                sendbuf,
                NULL,
                0,
                &scount,
                1,
                &sendtype,
                1,
                recvbuf,
                NULL,
                0,
                &rcount,
                1,
                &recvtype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Alltoallv
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::alltoallv (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        const int sendcounts[],
        const int sdispls[],
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        const int recvcounts[],
        const int rdispls[],
        MustDatatypeType recvtype,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize();

    if (myPSendCounts)
    {
        (*myPSendCounts) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALLV,
                sendbuf,
                sdispls,
                sendcounts,
                sendtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPRecvCounts)
    {
        (*myPRecvCounts) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALLV,
                recvbuf,
                rdispls,
                recvcounts,
                recvtype,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALLV,
                sendbuf,
                sdispls,
                commsize,
                sendcounts,
                commsize,
                &sendtype,
                1,
                recvbuf,
                rdispls,
                commsize,
                recvcounts,
                commsize,
                &recvtype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Alltoallw
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::alltoallw (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        const int sendcounts[],
        const int sdispls[],
        const MustDatatypeType sendtypes[],
        MustAddressType recvbuf,
        const int recvcounts[],
        const int rdispls[],
        const MustDatatypeType recvtypes[],
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize();

    if (myPSendTypes)
    {
        (*myPSendTypes) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALLW,
                sendbuf,
                sdispls,
                sendcounts,
                sendtypes,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPRecvTypes)
    {
        (*myPRecvTypes) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALLW,
                recvbuf,
                rdispls,
                recvcounts,
                recvtypes,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLTOALLW,
                sendbuf,
                sdispls,
                commsize,
                sendcounts,
                commsize,
                sendtypes,
                commsize,
                recvbuf,
                rdispls,
                commsize,
                recvcounts,
                commsize,
                recvtypes,
                commsize,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Allreduce
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::allreduce (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustAddressType recvbuf,
        int count,
        MustDatatypeType datatype,
        MustOpType op,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;

    int    commsize = commInfo->getGroup()->getSize();

    if (myPOpSendN)
    {
        (*myPOpSendN) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                sendbuf,
                count,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPOpRecvN)
    {
        (*myPOpRecvN) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                recvbuf,
                count,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                sendbuf,
                NULL,
                0,
                &count,
                1,
                &datatype,
                1,
                recvbuf,
                NULL,
                0,
                &count,
                1,
                &datatype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Reduce_scatter
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::reduce_scatter (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustAddressType recvbuf,
        const int recvcounts[],
        MustDatatypeType datatype,
        MustOpType op,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;

    int    commsize = commInfo->getGroup()->getSize();
    int    localCommRank;

    commInfo->getGroup()->containsWorldRank(pId2Rank(pId), &localCommRank);

    if (myPOpSendCounts)
    {
        (*myPOpSendCounts) (
                pId,
                lId,
                (int)MUST_COLL_REDUCE_SCATTER,
                sendbuf,
                recvcounts,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPOpRecvN)
    {
        (*myPOpRecvN) (
                pId,
                lId,
                (int)MUST_COLL_REDUCE_SCATTER,
                recvbuf,
                recvcounts[localCommRank],
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        int scount = 0;
        for (int i=0; i<commsize; i++)
            scount += recvcounts[i];
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                sendbuf,
                NULL,
                0,
                &scount,
                1,
                &datatype,
                1,
                recvbuf,
                NULL,
                0,
                &(recvcounts[localCommRank]),
                1,
                &datatype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Reduce_scatter_block
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::reduce_scatter_block (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType datatype,
        MustOpType op,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;

    int    commsize = commInfo->getGroup()->getSize();

    if (myPOpSendBuffers)
    {
        (*myPOpSendBuffers) (
                pId,
                lId,
                (int)MUST_COLL_REDUCE_SCATTER_BLOCK,
                sendbuf,
                recvcount,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPOpRecvN)
    {
        (*myPOpRecvN) (
                pId,
                lId,
                (int)MUST_COLL_REDUCE_SCATTER_BLOCK,
                recvbuf,
                recvcount,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        int scount = recvcount * commsize;
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                sendbuf,
                NULL,
                0,
                &scount,
                1,
                &datatype,
                1,
                recvbuf,
                NULL,
                0,
                &recvcount,
                1,
                &datatype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Scan
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::scan (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustAddressType recvbuf,
        int count,
        MustDatatypeType datatype,
        MustOpType op,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;

    int    commsize = commInfo->getGroup()->getSize();

    if (myPOpSendN)
    {
        (*myPOpSendN) (
                pId,
                lId,
                (int)MUST_COLL_SCAN,
                sendbuf,
                count,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPOpRecvN)
    {
        (*myPOpRecvN) (
                pId,
                lId,
                (int)MUST_COLL_SCAN,
                recvbuf,
                count,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                sendbuf,
                NULL,
                0,
                &count,
                1,
                &datatype,
                1,
                recvbuf,
                NULL,
                0,
                &count,
                1,
                &datatype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Exscan
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::exscan (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustAddressType recvbuf,
        int count,
        MustDatatypeType datatype,
        MustOpType op,
        MustCommType comm,
        int hasRequest,
        MustRequestType request
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;

    int    commsize = commInfo->getGroup()->getSize();

    if (myPOpSendN)
    {
        (*myPOpSendN) (
                pId,
                lId,
                (int)MUST_COLL_EXSCAN,
                sendbuf,
                count,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPOpRecvN)
    {
        (*myPOpRecvN) (
                pId,
                lId,
                (int)MUST_COLL_EXSCAN,
                recvbuf,
                count,
                datatype,
                op,
                commsize,
                comm,
                1,
                hasRequest,
                request
            );
    }

    if (myPSendRecv)
    {
        (*myPSendRecv) (
                pId,
                lId,
                (int)MUST_COLL_ALLREDUCE,
                sendbuf,
                NULL,
                0,
                &count,
                1,
                &datatype,
                1,
                recvbuf,
                NULL,
                0,
                &count,
                1,
                &datatype,
                1,
                hasRequest,
                request
            );
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// noTransfer
//=============================
GTI_ANALYSIS_RETURN CollectiveCondition::noTransfer (
                    MustParallelId pId,
                    MustLocationId lId,
                    int coll, // formerly gti::MustCollCommType
                    MustCommType comm,
                    int hasRequest,
                    MustRequestType request
            )
{
    if (myPNoTransfer)
    {
        (*myPNoTransfer) (
                pId,
                lId,
                coll,
                comm,
                1,
                hasRequest,
                request
        );
    }
    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
