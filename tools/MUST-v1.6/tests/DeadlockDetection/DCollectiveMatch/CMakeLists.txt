# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for collective matching tests that are part of deadlock detection.
#
# @author Tobias Hilbrich
# @date 1.08.2011
#

###################
#Test cases
SET (tests "simpleCollNoError" "collReduceNoError" "collBcastNoError" "collGatherNoError" "collGatherEx2NoError" "collGathervNoError" "collScatterNoError" "collScattervNoError" 
                "collAllgatherNoError" "collAllgathervNoError" "collAlltoallNoError" "collAlltoallvNoError" "collAllreduceNoError" "collInverseCommNoError" "collDisjointCommNoError" 
                "collAllgatherNoError2" "collAllgatherNoErrorEx" "collAllgathervNoError2" "collAllreduceNoError2" "collAlltoallNoError2" "collAlltoallvNoError2" "collBcastNoError2" 
                "collGatherNoError2" "collGathervNoError2" "collReduceNoError2" "collScanNoError" "collScatterNoError2" "collScattervNoError2" "collGathervNoErrorLarge" 
                "collScatterNoErrorLarge" "collScattervNoErrorLarge" 
                "collCallMissmatchError" "collRootMissmatchError" "collRootMissmatchErrorGather" "collRootMissmatchErrorScatter" "collRootMissmatchReduceError" 
                "collOpMissmatchReduceError" "collGatherTypeLenError" "collGathervTypeLenError" "collAllgathervTypeLenError" "collAlltoallvTypeMatchError" 
                "collAllgatherTypeLenError" "collAllgathervTypeLenError2" "collAllreduceTypeLenError" "collAlltoallTypeLenError" "collAlltoallvTypeLenError" 
                "collBcastRootMismatchError" "collGatherTypeLenError2" "collGathervTypeLenError2" "collReduceTypeLenError" "collScanTypeLenError" 
                "collScatterTypeLenError" "collScattervTypeLenError" "collGathervTypeLenErrorLarge" "collScattervTypeLenErrorLarge"
                )

SET (intratests "collGathervNoError" "collScattervNoError" "collAlltoallvNoError" "collAllgathervNoError2" "collAlltoallvNoError2" "collGathervNoError2" "collScattervNoError2" 
                       "collGathervNoErrorLarge" "collScattervNoErrorLarge" 
                       "collGathervTypeLenError" "collAlltoallvTypeMatchError" "collAlltoallvTypeLenError" "collGathervTypeLenError2" "collScattervTypeLenError" 
                       "collGathervTypeLenErrorLarge" "collScattervTypeLenErrorLarge"
                )

IF (HAVE_MPI_ALLTOALLW)
    SET (tests ${tests} "collAlltoallwNoError" "collAlltoallwNoErrorLarge" "collAlltoallwTypeLenError" "collAlltoallwTypeLenErrorLarge" )
    SET (intratests ${intratests} "collAlltoallwNoError" "collAlltoallwNoErrorLarge" "collAlltoallwTypeLenError" "collAlltoallwTypeLenErrorLarge") 
ENDIF ()

IF (HAVE_MPI_EXSCAN)
    SET (tests ${tests} "collExscanNoError" "collExscanTypeLenError")
ENDIF()

IF (HAVE_MPI_REDUCE_SCATTER_BLOCK)
    SET (tests ${tests} "collReduceScatterBlockNoError" "collReduceScatterBlockTypeLenError")
ENDIF ()

IF (HAVE_MPI_REDUCE_SCATTER)
    SET (tests ${tests} "collReduceScatterNoError" "collReduceScatterTypeLenError")
ENDIF ()

SET (NUMsimpleCollNoError 2)
SET (NUMcollReduceNoError 2)
SET (NUMcollBcastNoError 4)
SET (NUMcollGatherNoError 4)
SET (NUMcollGatherEx2NoError 5)
SET (NUMcollGathervNoError 4)
SET (NUMcollScatterNoError 2)
SET (NUMcollScattervNoError 4)
SET (NUMcollAllgatherNoError 2)
SET (NUMcollAllgathervNoError 2)
SET (NUMcollAlltoallNoError 4)
SET (NUMcollAlltoallvNoError 4)
SET (NUMcollAllreduceNoError 3)
SET (NUMcollInverseCommNoError 5)
SET (NUMcollDisjointCommNoError 7)
SET (NUMcollAllgatherNoError2 3)
SET (NUMcollAllgatherNoErrorEx 3)
SET (NUMcollAllgathervNoError2 3)
SET (NUMcollAllreduceNoError2 3)
SET (NUMcollAlltoallNoError2 3)
SET (NUMcollAlltoallvNoError2 3)
SET (NUMcollBcastNoError2 3)
SET (NUMcollGatherNoError2 3)
SET (NUMcollGathervNoError2 3)
SET (NUMcollReduceNoError2 3)
SET (NUMcollScanNoError 3)
SET (NUMcollScatterNoError2 3)
SET (NUMcollScattervNoError2 3)
SET (NUMcollGathervNoErrorLarge 8)
SET (NUMcollScatterNoErrorLarge 8)
SET (NUMcollScattervNoErrorLarge 8)
SET (NUMcollAlltoallwNoError 3)
SET (NUMcollAlltoallwNoErrorLarge 8)
SET (NUMcollExscanNoError 3)
SET (NUMcollReduceScatterBlockNoError 3)
SET (NUMcollReduceScatterNoError 3)
SET (NUMcollCallMissmatchError 2)
SET (NUMcollRootMissmatchError 2)
SET (NUMcollRootMissmatchErrorGather 2)
SET (NUMcollRootMissmatchErrorScatter 2)
SET (NUMcollRootMissmatchReduceError 2)
SET (NUMcollOpMissmatchReduceError 2) 
SET (NUMcollGatherTypeLenError 3)
SET (NUMcollGathervTypeLenError 3)
SET (NUMcollAllgathervTypeLenError 4)
SET (NUMcollAlltoallvTypeMatchError 5)
SET (NUMcollAllgatherTypeLenError 3)
SET (NUMcollAllgathervTypeLenError2 3)
SET (NUMcollAllreduceTypeLenError 3)
SET (NUMcollAlltoallTypeLenError 3)
SET (NUMcollAlltoallvTypeLenError 3)
SET (NUMcollBcastRootMismatchError 3)
SET (NUMcollGatherTypeLenError2 3)
SET (NUMcollGathervTypeLenError2 3)
SET (NUMcollReduceTypeLenError 3)
SET (NUMcollScanTypeLenError 3)
SET (NUMcollScatterTypeLenError 3)
SET (NUMcollScattervTypeLenError 3)
SET (NUMcollGathervTypeLenErrorLarge 8)
SET (NUMcollScattervTypeLenErrorLarge 8) 
SET (NUMcollAlltoallwTypeLenError 3)
SET (NUMcollAlltoallwTypeLenErrorLarge 8)
SET (NUMcollExscanTypeLenError 3)
SET (NUMcollReduceScatterBlockTypeLenError 3)
SET (NUMcollReduceScatterTypeLenError 3)

FOREACH (test ${tests})
    SET (source "")
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
        SET (source "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
    ELSE (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
        SET (source "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.cpp")
    ENDIF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
        
    addTestExecutable ("D${test}" ${source})
        
    SET (NUM_INTERMEDIATE_NODES 2)
    IF (NUM_INTERMEDIATE_NODES GREATER ${NUM${test}})
        SET (NUM_INTERMEDIATE_NODES ${NUM${test}})
    ENDIF()
        
    configureTest (
        ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml
        "CollectiveCondition:MUST_CollectiveCondition;FinalizeNotify:MUST_Base"
        ""
        "DCollectiveMatchRoot:MUST_DistributedDeadlock;MsgLoggerHtml:MUST_Base;MsgLoggerStdOut:MUST_Base"
        ${NUM${test}}
        ${NUM_INTERMEDIATE_NODES}
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

## No error cases    
SET_TESTS_PROPERTIES (DsimpleCollNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollGatherNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollReduceNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollBcastNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollGatherEx2NoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollGathervNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScatterNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScattervNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllgatherNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllgathervNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAlltoallNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAlltoallvNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllreduceNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollInverseCommNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollDisjointCommNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllgatherNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllgatherNoErrorExTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllgathervNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAllreduceNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAlltoallNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAlltoallvNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollBcastNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollGatherNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollGathervNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollReduceNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScanNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScatterNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScattervNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollGathervNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScatterNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollScattervNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

IF (HAVE_MPI_ALLTOALLW)
SET_TESTS_PROPERTIES (DcollAlltoallwNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DcollAlltoallwNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
ENDIF ()

IF (HAVE_MPI_EXSCAN)
SET_TESTS_PROPERTIES (DcollExscanNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
ENDIF()

IF (HAVE_MPI_REDUCE_SCATTER_BLOCK)
SET_TESTS_PROPERTIES (DcollReduceScatterBlockNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
ENDIF ()

IF (HAVE_MPI_REDUCE_SCATTER)
SET_TESTS_PROPERTIES (DcollReduceScatterNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
ENDIF ()
    
## Error cases
SET_TESTS_PROPERTIES (DcollCallMissmatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A collective mismatch occured [(]The application executes two different collective calls on the same communicator")
SET_TESTS_PROPERTIES (DcollRootMissmatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root")
SET_TESTS_PROPERTIES (DcollRootMissmatchErrorGatherTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root")
SET_TESTS_PROPERTIES (DcollRootMissmatchErrorScatterTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root")
SET_TESTS_PROPERTIES (DcollRootMissmatchReduceErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root")
SET_TESTS_PROPERTIES (DcollOpMissmatchReduceErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* that use an operation specified conflicting operations! This rank uses the operation")
SET_TESTS_PROPERTIES (DcollGatherTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length!")
SET_TESTS_PROPERTIES (DcollGathervTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length!")
SET_TESTS_PROPERTIES (DcollAllgathervTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length!")
SET_TESTS_PROPERTIES (DcollAlltoallvTypeMatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective calls cause a type mismatch!.*occurs at [(]struct[)].*")
SET_TESTS_PROPERTIES (DcollAllgatherTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollAllgathervTypeLenError2Test PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollAllreduceTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollAlltoallTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollAlltoallvTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
   "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length" )
SET_TESTS_PROPERTIES (DcollBcastRootMismatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective operations that use a root process specified conflicting roots")    
SET_TESTS_PROPERTIES (DcollGatherTypeLenError2Test PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollGathervTypeLenError2Test PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollReduceTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
   "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length" )
SET_TESTS_PROPERTIES (DcollScanTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollScatterTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollScattervTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollGathervTypeLenErrorLargeTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollScattervTypeLenErrorLargeTest PROPERTIES PASS_REGULAR_EXPRESSION 
   "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
   
IF (HAVE_MPI_ALLTOALLW)
SET_TESTS_PROPERTIES (DcollAlltoallwTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DcollAlltoallwTypeLenErrorLargeTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
ENDIF ()

IF (HAVE_MPI_EXSCAN)
SET_TESTS_PROPERTIES (DcollExscanTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
ENDIF (HAVE_MPI_EXSCAN)    

IF (HAVE_MPI_REDUCE_SCATTER_BLOCK)
SET_TESTS_PROPERTIES (DcollReduceScatterBlockTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
ENDIF (HAVE_MPI_REDUCE_SCATTER_BLOCK)
    
IF (HAVE_MPI_REDUCE_SCATTER)
SET_TESTS_PROPERTIES (DcollReduceScatterTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
ENDIF (HAVE_MPI_REDUCE_SCATTER)

#####################################################
############## Tests using intra layer communication
#####################################################
FOREACH (test ${intratests})
    SET (source "")
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
        SET (source "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
    ELSE (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
        SET (source "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.cpp")
    ENDIF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CollectiveMatch/${test}.c")
        
    addTestExecutable ("DIntra${test}" ${source})
        
    SET (NUM_INTERMEDIATE_NODES 2)
    IF (NUM_INTERMEDIATE_NODES GREATER ${NUM${test}})
        SET (NUM_INTERMEDIATE_NODES ${NUM${test}})
    ENDIF()
        
    configureTestIntra (
        ${CMAKE_CURRENT_BINARY_DIR}/DIntra${test}layout.xml
        "CollectiveCondition:MUST_CollectiveCondition"
        ""
        "DCollectiveMatchRoot:MUST_DistributedDeadlock;MsgLoggerHtml:MUST_Base;MsgLoggerStdOut:MUST_Base"
        ${NUM${test}}
        ${NUM_INTERMEDIATE_NODES}
        )
    
    ADD_TEST(NAME DIntra${test}Test COMMAND
        bash ${MUSTRUN} 
            --must:timeout ${MUSTRUN_TIMEOUT}
            --must:mpiexec ${MPIEXEC} 
            --must:np ${MPIEXEC_NUMPROC_FLAG} 
            --must:layout ${CMAKE_CURRENT_BINARY_DIR}/DIntra${test}layout.xml 
            --must:temp tempDIntra${test}
            --must:clean
            ${MPIEXEC_NUMPROC_FLAG} ${NUM${test}}  "$<TARGET_FILE:DIntra${test}>" 
            )
ENDFOREACH(test)

## No error cases    
SET_TESTS_PROPERTIES (DIntracollGathervNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollScattervNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollAlltoallvNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollAlltoallvNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollGathervNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollScattervNoError2Test PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollGathervNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollScattervNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

IF (HAVE_MPI_ALLTOALLW)
SET_TESTS_PROPERTIES (DIntracollAlltoallwNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
SET_TESTS_PROPERTIES (DIntracollAlltoallwNoErrorLargeTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
ENDIF ()

## Error cases
SET_TESTS_PROPERTIES (DIntracollGathervTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length!")
SET_TESTS_PROPERTIES (DIntracollAlltoallvTypeMatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective calls cause a type mismatch!.*occurs at [(]struct[)].*")
SET_TESTS_PROPERTIES (DIntracollAlltoallvTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
   "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length" )
SET_TESTS_PROPERTIES (DIntracollGathervTypeLenError2Test PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DIntracollScattervTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DIntracollGathervTypeLenErrorLargeTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DIntracollScattervTypeLenErrorLargeTest PROPERTIES PASS_REGULAR_EXPRESSION 
   "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
   
IF (HAVE_MPI_ALLTOALLW)
SET_TESTS_PROPERTIES (DIntracollAlltoallwTypeLenErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
SET_TESTS_PROPERTIES (DIntracollAlltoallwTypeLenErrorLargeTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective .* use [(]datatype,count[)] pairs that span type signatures of different length")
ENDIF ()
