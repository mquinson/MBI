# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for basic test.
#
# @author Tobias Hilbrich
# @date 03.01.2011
#

##################
#Recurse into sub directories
ADD_SUBDIRECTORY (modules)

###################
#Add test case
addTestExecutable ("basic" "basic.c")
CONFIGURE_FILE(analysis_spec.xml ${CMAKE_CURRENT_BINARY_DIR}/analysis_spec.xml @ONLY)

configureTest (
    ${CMAKE_CURRENT_BINARY_DIR}/layout.xml
    "TestLog:Basic;FinalizeNotify:MUST_Base"
    ""
    "TestLocation:Basic;MsgLoggerStdOut:MUST_Base;MsgLoggerHtml:MUST_Base"
    4
    2
    )

ADD_TEST(NAME basic COMMAND
    bash ${MUSTRUN} 
        --must:timeout ${MUSTRUN_TIMEOUT}
        --must:mpiexec ${MPIEXEC} 
        --must:np ${MPIEXEC_NUMPROC_FLAG} 
        --must:layout ${CMAKE_CURRENT_BINARY_DIR}/layout.xml 
        --must:temp tempBasic 
        --must:clean
        --must:analyses ${CMAKE_CURRENT_BINARY_DIR}/analysis_spec.xml
        ${MPIEXEC_NUMPROC_FLAG} 4  $<TARGET_FILE:basic> 
    )
    
SET_TESTS_PROPERTIES (basic PROPERTIES PASS_REGULAR_EXPRESSION "Test log message!.*MPI_Init.*rank")


addProcessLocalTest ("basic_initialized"
    "initialized.c"
    2
    ""
    "Error;ERROR;Warning;WARNING"
)
