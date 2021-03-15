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
#       MUST file for the modules/common directory.
#
# @author Tobias Hilbrich
# @date 20.08.2010

###Configure config.h
CONFIGURE_FILE(gtiConfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/gtiConfig.h")

#Install headers
INSTALL(FILES 
            "GtiDefines.h" 
            "GtiEnums.h"
            "GtiMacros.h"
            "GtiTypes.h"
            "GtiApi.h"
            "GtiTLS.h"
            "${CMAKE_CURRENT_BINARY_DIR}/gtiConfig.h"
    DESTINATION 
        "${CMAKE_INSTALL_PREFIX}/src"
    PERMISSIONS 
        OWNER_READ OWNER_WRITE 
        GROUP_READ 
        WORLD_READ
    )

