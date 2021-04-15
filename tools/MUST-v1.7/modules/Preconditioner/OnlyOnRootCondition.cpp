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
 * @file OnlyOnRootCondition.cpp
 *       @see MUST::OnlyOnRootCondition.
 *
 *  @date 23.08.2011
 *  @author Mathias Korepkat, Joachim Protze
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "OnlyOnRootConditionApi.h"
#include <iostream>
#include "OnlyOnRootCondition.h"

using namespace must;

mGET_INSTANCE_FUNCTION(OnlyOnRootCondition)
mFREE_INSTANCE_FUNCTION(OnlyOnRootCondition)
mPNMPI_REGISTRATIONPOINT_FUNCTION(OnlyOnRootCondition)


int OnlyOnRootCondition::pId2Rank(
        MustParallelId pId)
{
    return myPIdMod->getInfoForId(pId).rank;
}

//=============================
// Constructor
//=============================
OnlyOnRootCondition::OnlyOnRootCondition (const char* instanceName)
    : ModuleBase<OnlyOnRootCondition, I_OnlyOnRootCondition> (instanceName)
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
    getWrapperFunction ("MustOnRootTransfer", (GTI_Fct_t*)&myTransfer);
    getWrapperFunction ("MustOnRootTransferCounts", (GTI_Fct_t*)&myTransferCounts);
}

//=============================
// Destructor
//=============================
OnlyOnRootCondition::~OnlyOnRootCondition ()
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
GTI_ANALYSIS_RETURN OnlyOnRootCondition::gather (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType recvbuf,
        MustArgumentId recvbufArgId,
        int recvcount,
        MustArgumentId recvcountArgId,
        MustDatatypeType recvtype,
        MustArgumentId recvtypeArgId,
        int root,
        MustCommType comm
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int worldroot;
    commInfo->getGroup()->translate(root, &worldroot);


    if (pId2Rank(pId) == worldroot)
    {
        if (myTransfer)
        {
            (*myTransfer) (
                    pId,
                    lId,
                    0, /*recv transfer*/
                    recvbuf,
                    recvbufArgId,
                    recvcount,
                    recvcountArgId,
                    recvtype,
                    recvtypeArgId
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Gatherv
//=============================
GTI_ANALYSIS_RETURN OnlyOnRootCondition::gatherv (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType recvbuf,
        MustArgumentId recvbufArgId,
        const int recvcounts[],
        MustArgumentId recvcountsArgId,
        const int displs[],
        MustArgumentId displsArgId,
        MustDatatypeType recvtype,
        MustArgumentId recvtypeArgId,
        int root,
        MustCommType comm
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    commInfo->getGroup()->translate(root, &worldroot);

    if (pId2Rank(pId) == worldroot)
    {
        if (myTransferCounts)
        {
            (*myTransferCounts) (
                    pId,
                    lId,
                    0, /*True if send transfer, false otherwise*/
                    recvbuf,
                    recvbufArgId,
                    recvcounts,
                    recvcountsArgId,
                    displs,
                    displsArgId,
                    recvtype,
                    recvtypeArgId,
                    commsize
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handler for MPI_Scatter
//=============================
GTI_ANALYSIS_RETURN OnlyOnRootCondition::scatter (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustArgumentId sendbufArgId,
        int sendcount,
        MustArgumentId sendcountArgId,
        MustDatatypeType sendtype,
        MustArgumentId sendtypeArgId,
        int root,
        MustCommType comm
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int worldroot;
    commInfo->getGroup()->translate(root, &worldroot);

    if (pId2Rank(pId) == worldroot)
    {
        if (myTransfer)
        {

            (*myTransfer) (
                    pId,
                    lId,
                    1, /*send transfer*/
                    sendbuf,
                    sendbufArgId,
                    sendcount,
                    sendcountArgId,
                    sendtype,
                    sendtypeArgId
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}


//=============================
// handler for MPI_Scatterv
//=============================
GTI_ANALYSIS_RETURN OnlyOnRootCondition::scatterv (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        MustArgumentId sendbufArgId,
        const int sendcounts[],
        MustArgumentId sendcountsArgId,
        const int displs[],
        MustArgumentId displsArgId,
        MustDatatypeType sendtype,
        MustArgumentId sendtypeArgId,
        int root,
        MustCommType comm
        )
{
    I_Comm* commInfo = myComMod->getComm(pId, comm);
    // valid comm?
    if (commInfo == NULL || commInfo->isNull())
        return GTI_ANALYSIS_SUCCESS;
    int    commsize = commInfo->getGroup()->getSize(),
            worldroot;

    commInfo->getGroup()->translate(root, &worldroot);

    if (pId2Rank(pId) == worldroot)
    {
        if (myTransferCounts)
        {
            (*myTransferCounts) (
                    pId,
                    lId,
                    1, /*True if send transfer, false otherwise*/
                    sendbuf,
                    sendbufArgId,
                    sendcounts,
                    sendcountsArgId,
                    displs,
                    displsArgId,
                    sendtype,
                    sendtypeArgId,
                    commsize
                );
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}
/*EOF*/
