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
#       CMake file for system-builder directory.
#
# @author Tobias Hilbrich
# @date 04.01.2010

#Add Include directories
#INCLUDE_DIRECTORIES(${PnMPI_INCLUDE_DIR} ${MPI_C_INCLUDE_PATH}) #P^nMPI inc dir needed for modules!

#Include sub-directories
ADD_SUBDIRECTORY (specifications)
ADD_SUBDIRECTORY (dtd)
ADD_SUBDIRECTORY (weaver)
ADD_SUBDIRECTORY (wrapper-generator)
ADD_SUBDIRECTORY (receival-generator)
ADD_SUBDIRECTORY (buildgen)
ADD_SUBDIRECTORY (config-generator)
