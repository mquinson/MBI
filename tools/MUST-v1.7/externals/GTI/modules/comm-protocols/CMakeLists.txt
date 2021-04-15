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
#       GTI build file for the modules/comm-protocols directory.
#
# @author Tobias Hilbrich
# @date 16.03.2009

SET (BASE_SOURCES)

GTI_MAC_ADD_MODULE (commProtocolTemplate "CommProtocolTemplate.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (cProtSimpleTcp "CProtSimpleTcp1x1.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE_UNPATCHED (cProtMpiSplited "CProtMpiSplited.cpp;${BASE_SOURCES}" "CXX")
add_dependencies(cProtMpiSplited generate)
if(GTI_THREAD_SAFETY)
GTI_MAC_ADD_MODULE (cProtSharedMemory "CProtSharedMemory.cpp;${BASE_SOURCES}" "CXX")
#target_compile_options(cProtSharedMemory PUBLIC "-fsanitize=thread")
endif(GTI_THREAD_SAFETY)
GTI_MAC_ADD_MODULE (cProtIpcSM "CProtIPC-SM.cpp;${BASE_SOURCES}" "CXX")
target_compile_options(cProtSharedMemory PRIVATE
    -ftls-model=initial-exec)

#TODO: The below can run on OSX, but won't at the moment. It uses pthread_spinlock_t which is not present in OSX, but I am sure we can create a workaround there
IF (NOT APPLE)
  GTI_MAC_ADD_MODULE (cProtSMQueue "CProtSMQueue.cpp;../utility/GtiHelper.cpp;../utility/ThreadChannel.cpp;../utility/VectorBuffer.cpp;${BASE_SOURCES}" "CXX")
ENDIF ()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE(CProtMpiSplitModule.h.in ${CMAKE_CURRENT_BINARY_DIR}/CProtMpiSplitModule.h @ONLY)
add_wrapped_file(CProtMpiSplitWorld.cpp CProtMpiSplitWorld.w)
add_wrapped_file(CProtMpiRenameWorld.cpp CProtMpiRenameWorld.w)
SET_SOURCE_FILES_PROPERTIES(CProtMpiSplitModule.cpp PROPERTIES COMPILE_FLAGS -g) #For MUST-DDT integration we want to "show" some symbols
SET_SOURCE_FILES_PROPERTIES(CProtMpiSplitWorld.cpp PROPERTIES COMPILE_FLAGS -g) #For MUST-DDT integration we want to "show" some symbols
GTI_MAC_ADD_MODULE_UNPATCHED (cProtMpiSplitComm CProtMpiSplitWorld.cpp "CXX") #No regular module, needs no base stuff
# SET_TARGET_PROPERTIES(cProtMpiSplitComm PROPERTIES DEFINE_SYMBOL "SPLIT_STACKS")
GTI_MAC_ADD_MODULE (cProtMpiCommRewrite CProtMpiRenameWorld.cpp "CXX") #No regular module, needs no base stuff
# SET_TARGET_PROPERTIES(cProtMpiCommRewrite PROPERTIES DEFINE_SYMBOL "COMM_REWRITE")
