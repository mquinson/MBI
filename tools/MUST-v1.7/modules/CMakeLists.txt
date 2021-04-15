# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for the modules directory.
#
# @author Tobias Hilbrich
# @date 03.01.2011

#Add Include directories
INCLUDE_DIRECTORIES(
    Common
    MustBase
    ResourceTracking/Comm
    ResourceTracking/Datatype
    ResourceTracking/Err
    ResourceTracking/Group
    ResourceTracking/Keyval
    ResourceTracking/Op
    ResourceTracking/Request
    ResourceTracking/Utility
    Preconditioner
    ${GTI_INCLUDE_PATH}
    ${GTI_SOURCE_PATH}
    ${PnMPI_INCLUDE_PATH}
    ${MPI_C_INCLUDE_PATH}
    "${CMAKE_CURRENT_BINARY_DIR}/Common"
)

#Build base sources
SET (BASE_SOURCES)


# Recurse into all subdirectories.
#
# NOTE: Subdirectories will be searched dynamically to support excluding
#       specific modules by simply removing its folder (i.e. to not ship
#       experimental modules on deploy).
FILE(GLOB SUBDIRECTORIES "*")
FOREACH (DIR ${SUBDIRECTORIES})
    IF (EXISTS "${DIR}/CMakeLists.txt")
        ADD_SUBDIRECTORY(${DIR})
    ENDIF ()
ENDFOREACH ()
