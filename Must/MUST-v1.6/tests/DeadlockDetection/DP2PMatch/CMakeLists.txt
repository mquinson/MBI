# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for distributed P2P matching tests that are part of deadlock detection.
#
# @author Tobias Hilbrich
# @date 20.02.2012
#

#
# TODO 
# This is not yet in production! We use the manual version of creating tests here as the default 
# instantiation of mustrun uses the centralized p2p match still. We enforce DP2PMatch with
# the manually created layout here.
#

###################
#Add test case
SET (tests "simpleNoLoss" "anyTagNoLoss" "nonBlockingNoLoss" "wcRecvNoLoss" "wcIrecvNoLoss" "wcIrecvExNoLoss" "wcIrecvEx2NoLoss" "wcIrecvEx2Loss" "commNoLoss" "interCommNoLoss" "sendRecvNoLoss" "startAllNoLoss" "complexNoLoss")

 SET (NUMsimpleNoLoss 2)
 SET (NUManyTagNoLoss 2)
 SET (NUMnonBlockingNoLoss 2)
 SET (NUMwcRecvNoLoss 2)
 SET (NUMwcIrecvNoLoss 2)
 SET (NUMwcIrecvExNoLoss 2)
 SET (NUMwcIrecvEx2NoLoss 4)
 SET (NUMwcIrecvEx2Loss 4)
 SET (NUMcommNoLoss 2)
 SET (NUMinterCommNoLoss 4)
 SET (NUMsendRecvNoLoss 2)
 SET (NUMstartAllNoLoss 2)
 SET (NUMcomplexNoLoss 4)

FOREACH (test ${tests})
    addTestExecutable ("D${test}" "${CMAKE_CURRENT_SOURCE_DIR}/../P2PMatch/${test}.c")
    
    configureTestIntra (
        ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml
        "WcUpdate:MUST_Deadlock;OpSplitter:MUST_Deadlock;FinalizeNotify:MUST_Base"
        "DP2PMatch:MUST_DistributedDeadlock"
        "MsgLoggerHtml:MUST_Base;MsgLoggerStdOut:MUST_Base"
        ${NUM${test}}
        2
        )
    
    ADD_TEST(NAME D${test}Test COMMAND
        bash ${MUSTRUN} 
            --must:timeout ${MUSTRUN_TIMEOUT}
            --must:mpiexec ${MPIEXEC} 
            --must:np ${MPIEXEC_NUMPROC_FLAG} 
            --must:layout ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml 
            --must:temp temp${test}
            --must:clean
            ${MPIEXEC_NUMPROC_FLAG} ${NUM${test}}  "$<TARGET_FILE:D${test}>" 
            )
ENDFOREACH(test)


## No Loss cases    
 SET_TESTS_PROPERTIES (DsimpleNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DanyTagNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DnonBlockingNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DwcRecvNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DwcIrecvNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DwcIrecvExNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DwcIrecvEx2NoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcommNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DinterCommNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DsendRecvNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DstartAllNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcomplexNoLossTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
 
 ## Loss cases
 SET_TESTS_PROPERTIES (DwcIrecvEx2LossTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Error.*MPI_Isend@.*3.*Lost send of rank 3 to rank 0")