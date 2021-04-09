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
#       MUST file for the modules directory.
#
# @author Tobias Hilbrich
# @date 16.03.2009

#Add Include directories
INCLUDE_DIRECTORIES(  "${CMAKE_CURRENT_BINARY_DIR}/common" common interfaces utility trace-records state-trackers checks gti-internal ${PnMPI_INCLUDE_PATH} ${MPI_C_INCLUDE_PATH}) #P^nMPI inc dir needed for modules!


#Include sub-directories
IF (BUILD_BACKEND)
    ADD_SUBDIRECTORY (common)
    ADD_SUBDIRECTORY (interfaces)
    ADD_SUBDIRECTORY (comm-protocols)
    ADD_SUBDIRECTORY (comm-strategy)
    ADD_SUBDIRECTORY (placement-drivers)
    ADD_SUBDIRECTORY (gti-internal)
    ADD_SUBDIRECTORY (utility)
ENDIF()

IF (BUILD_FRONTEND)
    ADD_SUBDIRECTORY (trace-records)
ENDIF()