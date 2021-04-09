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
#       MUST file for the modules/utility directory.
#
# @author Tobias Hilbrich
# @date 16.03.2009
#
# TODO: Actually create utility modules use the code as part of the modules ?
#
# Currently:
#   To simplify the build use the sources provided here directly
#   as part of the modules, thus, there is no need to link to
#   additional libraries and the like.
#    
INSTALL(FILES 
        "I_ShutdownHandler.h" 
        "I_ShutdownReceiver.h"
        "I_PanicHandler.h"
        "I_PanicReceiver.h"
        "I_PanicFilter.h"
        "I_Profiler.h"
        "I_FloodControl.h"
        "I_BreakManager.h"
        "I_BreakEnforcer.h"
        "I_BreakReduction.h"
    DESTINATION 
        "${CMAKE_INSTALL_PREFIX}/include"
    PERMISSIONS 
        OWNER_READ OWNER_WRITE 
        GROUP_READ 
        WORLD_READ
    )
    
GTI_MAC_ADD_MODULE (shutdownHandler "ShutdownHandler.cpp" "CXX")
GTI_MAC_ADD_MODULE (shutdownReceiver "ShutdownReceiver.cpp" "CXX")
GTI_MAC_ADD_MODULE (panicHandler "PanicHandler.cpp" "CXX")
GTI_MAC_ADD_MODULE (panicReceiver "PanicReceiver.cpp" "CXX")
GTI_MAC_ADD_MODULE (panicFilter "PanicFilter.cpp" "CXX")
GTI_MAC_ADD_MODULE (profilerMpi "ProfilerMpi.cpp" "CXX")
GTI_MAC_ADD_MODULE (floodControl "FloodControl.cpp" "CXX")
GTI_MAC_ADD_MODULE (breakManager "BreakManager.cpp" "CXX")
GTI_MAC_ADD_MODULE (breakEnforcer "BreakEnforcer.cpp" "CXX")
GTI_MAC_ADD_MODULE (breakReduction "BreakReduction.cpp" "CXX")
