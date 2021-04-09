# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for DatatypeTrack test.
#
# @author Tobias Hilbrich
# @date 23.02.2011
#

IF (NOT GTI_ENABLE_FORTRAN)
    RETURN()
ENDIF ()


##################
#Recurse into sub directories
##Not needed

###################
#Add test case

SET (checkList
     IntegerChecks:MUST_BasicChecks; 
     BasicIntegrities:MUST_BasicChecks; 
     CommChecks:MUST_CommChecks;
     CommPredefs:MUST_Resource;
     DatatypeChecks:MUST_DatatypeChecks;
     GroupChecks:MUST_GroupChecks; 
     LeakChecks:MUST_LeakChecks; 
     OperationChecks:MUST_BasicChecks;
     OverlapChecks:MUST_OverlapChecks;
     RequestChecks:MUST_RequestChecks 
     CollectiveMatch:MUST_Deadlock; 
     CollectiveCondition:MUST_CollectiveCondition; 
     RequestCondition:MUST_RequestCondition;
     FinalizeNotify:MUST_Base;
     DatatypePredefs:MUST_Resource;
     OpSplitter:MUST_Deadlock;
     WcUpdate:MUST_Deadlock
     )
     
OPTION(ENABLE_Fortran_TESTS "Enables Fortran language tests" ${ENABLE_TESTS})
OPTION(ENABLE_Fortran_MIXED_TESTS "Enables Fortran-C mixed language tests" ${ENABLE_TESTS})
     
#Fortran Mixed code
IF (ENABLE_Fortran_MIXED_TESTS AND ENABLE_Fortran_TESTS)
    addProcessLocalTestMixed ("fortran_calls_c" "fortran_calls_c.f" "fortran_calls_c_lib.c" 2 "fortran" 
        "${checkList}"
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )
    
    addProcessLocalTestMixed ("c_calls_fortran" "c_calls_fortran.c" "c_calls_fortran_lib.f" 2 "C" 
        "${checkList}"
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (ENABLE_Fortran_MIXED_TESTS AND ENABLE_Fortran_TESTS)
    
#fortran only
IF (ENABLE_Fortran_TESTS)
    ##simple_fortran   (NO-ERROR)
    addProcessLocalTest ("simple_fortran" "simple_fortran.f" 2 
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )

    ##fortran_waitall   (NO-ERROR)
    addProcessLocalTest ("fortran_waitall" "fortran_waitall.f" 2 
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )

    
    ##datatype_fortran   (NO-ERROR)
    addProcessLocalTest ("datatype_fortran" "datatype_fortran.f" 2 
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )
        
    ##collective_fortran   (NO-ERROR)
    addProcessLocalTest ("collective_fortran" "collective_fortran.f" 2 
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )
        
    ##communicator_fortran   (NO-ERROR)
    addProcessLocalTest ("communicator_fortran" "communicator_fortran.f" 2 
        "Hello, I am *0 *of *2.*Signing off, rank *0"
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (ENABLE_Fortran_TESTS)