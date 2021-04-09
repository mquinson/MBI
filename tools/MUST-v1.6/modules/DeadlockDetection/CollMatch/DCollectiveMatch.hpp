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
 * @file DCollectiveMatch.hpp
 *       @see MUST::DCollectiveMatch.
 *
 *  @date 24.04.2012
 *  @author Fabian Haensel, Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

using namespace gti;

#include "DCollectiveTypeMatchInfo.h"
#include "GtiApi.h"

#include <vector>
#include <fstream>

namespace must
{
//=============================
// DCollectiveMatch
//=============================
template <class INSTANCE, class BASE>
DCollectiveMatch<INSTANCE, BASE>::DCollectiveMatch (const char* instanceName, bool isReduction)
 : ModuleBase<INSTANCE, BASE> (instanceName),
   myComms (),
   myIsReduction (isReduction),
   myIsActive (true),
   myDoIntraLayerChecking (false),
   myAncestorDoesIntraChecking (false),
   myHasIntraComm (false),
   myReusableCountArray (NULL),
   myWorldSize (-1),
   myPIdMod (NULL),
   myCommTrack (NULL),
   myTypeTrack (NULL),
   myOpTrack (NULL),
   myLogger (NULL),
   myConsts (NULL),
   myHadNewOp (false),
   myListener (NULL),
   myCommListeners ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = ModuleBase<INSTANCE, BASE>::createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 7
    if (subModInstances.size() < NUM_MODS_REQUIRED)
    {
        std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
        assert (0);
    }
    if (subModInstances.size() > NUM_MODS_REQUIRED)
    {
        for (std::vector<I_Module*>::size_type i = NUM_MODS_REQUIRED; i < subModInstances.size(); i++)
            ModuleBase<INSTANCE, BASE>::destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLIdMod = (I_LocationAnalysis*) subModInstances[1];
    myConsts = (I_BaseConstants*) subModInstances[2];
    myLogger = (I_CreateMessage*) subModInstances[3];
    myCommTrack = (I_CommTrack*) subModInstances[4];
    myTypeTrack = (I_DatatypeTrack*) subModInstances[5];
    myOpTrack = (I_OpTrack*) subModInstances[6];

    //Module data
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_No_Transfer", (GTI_Fct_t*)&myCollNoTransferFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Send", (GTI_Fct_t*)&mySendFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Op_Send", (GTI_Fct_t*)&myOpSendFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Send_n", (GTI_Fct_t*)&mySendNFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Op_Send_n", (GTI_Fct_t*)&myOpSendNFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Send_buffers", (GTI_Fct_t*)&mySendBuffersFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Op_Send_buffers", (GTI_Fct_t*)&myOpSendBuffersFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Send_counts", (GTI_Fct_t*)&mySendCountsFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Op_Send_counts", (GTI_Fct_t*)&myOpSendCountsFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Send_types", (GTI_Fct_t*)&mySendTypesFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Recv", (GTI_Fct_t*)&myRecvFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Recv_n", (GTI_Fct_t*)&myRecvNFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Op_Recv_n", (GTI_Fct_t*)&myOpRecvNFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Recv_buffers", (GTI_Fct_t*)&myRecvBuffersFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Recv_counts", (GTI_Fct_t*)&myRecvCountsFct);
    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("Must_Coll_Recv_types", (GTI_Fct_t*)&myRecvTypesFct);

    ModuleBase<INSTANCE, BASE>::getWrapperFunction ("dCollMatchAncestorHasIntra", (GTI_Fct_t*)&myAncestorHasIntraFct);

    ModuleBase<INSTANCE, BASE>::getWrapAcrossFunction("passTypeMatchInfo", (GTI_Fct_t*)&myPassTypeMatchInfoFct);
    ModuleBase<INSTANCE, BASE>::getWrapAcrossFunction("passTypeMatchInfoTypes", (GTI_Fct_t*)&myPassTypeMatchInfoTypesFct);

    ModuleBase<INSTANCE, BASE>::getSetNextEventStridedFunction((GTI_Fct_t*)&myNextEventStridedFct);

    if (myPassTypeMatchInfoFct && myPassTypeMatchInfoTypesFct)
        myHasIntraComm = true;
}

//=============================
// ~DCollectiveMatch
//=============================
template <class INSTANCE, class BASE>
DCollectiveMatch<INSTANCE, BASE>::~DCollectiveMatch()
{
    //Stop propagation of any resources
    if (myCommTrack)
        myCommTrack->notifyOfShutdown();

    if (myTypeTrack)
        myTypeTrack->notifyOfShutdown();

    if (myOpTrack)
        myOpTrack->notifyOfShutdown();

    //Free module data
    std::set<DCollectiveCommInfo*>::iterator i;
#ifdef DCOLL_DEBUG
    bool nonEmptyWarned = false;
#endif /*DCOLL_DEBUG*/
    for (i = myComms.begin(); i != myComms.end(); i++)
    {
        if (*i)
        {
#ifdef DCOLL_DEBUG
            if ((*i)->hasUncompletedWaves())
            {
                nonEmptyWarned = true;
                std::cout << "Warning: DCollectiveMatch still has open waves!" << std::endl;
            }
#endif /*DCOLL_DEBUG*/
            delete (*i);
        }
    }
    myComms.clear ();

    if (myReusableCountArray)
        delete [] myReusableCountArray;
    myReusableCountArray = NULL;

    //Free sub modules
    if (myPIdMod)
        ModuleBase<INSTANCE, BASE>::destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myConsts)
        ModuleBase<INSTANCE, BASE>::destroySubModuleInstance ((I_Module*) myConsts);
    myConsts = NULL;

    if (myLogger)
        ModuleBase<INSTANCE, BASE>::destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myCommTrack)
        ModuleBase<INSTANCE, BASE>::destroySubModuleInstance ((I_Module*) myCommTrack);
    myCommTrack = NULL;

    if (myTypeTrack)
        ModuleBase<INSTANCE, BASE>::destroySubModuleInstance ((I_Module*) myTypeTrack);
    myTypeTrack = NULL;

    if (myOpTrack)
        ModuleBase<INSTANCE, BASE>::destroySubModuleInstance ((I_Module*) myOpTrack);
    myOpTrack = NULL;
}

//=============================
// CollNoTransfer
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollNoTransfer (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS; //we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
        return getErrorReturn();

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollSend
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollSend (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int dest, /*Root process to send to as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return getErrorReturn();
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return getErrorReturn();
        }
    }

    //Check whether given root is reasonable
    if (dest < 0)
    {
        commInfo->erase();
        typeInfo->erase();
        if (opInfo)
            opInfo->erase();
        return getErrorReturn();
    }

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            true,
            count,
            typeInfo,
            type,
            opInfo,
            op,
            dest,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollSendN
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollSendN (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return getErrorReturn();
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return getErrorReturn();
        }
    }

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            true,
            count,
            typeInfo,
            type,
            opInfo,
            op,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollSendCounts
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollSendCounts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return getErrorReturn();
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return getErrorReturn();
        }
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            true,
            ownCounts,
            typeInfo,
            type,
            opInfo,
            op,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollSendTypes
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollSendTypes (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent** typeInfos = new I_DatatypePersistent* [commsize];
    for (int i = 0; i < commsize; i++)
    {
        if (!getTypeInfo(pId, types[i], &(typeInfos[i])))
        {
            for (int j = 0; j < i; j++)
                typeInfos[j]->erase();
            commInfo->erase();
            return getErrorReturn();
        }
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    MustDatatypeType *ownTypes = new MustDatatypeType [commsize];
    for (int i = 0; i < commsize; i++)
        ownTypes[i] = types[i];

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            true,
            ownCounts,
            typeInfos,
            ownTypes,
            NULL,
            0,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollRecv
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollRecv (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int src, /*Root process to receive from as a rank in MPI_COMM_WORLD*/
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return getErrorReturn();
    }

    //Check whether given root is reasonable
    if (src < 0)
    {
        commInfo->erase();
        typeInfo->erase();
        return getErrorReturn();
    }

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            false,
            count,
            typeInfo,
            type,
            NULL,
            0,
            src,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollRecvN
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollRecvN (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        int count,
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int hasOp,
        MustOpType op,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return getErrorReturn();
    }

    I_OpPersistent* opInfo = NULL;
    if (hasOp)
    {
        if (!getOpInfo (pId, op, &opInfo))
        {
            commInfo->erase();
            typeInfo->erase();
            return getErrorReturn();
        }
    }

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            false,
            count,
            typeInfo,
            type,
            opInfo,
            op,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollRecvCounts
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollRecvCounts (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        MustDatatypeType type,
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent* typeInfo;
    if (!getTypeInfo(pId, type, &typeInfo))
    {
        commInfo->erase();
        return getErrorReturn();
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            false,
            ownCounts,
            typeInfo,
            type,
            NULL,
            0,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// CollRecvTypes
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::CollRecvTypes (
        MustParallelId pId,
        MustLocationId lId,
        int coll, // formerly MustCollCommType
        const int counts[],
        const MustDatatypeType types[],
        int commsize,
        MustCommType comm,
        int numTasks,
        int hasRequest,
        MustRequestType request,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels
)
{
    if (!myIsActive)
        return GTI_ANALYSIS_SUCCESS;//we kill events here if we are a reduction, which is fine

    if (!myIsReduction)
        return GTI_ANALYSIS_SUCCESS;//Root actually does not adds any extra functionality (currently)

    //== 1) Prepare the operation
    //Get infos
    I_CommPersistent* commInfo;
    if (!getCommInfo (pId, comm, &commInfo))
    {
        return getErrorReturn();
    }

    I_DatatypePersistent** typeInfos = new I_DatatypePersistent* [commsize];
    for (int i = 0; i < commsize; i++)
    {
        if (!getTypeInfo(pId, types[i], &(typeInfos[i])))
        {
            for (int j = 0; j < i; j++)
                typeInfos[j]->erase();
            commInfo->erase();
            return getErrorReturn();
        }
    }

    //Copy arrays
    int *ownCounts = new int [commsize];
    for (int i = 0; i < commsize; i++)
        ownCounts[i] = counts[i];

    MustDatatypeType *ownTypes = new MustDatatypeType [commsize];
    for (int i = 0; i < commsize; i++)
        ownTypes[i] = types[i];

    int fromChannel = -1;
    if (cId)
        fromChannel = cId->getSubId(cId->getNumUsedSubIds()-1);
    if (cId->getNumUsedSubIds() == 1)
        fromChannel = -1; //Special rule, as reduction does not runs on application directly we must not avoid any checks on the first TBON layer!

    //Create op
    DCollectiveOp* newOp = new DCollectiveOp (
            this,
            pId,
            lId,
            (MustCollCommType) coll,
            commInfo,
            comm,
            false,
            ownCounts,
            typeInfos,
            ownTypes,
            NULL,
            0,
            numTasks,
            fromChannel,
            hasRequest,
            request);

    //== 2) Process or Queue ?
    int rank = myPIdMod->getInfoForId(pId).rank;
    return handleNewOp (rank, cId, outFinishedChannels, newOp);
}

//=============================
// getCommInfo
//=============================
template <class INSTANCE, class BASE>
bool DCollectiveMatch<INSTANCE, BASE>::getCommInfo (
                MustParallelId pId,
                MustCommType comm,
                I_CommPersistent **outComm)
{
    I_CommPersistent *commInfo = myCommTrack->getPersistentComm(pId, comm);

    //Handle unknown
    if (!commInfo) return false;

    //Handle MPI_COMM_NULL
    if (commInfo->isNull())
    {
        commInfo->erase();
        return false;
    }

    if (outComm) *outComm = commInfo;

    return true;
}

//=============================
// getTypeInfo
//=============================
template <class INSTANCE, class BASE>
bool DCollectiveMatch<INSTANCE, BASE>::getTypeInfo (
                MustParallelId pId,
                MustDatatypeType type,
                I_DatatypePersistent **outType)
{
    I_DatatypePersistent *typeInfo = myTypeTrack->getPersistentDatatype (pId, type);

    if (!typeInfo) return false;

    if (typeInfo->isNull())
    {
        typeInfo->erase();
        return false;
    }

    if (outType) *outType = typeInfo;
    return true;
}

//=============================
// getOpInfo
//=============================
template <class INSTANCE, class BASE>
bool DCollectiveMatch<INSTANCE, BASE>::getOpInfo (
                MustParallelId pId,
                MustOpType op,
                I_OpPersistent** outOp)
{
    I_OpPersistent *opInfo = myOpTrack->getPersistentOp(pId, op);

    if (!opInfo) return false;

    if (opInfo->isNull())
    {
        opInfo->erase();
        return false;
    }

    if (outOp) *outOp = opInfo;
    return true;
}

//=============================
// handleNewOp
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::handleNewOp (
        int rank,
        I_ChannelId* cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels,
        DCollectiveOp* newOp)
{
    DCollectiveCommInfo* commInfo = NULL;
    myHadNewOp = true;
    /*static int numPotentialCommConflicts = 0;
    const int conflictThreshold = 1;
    static std::map<int, DCollectiveCommInfo*> conflictComms;
    static bool done = false;*/

    //Init world size if necessary
    if (myWorldSize <= 0)
    {
        myWorldSize = myCommTrack->getComm(newOp->getPId(), myCommTrack->getWorldHandle())->getGroup()->getSize();
    }

    //Search for the comm info for this op
    std::set<DCollectiveCommInfo*>::iterator iter;

    for (iter = myComms.begin(); iter != myComms.end(); iter++)
    {
        if (!commInfo && (*iter)->getComm()->compareComms(newOp->getComm()))
        {
            commInfo = *iter;
            break;
        }

        /**
         * @todo
         * Sept 2012(?): Extension to reduce number of unsucessful aggregations, had negative impact on BGQ, so disabled it; To enable it exchange the break above with a continue.
         *
         * Feb 20, 2013: I Extended the initial heuristic a bit to completely disable aggregations for some communicators (rather than disabling based on an active wave),
         * the assumption was that if we time out other events based on a wave being active, we may get large successive numbers of events from the same process and thus
         * may create a large number of timed out waves as a result. I.e. the ratio between active and timed out waves will largely shift towards timed out waves.
         *
         * The idea with disabling whole communicators did not even work too nicely for collDisjointCommNoError, where we actually only have 3 communicators.
         * Looking at NPB FT, where communicators for each row and column exist, the situation is much more challenging, there exis sqrt(p) comb shaped communicators.
         * Currently a good heuristic solution does not really come to my mind, it appears a modification to the channel-id's to incorporate additional information there might be
         * the most reasonable choice.
         *
         * As of Feb 20, 2013, I will focus on reducing the performance impact of failing aggregations since t/op increases towards the root.
         */
        //If this is not the communicator info, do we already have an active wave there?
        /*if (!hasActiveWave && (*iter)->hasActiveWave() && !done)
        {
            hasActiveWave = true;
            conflictComms[conflictComms.size()] = *iter;
        }*/
    }

    //Do we need a new Info?
    if (!commInfo)
    {
        commInfo = new DCollectiveCommInfo (newOp->getCommCopy());
        myComms.insert(commInfo);

        //Notify our comm listener of the new comm in use by a collective
        std::list<I_CollCommListener*>::iterator lIter;
        I_CommPersistent* tempComm = newOp->getCommCopy();

        for (lIter = myCommListeners.begin(); lIter != myCommListeners.end(); lIter++)
            (*lIter)->newCommInColl(newOp->getPId(), tempComm);

        tempComm->erase();
    }

    /*
    //Increase number of potential communicator conflicts
    if (hasActiveWave && !done)
    {
        numPotentialCommConflicts++;
        conflictComms[conflictComms.size()] = commInfo;
    }
    */

    /*
     * If we had too many conflicts, we disable all but one communicator for aggregation
     * We use our level id to select the remaining communicator, such that other levels
     * hopefully select different communicators
     * (@todo Heurisitc solution might be far from optimal)
     */
    /*if (numPotentialCommConflicts == conflictThreshold && !done)
    {
        int toRemain;
        ModuleBase<INSTANCE, BASE>::getLevelId (&toRemain);

        toRemain = toRemain % conflictComms.size();

        for (int i = 0; i < conflictComms.size(); i++)
        {
            if (i == toRemain) continue;
                conflictComms[i]->disableAggregation();
            //std::cout << getpid() << " disabled " << i << std::endl;
        }

        done=true;
    }*/

    //Apply
    GTI_ANALYSIS_RETURN ret = commInfo->addNewOp (myListener, cId, outFinishedChannels, newOp, myDoIntraLayerChecking, myAncestorDoesIntraChecking, false);

#ifdef DCOLL_DEBUG
    printAsDot ();
#endif /*DCOLL_DEBUG*/


    //If there was an error we stop processing
    if (ret == GTI_ANALYSIS_FAILURE)
    {
        myIsActive = false;
        ret = GTI_ANALYSIS_IRREDUCIBLE;
    }

    //If we are the reduction we pass the actual return value
    if (myIsReduction)
        return ret;

    //If we are the root, we must remove the reduction specific returns
    if (ret == GTI_ANALYSIS_IRREDUCIBLE || ret == GTI_ANALYSIS_WAITING)
        return GTI_ANALYSIS_SUCCESS;
    return ret;
}

//=============================
// pIdToRank
//=============================
template <class INSTANCE, class BASE>
int DCollectiveMatch<INSTANCE, BASE>::pIdToRank (MustParallelId pId)
{
    return myPIdMod->getInfoForId(pId).rank;
}

//=============================
// getLevelIdForApplicationRank
//=============================
template <class INSTANCE, class BASE>
int DCollectiveMatch<INSTANCE, BASE>::getLevelIdForApplicationRank (int rank)
{
    int ret;
    if (GTI_SUCCESS== ModuleBase<INSTANCE, BASE>::getLevelIdForApplicationRank (rank, &ret))
        return ret;
    return -1;
}

//=============================
// getLogger
//=============================
template <class INSTANCE, class BASE>
I_CreateMessage* DCollectiveMatch<INSTANCE, BASE>::getLogger (void)
{
    return myLogger;
}

//=============================
// getDatatypeTrack
//=============================
template <class INSTANCE, class BASE>
I_DatatypeTrack* DCollectiveMatch<INSTANCE, BASE>::getDatatypeTrack (void)
{
    return myTypeTrack;
}

//=============================
// getCommTrack
//=============================
template <class INSTANCE, class BASE>
I_CommTrack* DCollectiveMatch<INSTANCE, BASE>::getCommTrack (void)
{
    return myCommTrack;
}

//=============================
// getLocationModule
//=============================
template <class INSTANCE, class BASE>
I_LocationAnalysis* DCollectiveMatch<INSTANCE, BASE>::getLocationModule (void)
{
    return myLIdMod;
}

//=============================
// getErrorReturn
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::getErrorReturn (void)
{
    if (myIsReduction)
        return GTI_ANALYSIS_IRREDUCIBLE;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getNoTransferFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_No_TransferP DCollectiveMatch<INSTANCE, BASE>::getNoTransferFct (void)
{
    return myCollNoTransferFct;
}

//=============================
// getSendFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_SendP DCollectiveMatch<INSTANCE, BASE>::getSendFct (void)
{
    return mySendFct;
}

//=============================
// getOpSendFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Op_SendP DCollectiveMatch<INSTANCE, BASE>::getOpSendFct (void)
{
    return myOpSendFct;
}

//=============================
// getSendNFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Send_nP DCollectiveMatch<INSTANCE, BASE>::getSendNFct (void)
{
    return mySendNFct;
}

//=============================
// getOpSendNFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Op_Send_nP DCollectiveMatch<INSTANCE, BASE>::getOpSendNFct (void)
{
    return myOpSendNFct;
}

//=============================
// getSendBuffersFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Send_buffersP DCollectiveMatch<INSTANCE, BASE>::getSendBuffersFct (void)
{
    return mySendBuffersFct;
}

//=============================
// getOpSendBuffersFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Op_Send_buffersP DCollectiveMatch<INSTANCE, BASE>::getOpSendBuffersFct (void)
{
    return myOpSendBuffersFct;
}

//=============================
// getSendCountsFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Send_countsP DCollectiveMatch<INSTANCE, BASE>::getSendCountsFct (void)
{
    return mySendCountsFct;
}

//=============================
// getOpSendCountsFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Op_Send_countsP DCollectiveMatch<INSTANCE, BASE>::getOpSendCountsFct (void)
{
    return myOpSendCountsFct;
}

//=============================
// getSendTypesFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Send_typesP DCollectiveMatch<INSTANCE, BASE>::getSendTypesFct (void)
{
    return mySendTypesFct;
}

//=============================
// getRecvFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_RecvP DCollectiveMatch<INSTANCE, BASE>::getRecvFct (void)
{
    return myRecvFct;
}

//=============================
// getRecvNFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Recv_nP DCollectiveMatch<INSTANCE, BASE>::getRecvNFct (void)
{
    return myRecvNFct;
}

//=============================
// getOpRecvNFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Op_Recv_nP DCollectiveMatch<INSTANCE, BASE>::getOpRecvNFct (void)
{
    return myOpRecvNFct;
}

//=============================
// getRecvBuffersFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Recv_buffersP DCollectiveMatch<INSTANCE, BASE>::getRecvBuffersFct (void)
{
    return myRecvBuffersFct;
}

//=============================
// getRecvCountsFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Recv_countsP DCollectiveMatch<INSTANCE, BASE>::getRecvCountsFct (void)
{
    return myRecvCountsFct;
}

//=============================
// getRecvTypesFct
//=============================
template <class INSTANCE, class BASE>
Must_Coll_Recv_typesP DCollectiveMatch<INSTANCE, BASE>::getRecvTypesFct (void)
{
    return myRecvTypesFct;
}

//=============================
// getPassTypeMatchInfoFct
//=============================
template <class INSTANCE, class BASE>
passTypeMatchInfoP DCollectiveMatch<INSTANCE, BASE>::getPassTypeMatchInfoFct (void)
{
    return myPassTypeMatchInfoFct;
}

//=============================
// getPassTypeMatchInfoTypesFct
//=============================
template <class INSTANCE, class BASE>
passTypeMatchInfoTypesP DCollectiveMatch<INSTANCE, BASE>::getPassTypeMatchInfoTypesFct (void)
{
    return myPassTypeMatchInfoTypesFct;
}

//=============================
// getSetNextEventStridedFct
//=============================
template <class INSTANCE, class BASE>
gtiSetNextEventStridedP DCollectiveMatch<INSTANCE, BASE>::getSetNextEventStridedFct (void)
{
    return myNextEventStridedFct;
}

//=============================
// ancestorHasIntraComm
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::ancestorHasIntraComm (
        int hasIntra,
        gti::I_ChannelId *cId,
        std::list<gti::I_ChannelId*> *outFinishedChannels)
{
    static bool hadIt = false;

    /*
     * If we already handled this we can filter out all other events of this type.
     */
    if (hadIt)
        return GTI_ANALYSIS_SUCCESS;

    /*
     * If this is the first of these events, lets create a new one
     */
    hadIt = true;

    if (!hasIntra && myHasIntraComm)
        myDoIntraLayerChecking = true;

    myAncestorDoesIntraChecking = hasIntra;

    bool ret = hasIntra;

    if (myDoIntraLayerChecking)
        ret = true;

    if (myAncestorHasIntraFct)
        (*myAncestorHasIntraFct) (ret);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handleIntraTypeMatchInfo
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::handleIntraTypeMatchInfo (
        MustParallelId pId,
        MustLocationId lId,
        MustRemoteIdType commRId,
        MustRemoteIdType typeRId,
        int numCounts,
        int* counts,
        int firstRank,
        int collectiveNumber,
        int collId)
{
    //==1) Prepare
    I_CommPersistent* commInfo = myCommTrack->getPersistentRemoteComm(pId, commRId);
    I_DatatypePersistent* typeInfo = myTypeTrack->getPersistentRemoteDatatype(pId, typeRId);

    if (!commInfo || !typeInfo || numCounts <= 0)
    {
        assert(0); //Should not happen
        return GTI_ANALYSIS_SUCCESS;
    }

    //==2) Create type match info
    DCollectiveTypeMatchInfo *matchInfo = new DCollectiveTypeMatchInfo (
            pIdToRank (pId),
            pId,
            lId,
            commInfo,
            typeInfo,
            numCounts,
            counts,
            firstRank,
            collectiveNumber,
            (MustCollCommType) collId);

    //==3) Pass info to comm matching info
    std::set<DCollectiveCommInfo*>::iterator iter;
    DCollectiveCommInfo* commMatchInfo = NULL;

    for (iter = myComms.begin(); iter != myComms.end(); iter++)
    {
        if ((*iter)->getComm()->compareComms(commInfo))
        {
            commMatchInfo = *iter;
            break;
        }
    }

    //Do we need a new Info?
    if (!commMatchInfo)
    {
        commInfo->copy();
        commMatchInfo = new DCollectiveCommInfo (commInfo);
        myComms.insert(commMatchInfo);
    }

    commMatchInfo->addNewTypeMatchInfo (matchInfo);

#ifdef DCOLL_DEBUG
    printAsDot ();
#endif /*DCOLL_DEBUG*/

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// handleIntraTypeMatchInfoTypes
//=============================
template <class INSTANCE, class BASE>
GTI_ANALYSIS_RETURN DCollectiveMatch<INSTANCE, BASE>::handleIntraTypeMatchInfoTypes (
        MustParallelId pId,
        MustLocationId lId,
        MustRemoteIdType commRId,
        int numCountsAndTypes,
        MustRemoteIdType* typeRIds,
        int* counts,
        int firstRank,
        int collectiveNumber,
        int collId )
{
    //==1) Prepare
    if (numCountsAndTypes <= 0 || !typeRIds)
    {
        assert(0); //Should not happen
        return GTI_ANALYSIS_SUCCESS;
    }

    I_CommPersistent* commInfo = myCommTrack->getPersistentRemoteComm(pId, commRId);
    I_DatatypePersistent** typeInfos = new I_DatatypePersistent*[numCountsAndTypes];

    for (int i = 0; i < numCountsAndTypes; i++)
    {
        if (counts[i] == -1) continue;

        typeInfos[i] = myTypeTrack->getPersistentRemoteDatatype(pId, typeRIds[i]);

        if (!typeInfos[i])
        {
            assert(0); //Should not happen
            return GTI_ANALYSIS_SUCCESS;
        }
    }

    if (!commInfo)
    {
        assert(0); //Should not happen
        return GTI_ANALYSIS_SUCCESS;
    }

    //==2) Create type match info
    DCollectiveTypeMatchInfo *matchInfo = new DCollectiveTypeMatchInfo (
            pIdToRank (pId),
            pId,
            lId,
            commInfo,
            numCountsAndTypes,
            typeInfos,
            counts,
            firstRank,
            collectiveNumber,
            (MustCollCommType) collId);

    //==3) Pass info to comm matching info
    std::set<DCollectiveCommInfo*>::iterator iter;
    DCollectiveCommInfo* commMatchInfo = NULL;

    for (iter = myComms.begin(); iter != myComms.end(); iter++)
    {
        if ((*iter)->getComm()->compareComms(commInfo))
        {
            commMatchInfo = *iter;
            break;
        }
    }

    //Do we need a new Info?
    if (!commMatchInfo)
    {
        commInfo->copy();
        commMatchInfo = new DCollectiveCommInfo (commInfo);
        myComms.insert(commMatchInfo);
    }

    commMatchInfo->addNewTypeMatchInfo (matchInfo);

#ifdef DCOLL_DEBUG
    printAsDot ();
#endif /*DCOLL_DEBUG*/

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// timeout
//=============================
template <class INSTANCE, class BASE>
void DCollectiveMatch<INSTANCE, BASE>::timeout()
{
    //If this is the root, we should querry a flush in case of a timeout
    if (!myIsReduction)
    {
        //2) Request that all ancestor TBON nodes provide us with all information they have
        gtiNotifyFlushP f;
        if (ModuleBase<INSTANCE, BASE>::getBroadcastFunction("gtiNotifyFlush", (GTI_Fct_t*) &f) == GTI_SUCCESS)
            (*f) ();
    }
    else
    {
        std::set<DCollectiveCommInfo*>::iterator iter;

        for (iter = myComms.begin(); iter != myComms.end(); iter++)
        {
            if (!*iter)
                continue;

            (*iter)->timeout ();
        }

#ifdef DCOLL_DEBUG
        if (myHadNewOp)
            printAsDot ();
#endif /*DCOLL_DEBUG*/
    }
}

//=============================
// printAsDot
//=============================
template <class INSTANCE, class BASE>
bool DCollectiveMatch<INSTANCE, BASE>::printAsDot (void)
{
    static int invocationCount = 0;
    std::stringstream stream;

    if (myIsReduction)
        stream << "must_DCollectiveReductionDebug_" << getpid() << "_" << ++invocationCount << ".dot";
    else
        stream << "must_DCollectiveRootDebug_" << getpid() << "_" << ++invocationCount << ".dot";
    std::ofstream out (stream.str().c_str());

    out
        << "digraph DCollectiveMatchDebug_" << getpid() << std::endl
        << "{" << std::endl;

    std::set<DCollectiveCommInfo*>::iterator iter;
    int i = 0;
    for (iter = myComms.begin(); iter != myComms.end(); iter++)
    {
        if (!*iter)
            continue;
        std::stringstream streamTemp;
        streamTemp << "Comm_" << ++i;
        (*iter)->printAsDot(out, streamTemp.str(), myLIdMod);
    }

    out << "}" << std::endl;

    out.close();

    myHadNewOp = false;

    return true;
}

//=============================
// getWorldSizedCountArray
//=============================
template <class INSTANCE, class BASE>
int* DCollectiveMatch<INSTANCE, BASE>::getWorldSizedCountArray (void)
{
    if (!myReusableCountArray && myWorldSize > 0)
    {
        myReusableCountArray = new int [myWorldSize];
    }

    return myReusableCountArray;
}

//=============================
// getWorldSize
//=============================
template <class INSTANCE, class BASE>
int DCollectiveMatch<INSTANCE, BASE>::getWorldSize (void)
{
    return myWorldSize;
}

//=============================
// registerListener
//=============================
template <class INSTANCE, class BASE>
void DCollectiveMatch<INSTANCE, BASE>::registerListener (I_DCollectiveListener *listener)
{
    myListener = listener;
}

//=============================
// registerCommListener
//=============================
template <class INSTANCE, class BASE>
void DCollectiveMatch<INSTANCE, BASE>::registerCommListener (I_CollCommListener *listener)
{
    myCommListeners.push_back(listener);
}

} /* namespace must */
