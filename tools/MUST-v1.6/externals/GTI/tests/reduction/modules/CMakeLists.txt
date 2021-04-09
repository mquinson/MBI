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
#       GTI CMake file for reduction example in tests/reduction/modules directory.
#
# @author Tobias Hilbrich
# @date 25.12.2010
#
# TODO:
#  - Currently likely not very portable, should work well with OpenMPI atm.
#

###################
#Build Testing modules ....
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/modules/utility ${PROJECT_SOURCE_DIR}/modules/common ${PROJECT_SOURCE_DIR}/modules/interfaces ${PnMPI_INCLUDE_DIR} ${MPI_C_INCLUDE_PATH} "${PROJECT_SOURCE_DIR}/tests") #P^nMPI inc dir needed for modules!

SET (BASE_SOURCES)

GTI_MAC_ADD_MODULE (sumFloatReduction "SumFloatReduction.cpp;${PROJECT_SOURCE_DIR}/modules/utility/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (printFloatSumExample "PrintFloatSum.cpp;${BASE_SOURCES}" "CXX")

