# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for tests directory.
#
# @author Tobias Hilbrich
# @date 03.01.2011
#
# The tests have to be used as follows:
#    ccmake       #configure with CMake, enable the test option there, generate
#    make         #build
#    make install #install and patch the libraries
#    make test    #run the tests
#
# Currently likely not very portable, should work well with OpenMPI atm.
#

##################
#Recurse into sub directories

#Add Include directories for modules
INCLUDE_DIRECTORIES(
    "${CMAKE_BINARY_DIR}/modules/Common" #For results of feature tests
    "${CMAKE_SOURCE_DIR}/modules/MustBase"  #For MustParallelId and the like
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Comm" 
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Datatype"
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Err"
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Group"
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Keyval"
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Op"
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Request"
    "${CMAKE_SOURCE_DIR}/modules/ResourceTracking/Utility"
    "${CMAKE_SOURCE_DIR}/modules/Common"
    "${GTI_INCLUDE_PATH}" # For I_Module and the like
    "${GTI_SOURCE_PATH}"  # For gti::ModuleBase and the like
    ${PnMPI_INCLUDE_DIR}  # Used by gti::ModuleBase
    ${MPI_C_INCLUDE_PATH}      # For mpi.h
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#Build base sources variable for modules
SET (BASE_SOURCES)
SET (MUSTRUN "${CMAKE_BINARY_DIR}/utility/mustrun")


# Recurse into all subdirectories.
#
# NOTE: Subdirectories will be searched dynamically to support excluding
#       specific test cases by simply removing their folders (i.e. to not ship
#       experimental tests on deploy).
FILE(GLOB SUBDIRECTORIES "*")
FOREACH (DIR ${SUBDIRECTORIES})
    IF (EXISTS "${DIR}/CMakeLists.txt")
        ADD_SUBDIRECTORY(${DIR})
    ENDIF ()
ENDFOREACH ()
