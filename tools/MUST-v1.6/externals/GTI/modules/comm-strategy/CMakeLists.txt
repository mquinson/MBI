# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

##
# @file CMakeLists.cmake
#       MUST file for the modules/comm-strategy directory.
#
# @author Tobias Hilbrich
# @date 16.03.2009

SET (BASE_SOURCES "")
SET (ERRH_SOURCES "CStratCrashHandling.cpp")

##Inter-layer communication
GTI_MAC_ADD_MODULE (commStrategyDownTemplate "CommStrategyDownTemplate.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (commStrategyUpTemplate "CommStrategyUpTemplate.cpp;${BASE_SOURCES};${ERRH_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratSimpleDown "CStratSimpleDown.cpp;CStratQueue.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratSimpleUp "CStratSimpleUp.cpp;CStratQueue.cpp;${BASE_SOURCES};${ERRH_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratIsendDown "CStratIsendDown.cpp;CStratIsend.cpp;CStratQueue.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratIsendUp "CStratIsendUp.cpp;CStratIsend.cpp;CStratQueue.cpp;${BASE_SOURCES};${ERRH_SOURCES}" "CXX") 
GTI_MAC_ADD_MODULE (cStratThreadedDown "CStratThreadedDown.cpp;CStratThreaded.cpp;CStratQueue.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratThreadedUp "CStratThreadedUp.cpp;CStratThreaded.cpp;CStratQueue.cpp;${BASE_SOURCES};${ERRH_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratPRecvDown "CStratPRecvDown.cpp;CStratPRecv.cpp;CStratQueue.cpp;CStratIsend.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratPRecvUp "CStratPRecvUp.cpp;CStratPRecv.cpp;CStratQueue.cpp;CStratIsend.cpp;${BASE_SOURCES};${ERRH_SOURCES}" "CXX")

#Let the up strategies signal handler raise a panic in order to flush all outstanding upwards communication
GET_TARGET_PROPERTY(BASE_FLAGS cStratSimpleUp COMPILE_FLAGS)
IF (NOT BASE_FLAGS)
    SET(BASE_FLAGS "")
ENDIF ()
SET_TARGET_PROPERTIES(commStrategyUpTemplate cStratSimpleUp cStratIsendUp cStratThreadedUp PROPERTIES COMPILE_FLAGS "${BASE_FLAGS} -DGTI_STRAT_RAISE_PANIC")

##Intra-layer communication
GTI_MAC_ADD_MODULE (cStratIsendIntra "CStratIsendIntra.cpp;CStratIsend.cpp;CStratQueue.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratThreadedIntra "CStratThreadedIntra.cpp;CStratThreaded.cpp;CStratQueue.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cStratPRecvIntra "CStratPRecvIntra.cpp;CStratPRecv.cpp;CStratQueue.cpp;CStratIsend.cpp;${BASE_SOURCES}" "CXX")
