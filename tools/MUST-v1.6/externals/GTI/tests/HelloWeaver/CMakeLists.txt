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
#       GTI CMake file for reduction example in tests/reduction directory.
#
# @author Tobias Hilbrich
# @date 25.12.2010
#
# TODO:
#  - Currently likely not very portable, should work well with OpenMPI atm.
#

ADD_SUBDIRECTORY (modules)

###################
#Set libs, dirs, ... for executable
SET(CMAKE_C_FLAGS ${MPI_CFLAGS})
INCLUDE_DIRECTORIES (${MPI_C_INCLUDE_PATH})

FOREACH (lan C CXX Fortran)
    SET (${lan}_compiler_to_use ${CMAKE_${lan}_COMPILER})
    IF (MPI_${lan}_COMPILER)
        SET (${lan}_compiler_to_use ${MPI_${lan}_COMPILER})
    ENDIF ()
ENDFOREACH ()

SET(CMAKE_C_COMPILER ${C_compiler_to_use})
SET(CMAKE_CXX_COMPILER ${CXX__compiler_to_use})
SET(CMAKE_EXE_LINKER_FLAGS ${MPI_C_LINK_FLAGS})#For the tests we have to link against P^nMPI lib
#LINK_DIRECTORIES("${PnMPI_LIBRARY_DIR}")
SET (TEST_LIBRARIES ${PnMPI_C_LIBRARIES}) 

###################
#Add test case
ADD_EXECUTABLE (HelloWeaver test.cpp)
TARGET_LINK_LIBRARIES(HelloWeaver ${TEST_LIBRARIES})
GET_TARGET_PROPERTY(WEAVER_EXE weaver LOCATION)
GET_TARGET_PROPERTY(WRAPPGEN_EXE wrappgen LOCATION)
GET_TARGET_PROPERTY(BUILDGEN_EXE buildgen LOCATION)
GET_TARGET_PROPERTY(MODCONFGEN_EXE modconfgen LOCATION)
GET_TARGET_PROPERTY(HELLOWEAVER_EXAMPLE HelloWeaver LOCATION)
CONFIGURE_FILE(analysis-spec.xml ${PROJECT_BINARY_DIR}/tests/HelloWeaver/analysis-spec.xml @ONLY)
CONFIGURE_FILE(api-spec.xml ${PROJECT_BINARY_DIR}/tests/HelloWeaver/api-spec.xml @ONLY)
CONFIGURE_FILE(RunTest.sh.in ${PROJECT_BINARY_DIR}/tests/HelloWeaver/RunTest.sh)

ADD_TEST(HelloWeaverTest 
    bash ${PROJECT_BINARY_DIR}/tests/HelloWeaver/RunTest.sh
    )
