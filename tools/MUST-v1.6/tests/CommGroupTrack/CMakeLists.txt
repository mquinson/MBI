# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for communicator and group tracking tests.
#
# @author Tobias Hilbrich
# @date 06.03.2011
#

##################
#Recurse into sub directories
ADD_SUBDIRECTORY (modules)

###################
#Add test case
CONFIGURE_FILE(analysis_spec.xml ${CMAKE_CURRENT_BINARY_DIR}/analysis_spec.xml @ONLY)

SET (tests "Comm" "Group" "InterComm")
SET (CommNumProcs 5)
SET (InterCommNumProcs 5)
SET (GroupNumProcs 4)

FOREACH (test ${tests})
    ### test${test}Track.c
    addTestExecutable ("test${test}Track" "test${test}Track.c")
    
    configureTest (
        ${CMAKE_CURRENT_BINARY_DIR}/${test}Layout.xml
        "FinalizeNotify:MUST_Base"
        ""
        "PrintCommGroup:TestCommGroupTrack;MsgLoggerHtml:MUST_Base;MsgLoggerStdOut:MUST_Base"
        ${${test}NumProcs}
        2
        )
    
    ADD_TEST(NAME test${test}Track COMMAND
        bash ${MUSTRUN} 
            --must:timeout ${MUSTRUN_TIMEOUT}
            --must:mpiexec ${MPIEXEC} 
            --must:np ${MPIEXEC_NUMPROC_FLAG} 
            --must:layout ${CMAKE_CURRENT_BINARY_DIR}/${test}Layout.xml 
            --must:temp temp${test} 
            --must:clean
            --must:analyses ${CMAKE_CURRENT_BINARY_DIR}/analysis_spec.xml
            ${MPIEXEC_NUMPROC_FLAG} ${${test}NumProcs}  $<TARGET_FILE:test${test}Track> 
    )
    
ENDFOREACH (test)

SET_TESTS_PROPERTIES (testCommTrack PROPERTIES PASS_REGULAR_EXPRESSION
        "MPI_COMM_NULL.*MPI_COMM_SELF.*size=1 table: 0->0.*Communicator created at.*size=3 table: 0->0; 1->2; 2->4.*MPI_Comm_create.*Communicator created at.*size=3 table: 0->0; 1->2; 2->4.*MPI_Comm_dup.*Communicator created at.*size=3 table: 0->2; 1->1; 2->0.*MPI_Comm_split.*Communicator created at.*size=5 table: 0->0; 1->1; 2->2; 3->3; 4->4.*MPI_Graph_create.*Communicator created at.*size=4 table: 0->0; 1->1; 2->2; 3->3.*MPI_Cart_create.*Communicator created at.*size=5 table: 0->0; 1->1; 2->2; 3->3; 4->4.*MPI_Comm_dup")
SET_TESTS_PROPERTIES (testInterCommTrack PROPERTIES PASS_REGULAR_EXPRESSION
        "size=3 table: 0->4; 1->2; 2->0.*Communicator.*size=3.*is an intercommunicator.*table: 0->4; 1->2; 2->0.*remote group: size=2 table: 0->3; 1->1.*MPI_Intercomm_create.*Communicator.*size=5 table: 0->4; 1->2; 2->0; 3->3; 4->1")
SET_TESTS_PROPERTIES (testGroupTrack PROPERTIES PASS_REGULAR_EXPRESSION
        "MPI_GROUP_NULL.*MPI_GROUP_EMPTY size=0.*size=1 table: 0->0.*MPI_Comm_group.*size=4 table: 0->0; 1->1; 2->2; 3->3.*MPI_Comm_group.*size=4 table: 0->3; 1->2; 2->1; 3->0.*MPI_Group_incl.*size=4 table: 0->0; 1->1; 2->2; 3->3.*MPI_Group_incl.*MPI_GROUP_EMPTY size=0.*size=2 table: 0->3; 1->0.*MPI_Group_excl.*size=2 table: 0->0; 1->2.*MPI_Group_range_incl.*size=2 table: 0->3; 1->1.*MPI_Group_range_excl.*size=3 table: 0->3; 1->0; 2->2.*MPI_Group_union.*size=1 table: 0->3.*MPI_Group_intersection.*size=2 table: 0->2; 1->0.*MPI_Group_difference")
        
		 

        
        
