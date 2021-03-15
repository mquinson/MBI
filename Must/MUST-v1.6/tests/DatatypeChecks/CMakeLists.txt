# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for RequestChecks test.
#
# @author Tobias Hilbrich,Joachim Protze
# @date 23.05.2011
#

##################
#Recurse into sub directories
##Not needed

###################
#Add test cases

##RequestNoError   (NO-ERROR)
# addProcessLocalTest ("DatatypeNoError" "DatatypeNoError.cpp" 1 
#     ""
#     ""
# #     "Error;ERROR;Warning;WARNING"
#     )

## Commit of MpiDatatypeNull
addProcessLocalTest ("NotValidForCommit_MpiDatatypeNull" "NotValidForCommit_MpiDatatypeNull.cpp" 1 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error.*call MPI_Type_commit.*Argument 1.*is MPI_DATATYPE_NULL"
    ""
    )

## Commit of Predefined
addProcessLocalTest ("NotValidForCommit_Predefined" "NotValidForCommit_Predefined.cpp" 1 
    #Warning: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is predefined, there is no need to commit it
    "Warning.*call MPI_Type_commit.*Argument 1.*is predefined"
    ""
    )

## ReCommit of commited datatype
addProcessLocalTest ("NotValidForCommit_Recommit" "NotValidForCommit_Recommit.cpp" 1 
    #Warning: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is already commited, there is no need to commit it again!
    "Warning.*call MPI_Type_commit.*Argument 1.*is already commited"
    ""
    )

## Commit of undefined datatype
addProcessLocalTest ("NotValidForCommit_Undefined" "NotValidForCommit_Undefined.cpp" 1 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is an unknown datatype (neither a predefined nor a user defined datatype)!
    "Error.*call MPI_Type_commit.*Argument 1.*is an unknown datatype"
    ""
    )

## Create with MpiDatatypeNull
addProcessLocalTest ("NotValidForCreate_MpiDatatypeNull" "NotValidForCreate_MpiDatatypeNull.cpp" 1 
    "Error.*call MPI_Type_contiguous.*Argument 2.*is MPI_DATATYPE_NULL"
    ""
    )

## Create with undefined datatype
addProcessLocalTest ("NotValidForCreate_Undefined" "NotValidForCreate_Undefined.cpp" 1 
    "Error.*call MPI_Type_contiguous.*Argument 2.*is an unknown datatype"
    ""
    )

## struct create with MpiDatatypeNull
addProcessLocalTest ("ArrayNotValidForCreate_MpiDatatypeNull" "ArrayNotValidForCreate_MpiDatatypeNull.cpp" 1 
    "Error.*call MPI_Type_struct.*Element.*Array-Argument 4.*is MPI_DATATYPE_NULL"
    ""
    )

## struct create with undefined datatype
addProcessLocalTest ("ArrayNotValidForCreate_Undefined" "ArrayNotValidForCreate_Undefined.cpp" 1 
    "Error.*call MPI_Type_struct.*Element.*Array-Argument 4.*is an unknown datatype"
    ""
    )

## Communication with MpiDatatypeNull
addProcessLocalTest ("NotValidForCommunication_MpiDatatypeNull" "NotValidForCommunication_MpiDatatypeNull.cpp" 1 
    "Error.*call MPI_Bcast.*Argument 3.*is MPI_DATATYPE_NULL"
    ""
    )

## Communication with undefined datatype
addProcessLocalTest ("NotValidForCommunication_Undefined" "NotValidForCommunication_Undefined.cpp" 1 
    "Error.*call MPI_Bcast.*Argument 3.*is an unknown datatype"
    ""
    )

## Communication with undefined datatype
addProcessLocalTest ("NotValidForCommunication_NotCommited" "NotValidForCommunication_NotCommited.cpp" 1 
    "Error.*call MPI_Bcast.*Argument 3.*is not commited for transfer"
    ""
    )

################################
## No error Check
## Creation, Commit, Communication with no error
addProcessLocalTest ("NotValidForCxxxx_NoErRoR" "NotValidForCxxxx_NoErRoR.cpp" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )
