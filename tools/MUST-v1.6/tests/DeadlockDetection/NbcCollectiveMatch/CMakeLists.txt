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
# @date 10.02.2015
#

###################
#Test cases
SET (tests "nbcCollIbarrierNoError" "nbcCollIbarrierAvoidDlNoError" "nbcCollIreduceWaitAnyNoError" "nbcCollBlockingBarrierMismatch" "nbcCollAlltoallvTypeMatchError" "nbcCollIbarrierWaitDl" "nbcCollIbarrierWaitP2PDl" "nbcCollIbarrierTestBgWaitP2PDl" "nbcCollIbarrierWaitsomeDl" "nbcCollIbarrierWaitallP2PDl" "nbcCollIreduceBufferOverlapError" "nbcCollIbcastStrideBufferNoError" "nbcCollIbcastStrideBuffer2NoError")

SET (NUMnbcCollIbarrierNoError 3)
SET (NUMnbcCollIbarrierAvoidDlNoError 3)
SET (NUMnbcCollIreduceWaitAnyNoError 4)
SET (NUMnbcCollBlockingBarrierMismatch 5)
SET (NUMnbcCollAlltoallvTypeMatchError 4)
SET (NUMnbcCollIbarrierWaitDl 3)
SET (NUMnbcCollIbarrierWaitP2PDl 3)
SET (NUMnbcCollIbarrierTestBgWaitP2PDl 4)
SET (NUMnbcCollIbarrierWaitsomeDl 5)
SET (NUMnbcCollIbarrierWaitallP2PDl 3)
SET (NUMnbcCollIreduceBufferOverlapError 4)
SET (NUMnbcCollIbcastStrideBufferNoError 3)
SET (NUMnbcCollIbcastStrideBuffer2NoError 2)

FOREACH (test ${tests})
    SET (source "")
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${test}.c")
        SET (source "${CMAKE_CURRENT_SOURCE_DIR}/${test}.c")
    ELSE ()
        SET (source "${CMAKE_CURRENT_SOURCE_DIR}/${test}.cpp")
    ENDIF ()
        
    addTestExecutable ("D${test}" ${source})
                    
    ADD_TEST(NAME D${test}Test COMMAND
        bash ${MUSTRUN} 
            --must:timeout ${MUSTRUN_TIMEOUT}
            --must:mpiexec ${MPIEXEC} 
            --must:np ${MPIEXEC_NUMPROC_FLAG}
            --must:fanin 2  
            --must:temp temp${test}
            --must:clean
            --must:output stdout
            ${MPIEXEC_NUMPROC_FLAG} ${NUM${test}}  $<TARGET_FILE:D${test}> 
            )
ENDFOREACH(test)

## No error cases    
SET_TESTS_PROPERTIES (DnbcCollIbarrierNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

SET_TESTS_PROPERTIES (DnbcCollIbarrierAvoidDlNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")
    
SET_TESTS_PROPERTIES (DnbcCollIreduceWaitAnyNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")

SET_TESTS_PROPERTIES (DnbcCollIbcastStrideBufferNoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    

SET_TESTS_PROPERTIES (DnbcCollIbcastStrideBuffer2NoErrorTest PROPERTIES FAIL_REGULAR_EXPRESSION 
    "Error;ERROR;Warning;WARNING")    
    
## Error cases
SET_TESTS_PROPERTIES (DnbcCollBlockingBarrierMismatchTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The application matches a blocking collective .* with a non-blocking collective .*!")

SET_TESTS_PROPERTIES (DnbcCollAlltoallvTypeMatchErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "Two collective calls cause a type mismatch! This call sends data to the call in reference 1")
    
SET_TESTS_PROPERTIES (DnbcCollIbarrierWaitDlTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Wait")

SET_TESTS_PROPERTIES (DnbcCollIbarrierWaitP2PDlTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Wait")    

SET_TESTS_PROPERTIES (DnbcCollIbarrierTestBgWaitP2PDlTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Wait")
    
SET_TESTS_PROPERTIES (DnbcCollIbarrierWaitsomeDlTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitsome")
    
SET_TESTS_PROPERTIES (DnbcCollIbarrierWaitallP2PDlTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall")
    
SET_TESTS_PROPERTIES (DnbcCollIreduceBufferOverlapErrorTest PROPERTIES PASS_REGULAR_EXPRESSION 
    "The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation")    
    