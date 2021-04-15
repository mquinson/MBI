# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for the modules/Common directory.
#
# @author Tobias Hilbrich
# @date 04.02.2011

#Install headers
MUST_INSTALL(
    FILES 
        MustEnums.h 
#         MustTypes.h
        MustDefines.h  
        mustFeaturetested.h 
    DESTINATION 
        include)

###Configure config.h
CONFIGURE_FILE(mustConfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/mustConfig.h")
CONFIGURE_FILE(MustTypes.h.in "${CMAKE_CURRENT_BINARY_DIR}/MustTypes.h")

#Install headers
MUST_INSTALL(
    FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/mustConfig.h"
        "${CMAKE_CURRENT_BINARY_DIR}/MustTypes.h"
    DESTINATION 
        include)
