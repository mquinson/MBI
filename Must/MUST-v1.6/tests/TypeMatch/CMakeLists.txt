# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for type matching tests.
#
# @author Tobias Hilbrich
# @date 26.07.2011
#

##################
#Recurse into sub directories

###################
#Add test case
SET (tests 
    "MatchSimpleNoError" 
    "MatchSimpleLengthError" 
    "MatchSimpleLengthNoError" 
    "MatchSimpleTypeMissmatchError" 
    "MatchContiNoError" 
    "MatchContiLengthError" 
    "MatchContiMissmatchError" 
    "MatchMultipleContiMissmatchError" 
    "MatchMultipleContiMissmatchNoError"
    "MatchIndexedNoError"
    "MatchIndexedLengthError"
    "MatchHindexedNoError"
    "MatchHindexedLengthError"
    "MatchVectorNoError"
    "MatchVectorLengthError"
    "MatchHvectorNoError"
    "MatchHvectorLengthError"
    "MatchStructNoError"
    "sendRecvStructMismatch"
    "MatchStructMismatchError")

IF (HAVE_MPI_TYPE_CREATE_INDEXED_BLOCK)
    SET (tests ${tests} "MatchIndexedBlockNoError")
    SET (tests ${tests} "MatchIndexedBlockLengthError")
ENDIF (HAVE_MPI_TYPE_CREATE_INDEXED_BLOCK)

#TODO add test cases for resized, subarray, and darray

SET (NUMMatchSimpleNoError 2)
SET (NUMMatchSimpleLengthError 2)
SET (NUMMatchSimpleLengthNoError 2)
SET (NUMMatchSimpleTypeMissmatchError 2)
SET (NUMMatchContiNoError 2)
SET (NUMMatchContiLengthError 2)
SET (NUMMatchContiMissmatchError 2)
SET (NUMMatchMultipleContiMissmatchError 2)
SET (NUMMatchMultipleContiMissmatchNoError 2)
SET (NUMMatchIndexedNoError 2)
SET (NUMMatchIndexedLengthError 2)
SET (NUMMatchHindexedNoError 2)
SET (NUMMatchHindexedLengthError 2)
SET (NUMMatchVectorNoError 2)
SET (NUMMatchVectorLengthError 2)
SET (NUMMatchHvectorNoError 2)
SET (NUMMatchHvectorLengthError 2)
SET (NUMMatchStructNoError 2)
SET (NUMMatchStructMismatchError 2)
SET (NUMsendRecvStructMismatch 2)
SET (NUMMatchIndexedBlockNoError 2)
SET (NUMMatchIndexedBlockLengthError 2)
 
FOREACH (test ${tests})
    addTestExecutable ("${test}" "${test}.c")
    
    configureTest (
        ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml
        "WcUpdate:MUST_Deadlock;OpSplitter:MUST_Deadlock;FinalizeNotify:MUST_Base"
        ""
        "P2PMatch:MUST_Deadlock;MsgLoggerHtml:MUST_Base;MsgLoggerStdOut:MUST_Base"
        ${NUM${test}}
        1
        )
    
    ADD_TEST(NAME ${test}Test COMMAND
        bash ${MUSTRUN} 
            --must:timeout ${MUSTRUN_TIMEOUT}
            --must:mpiexec ${MPIEXEC} 
            --must:np ${MPIEXEC_NUMPROC_FLAG} 
            --must:layout ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml 
            --must:temp temp${test}
            --must:clean
            ${MPIEXEC_NUMPROC_FLAG} ${NUM${test}}  $<TARGET_FILE:${test}> 
            )
ENDFOREACH(test)

## MatchSimpleNoError    
SET_TESTS_PROPERTIES (MatchSimpleNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
  
## MatchSimpleLengthError  
SET_TESTS_PROPERTIES (MatchSimpleLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchSimpleLengthNoError    
SET_TESTS_PROPERTIES (MatchSimpleLengthNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

## MatchSimpleTypeMissmatchError  
SET_TESTS_PROPERTIES (MatchSimpleTypeMissmatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A send and a receive operation use datatypes that do not match!")    
    
## MatchContiNoError  
SET_TESTS_PROPERTIES (MatchContiNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    

## MatchContiLengthError  
SET_TESTS_PROPERTIES (MatchContiLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchContiMissmatchError  
SET_TESTS_PROPERTIES (MatchContiMissmatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A send and a receive operation use datatypes that do not match!")    

## MatchIndexedNoError  
SET_TESTS_PROPERTIES (MatchIndexedNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    

## MatchIndexedLengthError  
SET_TESTS_PROPERTIES (MatchIndexedLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")
    
## MatchHindexedNoError  
SET_TESTS_PROPERTIES (MatchHindexedNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
    
## MatchHindexedLengthError  
SET_TESTS_PROPERTIES (MatchHindexedLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchVectorNoError  
SET_TESTS_PROPERTIES (MatchVectorNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

## MatchVectorLengthError  
### TODO only caught sometimes (depending on matching order), here both types have equal size and extent, but different basic type counts, we need a bugfix/extension in the I_Datatype implementation here
SET_TESTS_PROPERTIES (MatchVectorLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchHvectorNoError  
SET_TESTS_PROPERTIES (MatchHvectorNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

## MatchHvectorLengthError  
SET_TESTS_PROPERTIES (MatchHvectorLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")
    
## MatchStructNoError  
SET_TESTS_PROPERTIES (MatchStructNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
    
## MatchStructMismatchError  
### TODO the long error position is broken it says (STRUCT)[0][2](MPI_LONG) twice but should be (STRUCT)[2][0](MPI_{LONG|INT}) (once INT once LONG)
SET_TESTS_PROPERTIES (MatchStructMismatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A send and a receive operation use datatypes that do not match!")

## MatchIndexedBlock*  
IF (HAVE_MPI_TYPE_CREATE_INDEXED_BLOCK)
     SET_TESTS_PROPERTIES (MatchIndexedBlockNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
        "Error;ERROR;Warning;WARNING")
        
     SET_TESTS_PROPERTIES (MatchIndexedBlockLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
        "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")
ENDIF (HAVE_MPI_TYPE_CREATE_INDEXED_BLOCK)
    
###################################
###### Distributed version of Type matching
###################################
FOREACH (test ${tests})
    addTestExecutable ("D${test}" "${test}.c")
    
    configureTestIntra (
        ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml
        "WcUpdate:MUST_Deadlock;OpSplitter:MUST_Deadlock"
        "DP2PMatch:MUST_DistributedDeadlock"
        "MsgLoggerHtml:MUST_Base;MsgLoggerStdOut:MUST_Base"
        ${NUM${test}}
        ${NUM${test}}
        )
    
    ADD_TEST(NAME D${test}Test COMMAND
        bash ${MUSTRUN} 
            --must:timeout ${MUSTRUN_TIMEOUT}
            --must:mpiexec ${MPIEXEC} 
            --must:np ${MPIEXEC_NUMPROC_FLAG} 
            --must:layout ${CMAKE_CURRENT_BINARY_DIR}/${test}layout.xml 
            --must:temp tempD${test}
            --must:clean
            ${MPIEXEC_NUMPROC_FLAG} ${NUM${test}}  $<TARGET_FILE:D${test}> 
            )
ENDFOREACH(test)

## MatchSimpleNoError    
SET_TESTS_PROPERTIES (DMatchSimpleNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
  
## MatchSimpleLengthError  
SET_TESTS_PROPERTIES (DMatchSimpleLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchSimpleLengthNoError    
SET_TESTS_PROPERTIES (DMatchSimpleLengthNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

## MatchSimpleTypeMissmatchError  
SET_TESTS_PROPERTIES (DMatchSimpleTypeMissmatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A send and a receive operation use datatypes that do not match!")    
    
## MatchContiNoError  
SET_TESTS_PROPERTIES (DMatchContiNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    

## MatchContiLengthError  
SET_TESTS_PROPERTIES (DMatchContiLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchContiMissmatchError  
SET_TESTS_PROPERTIES (DMatchContiMissmatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A send and a receive operation use datatypes that do not match!")    

## MatchIndexedNoError  
SET_TESTS_PROPERTIES (DMatchIndexedNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    
    
## MatchIndexedLengthError  
SET_TESTS_PROPERTIES (DMatchIndexedLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchHindexedNoError  
SET_TESTS_PROPERTIES (DMatchHindexedNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    

## MatchHindexedLengthError  
SET_TESTS_PROPERTIES (DMatchHindexedLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")
    
## MatchVectorNoError  
SET_TESTS_PROPERTIES (DMatchVectorNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
    
## MatchVectorLengthError  
SET_TESTS_PROPERTIES (DMatchVectorLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")

## MatchHvectorNoError  
SET_TESTS_PROPERTIES (DMatchHvectorNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
    
## MatchHvectorLengthError  
SET_TESTS_PROPERTIES (DMatchHvectorLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")
    
## MatchStructNoError  
SET_TESTS_PROPERTIES (DMatchStructNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
    
## MatchStructMismatchError  
SET_TESTS_PROPERTIES (DMatchStructMismatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "A send and a receive operation use datatypes that do not match!")

## MatchIndexedBlock*  
IF (HAVE_MPI_TYPE_CREATE_INDEXED_BLOCK)
     SET_TESTS_PROPERTIES (DMatchIndexedBlockNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
        "Error;ERROR;Warning;WARNING")
        
    SET_TESTS_PROPERTIES (DMatchIndexedBlockLengthErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
        "A receive operation uses a [(]datatype,count[)] pair that can not hold the data transfered by the send it matches")
ENDIF (HAVE_MPI_TYPE_CREATE_INDEXED_BLOCK)