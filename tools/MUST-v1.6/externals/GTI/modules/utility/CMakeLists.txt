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

##Create the tcp server
ADD_EXECUTABLE (SimpleTcpServer SimpleTcpServer.cpp)

INSTALL(TARGETS SimpleTcpServer
    PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
        GROUP_EXECUTE GROUP_READ 
        WORLD_EXECUTE WORLD_READ
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

##Install other important utility source files    
INSTALL(FILES 
        "ModuleBase.h" 
        "ModuleBase.hxx"
        "GtiChannelId.h"
        "GtiChannelId.hpp"
        "GtiHelper.h"
        "GtiHelper.cpp"
        "CompletionTree.h"
        "CompletionTree.cpp"
        "I_ChannelTree.h"
        "I_ChannelTree.hpp"
    DESTINATION 
        "${CMAKE_INSTALL_PREFIX}/src"
    PERMISSIONS 
        OWNER_READ OWNER_WRITE 
        GROUP_READ 
        WORLD_READ
    )
