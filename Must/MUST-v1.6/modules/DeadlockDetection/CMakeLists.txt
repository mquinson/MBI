# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for the modules/DeadlockDetection directory.
#
# @author Tobias Hilbrich
# @date 25.07.2011

INCLUDE_DIRECTORIES(BEFORE BlockingState CollMatch OperationReordering P2PMatch DWaitState Utility)

#Install headers
MUST_INSTALL(
    FILES 
        OperationReordering/I_OperationReordering.h
        OperationReordering/I_Operation.h
        P2PMatch/I_P2PMatch.h
        P2PMatch/I_MsgMatchReduction.h
        P2PMatch/I_DP2PMatch.h
        Utility/I_OpSplitter.h
        Utility/I_WcUpdate.h
        CollMatch/I_CollectiveMatch.h
        CollMatch/I_DCollectiveMatch.h
        CollMatch/I_DCollectiveMatchRoot.h
        CollMatch/I_DCollectiveMatchReduction.h
        CollMatch/I_DCollectiveInitNotify.h
        BlockingState/I_BlockingState.h
        DWaitState/I_DWaitState.h
        P2PMatch/I_P2PMatchListener.h
        CollMatch/I_CollMatchListener.h
        DWaitState/I_DP2PListener.h
        DWaitState/I_DCollectiveListener.h
        DWaitState/I_CollCommListener.h
        DWaitState/I_DWaitStateCollMgr.h
        DWaitState/I_DWaitStateCollReduction.h
        DWaitState/I_DWaitStateWfgMgr.h
        Utility/DeadlockApi.h
        Utility/DistributedDeadlockApi.h
    DESTINATION 
        include)

#Build and install modules 
GTI_MAC_ADD_MODULE (operationReordering "OperationReordering/OperationReordering.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (opSplitter "Utility/OpSplitter.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (wcUpdate "Utility/WcUpdate.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (p2PMatch "P2PMatch/P2PMatch.cpp;P2PMatch/P2POp.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (msgMatchReduction "P2PMatch/MsgMatchReduction.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (collectiveMatch "CollMatch/CollectiveMatch.cpp;CollMatch/CollectiveOp.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (
    blockingState 
    "BlockingState/BlockingState.cpp;BlockingState/BlockingOp.cpp;BlockingState/BlockingP2P.cpp;BlockingState/BlockingColl.cpp;BlockingState/BlockingCompletion.cpp;BlockingState/BlockingRequestCompletion.cpp;Utility/Wfg.cpp;Utility/wfgsigreduce.cpp;Utility/MatchExplorer.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}"
    "CXX")
    
#Distributed modules
GTI_MAC_ADD_MODULE (dP2PMatch "P2PMatch/DP2PMatch.cpp;P2PMatch/DP2POp.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (dCollectiveMatchReduction "CollMatch/DCollectiveMatchReduction.cpp;CollMatch/DCollectiveOp.cpp;CollMatch/DCollectiveCommInfo.cpp;CollMatch/DCollectiveWave.cpp;CollMatch/DCollectiveTypeMatchInfo.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (dCollectiveMatchRoot "CollMatch/DCollectiveMatchRoot.cpp;CollMatch/DCollectiveOp.cpp;CollMatch/DCollectiveCommInfo.cpp;CollMatch/DCollectiveWave.cpp;CollMatch/DCollectiveTypeMatchInfo.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (dCollectiveInitNotify "CollMatch/DCollectiveInitNotify.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (
    dWaitState 
    "DWaitState/DWaitState.cpp;DWaitState/QOp.cpp;DWaitState/QOpCommunication.cpp;DWaitState/QOpCommunicationColl.cpp;DWaitState/QOpCommunicationCollNonBlocking.cpp;DWaitState/QOpCommunicationP2P.cpp;DWaitState/QOpCommunicationP2PNonBlocking.cpp;DWaitState/QOpCompletion.cpp;${BASE_SOURCES}"
    "CXX")
GTI_MAC_ADD_MODULE (dWaitStateCollMgr "DWaitState/DWaitStateCollMgr.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (dWaitStateCollReduction "DWaitState/DWaitStateCollReduction.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (dWaitStateWfgMgr "DWaitState/DWaitStateWfgMgr.cpp;Utility/Wfg.cpp;Utility/wfgsigreduce.cpp;${BASE_SOURCES}" "CXX")
