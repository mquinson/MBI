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
#Add test case

## simpleCollNoError
addProcessLocalTest ("simpleCollNoError" "simpleCollNoError.c" 2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collCallMissmatchError
addProcessLocalTest ("collCallMissmatchError" "collCallMissmatchError.c" 2
    "A collective mismatch occured [(]The application executes two different collective calls on the same communicator"
    ""
    )


## collRootMissmatchError
addProcessLocalTest ("collRootMissmatchError" "collRootMissmatchError.c" 2
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root"
    ""
    )


## collRootMissmatchErrorGather
addProcessLocalTest ("collRootMissmatchErrorGather" "collRootMissmatchErrorGather.c" 2
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root"
    ""
    )


## collRootMissmatchErrorScatter
addProcessLocalTest ("collRootMissmatchErrorScatter" "collRootMissmatchErrorScatter.c" 2
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root"
    ""
    )


## collRootMissmatchReduceError
addProcessLocalTest ("collRootMissmatchReduceError" "collRootMissmatchReduceError.c" 2
    "Two collective operations that use a root process specified conflicting roots! This collective uses rank .* as root .*The conflicting operation uses rank .* as root"
    ""
    )


## collOpMissmatchReduceError
addProcessLocalTest ("collOpMissmatchReduceError" "collOpMissmatchReduceError.c" 2
    "Two collective calls that use an operation specified conflicting operations! This rank uses the operation"
    ""
    )


## collReduceNoError
addProcessLocalTest ("collReduceNoError" "collReduceNoError.c" 2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collBcastNoError
addProcessLocalTest ("collBcastNoError" "collBcastNoError.c" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collGatherNoError
addProcessLocalTest ("collGatherNoError" "collGatherNoError.c" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collGatherEx2NoError
addProcessLocalTest ("collGatherEx2NoError" "collGatherEx2NoError.c" 5
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collGatherTypeLenError
addProcessLocalTest ("collGatherTypeLenError" "collGatherTypeLenError.c" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length! Each send and receive transfer of a collective call must use equal type signatures.*The first element of the send that did not fit into the receive operation is at .2.[(]MPI_INT[)] in the send type"
    ""
    )


## collGathervNoError
addProcessLocalTest ("collGathervNoError" "collGathervNoError.c" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collGathervTypeLenError
addProcessLocalTest ("collGathervTypeLenError" "collGathervTypeLenError.c" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length! Each send and receive transfer of a collective call must use equal type signatures.*The first element of the send that did not fit into the receive operation is at [(]MPI_INT[)] in the send type"
    ""
    )


## collScatterNoError
addProcessLocalTest ("collScatterNoError" "collScatterNoError.c" 2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collScattervNoError
addProcessLocalTest ("collScattervNoError" "collScattervNoError.c" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collAllgatherNoError
addProcessLocalTest ("collAllgatherNoError" "collAllgatherNoError.c" 2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collAllgathervNoError
addProcessLocalTest ("collAllgathervNoError" "collAllgathervNoError.c" 2
    ""
    "Error;ERROR;Warning;WARNING"
    )
    

## collAllgathervTypeLenError
addProcessLocalTest ("collAllgathervTypeLenError" "collAllgathervTypeLenError.c" 4
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length!.*The first element of .* that did not fit into the .* operation is at [(]MPI_INT[)]*"
    ""
    )


## collAlltoallNoError
addProcessLocalTest ("collAlltoallNoError" "collAlltoallNoError.c" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collAlltoallvNoError
#TODO: there are still warnings in this test that we need to investigate (for Joachim)
addProcessLocalTest ("collAlltoallvNoError" "collAlltoallvNoError.c" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collAllreduceNoError
addProcessLocalTest ("collAllreduceNoError" "collAllreduceNoError.c" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collAlltoallvTypeMatchError
addProcessLocalTest ("collAlltoallvTypeMatchError" "collAlltoallvTypeMatchError.c" 5
    "Two collective calls cause a type mismatch!.*The mismatch occurs at [(]struct[)].*[(]MPI_INT[)] in the send type and at [(]struct[)].*[(]MPI_UNSIGNED[)] in the receive type"
    ""
    )


## collInverseCommNoError
addProcessLocalTest ("collInverseCommNoError" "collInverseCommNoError.c" 5
    ""
    "Error;ERROR;Warning;WARNING"
    )


## collDisjointCommNoError
addProcessLocalTest ("collDisjointCommNoError" "collDisjointCommNoError.c" 7
    ""
    "Error;ERROR;Warning;WARNING"
    )    

#### ----------------- Additional tests from Joachim --------------------
### --- Smal ---
## collAllgatherTypeLenError
addProcessLocalTest ("collAllgatherTypeLenError" "collAllgatherTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collAllgatherNoError2
addProcessLocalTest ("collAllgatherNoError2" "collAllgatherNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collAllgatherNoErrorEx
addProcessLocalTest ("collAllgatherNoErrorEx" "collAllgatherNoErrorEx.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )    

## collAllgathervTypeLenError2
addProcessLocalTest ("collAllgathervTypeLenError2" "collAllgathervTypeLenError2.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )    
    
## collAllgathervNoError2
addProcessLocalTest ("collAllgathervNoError2" "collAllgathervNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collAllreduceTypeLenError
addProcessLocalTest ("collAllreduceTypeLenError" "collAllreduceTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )    

## collAllreduceNoError2
addProcessLocalTest ("collAllreduceNoError2" "collAllreduceNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collAlltoallTypeLenError
addProcessLocalTest ("collAlltoallTypeLenError" "collAlltoallTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )    

## collAlltoallNoError2
addProcessLocalTest ("collAlltoallNoError2" "collAlltoallNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collAlltoallvTypeLenError
addProcessLocalTest ("collAlltoallvTypeLenError" "collAlltoallvTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )    

## collAlltoallvNoError2
addProcessLocalTest ("collAlltoallvNoError2" "collAlltoallvNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

IF (HAVE_MPI_ALLTOALLW)
    ## collAlltoallwTypeLenError
    addProcessLocalTest ("collAlltoallwTypeLenError" "collAlltoallwTypeLenError.cpp" 3
        "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
        ""
        )
        
    ## collAlltoallwNoError
    addProcessLocalTest ("collAlltoallwNoError" "collAlltoallwNoError.cpp" 3
        ""
        "Error;ERROR;Warning;WARNING"
        )    
ENDIF (HAVE_MPI_ALLTOALLW)

## collBcastRootMismatchError
addProcessLocalTest ("collBcastRootMismatchError" "collBcastRootMismatchError.cpp" 3
    "Two collective operations that use a root process specified conflicting roots"
    ""
    )    

## collBcastNoError2
addProcessLocalTest ("collBcastNoError2" "collBcastNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

IF (HAVE_MPI_EXSCAN)
    ## collExscanTypeLenError
    addProcessLocalTest ("collExscanTypeLenError" "collExscanTypeLenError.cpp" 3
        "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
        ""
        )

    ## collExscanNoError
    addProcessLocalTest ("collExscanNoError" "collExscanNoError.cpp" 3
        ""
        "Error;ERROR;Warning;WARNING"
        )    
ENDIF (HAVE_MPI_EXSCAN)    

## collGatherTypeLenError2
addProcessLocalTest ("collGatherTypeLenError2" "collGatherTypeLenError2.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )    

## collGatherNoError2
addProcessLocalTest ("collGatherNoError2" "collGatherNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )    

## collGathervTypeLenError2
addProcessLocalTest ("collGathervTypeLenError2" "collGathervTypeLenError2.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collGathervNoError2
addProcessLocalTest ("collGathervNoError2" "collGathervNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collReduceTypeLenError
addProcessLocalTest ("collReduceTypeLenError" "collReduceTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

IF (HAVE_MPI_REDUCE_SCATTER_BLOCK)
    ## collReduceScatterBlockTypeLenError
    addProcessLocalTest ("collReduceScatterBlockTypeLenError" "collReduceScatterBlockTypeLenError.cpp" 3
        "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
        ""
        )

    ## collReduceScatterBlockNoError
    addProcessLocalTest ("collReduceScatterBlockNoError" "collReduceScatterBlockNoError.cpp" 3
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_REDUCE_SCATTER_BLOCK)
    
IF (HAVE_MPI_REDUCE_SCATTER)
    ## collReduceScatterTypeLenError
    addProcessLocalTest ("collReduceScatterTypeLenError" "collReduceScatterTypeLenError.cpp" 3
        "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
        ""
        )

    ## collReduceScatterNoError
    addProcessLocalTest ("collReduceScatterNoError" "collReduceScatterNoError.cpp" 3
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_REDUCE_SCATTER)

## collReduceNoError2
addProcessLocalTest ("collReduceNoError2" "collReduceNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
## collScanTypeLenError
addProcessLocalTest ("collScanTypeLenError" "collScanTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collScanNoError
addProcessLocalTest ("collScanNoError" "collScanNoError.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collScatterTypeLenError
addProcessLocalTest ("collScatterTypeLenError" "collScatterTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collScatterNoError2
addProcessLocalTest ("collScatterNoError2" "collScatterNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collScattervTypeLenError
addProcessLocalTest ("collScattervTypeLenError" "collScattervTypeLenError.cpp" 3
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collScattervNoError2
addProcessLocalTest ("collScattervNoError2" "collScattervNoError2.cpp" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
### --- Large ---
IF (HAVE_MPI_ALLTOALLW)
    ## collAlltoallwTypeLenErrorLarge
    addProcessLocalTest ("collAlltoallwTypeLenErrorLarge" "collAlltoallwTypeLenErrorLarge.cpp" 8
        "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
        ""
        )
    
    ## collAlltoallwNoError
    #TODO: there are still warnings in this test that we need to investigate (for Joachim)
    addProcessLocalTest ("collAlltoallwNoErrorLarge" "collAlltoallwNoErrorLarge.cpp" 8
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_ALLTOALLW)

## collGathervTypeLenErrorLarge
addProcessLocalTest ("collGathervTypeLenErrorLarge" "collGathervTypeLenErrorLarge.cpp" 8
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collGathervNoErrorLarge
addProcessLocalTest ("collGathervNoErrorLarge" "collGathervNoErrorLarge.cpp" 8
    ""
    "Error;ERROR;Warning;WARNING"
    )

## collScatterNoErrorLarge
addProcessLocalTest ("collScatterNoErrorLarge" "collScatterNoErrorLarge.cpp" 8
    ""
    "Error;ERROR;Warning;WARNING"
    )  

## collScattervTypeLenErrorLarge
addProcessLocalTest ("collScattervTypeLenErrorLarge" "collScattervTypeLenErrorLarge.cpp" 8
    "Two collective calls use [(]datatype,count[)] pairs that span type signatures of different length"
    ""
    )

## collScattervNoErrorLarge
addProcessLocalTest ("collScattervNoErrorLarge" "collScattervNoErrorLarge.cpp" 8
    ""
    "Error;ERROR;Warning;WARNING"
    )
