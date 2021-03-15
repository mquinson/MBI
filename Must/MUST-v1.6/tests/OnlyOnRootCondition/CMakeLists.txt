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

##################
#Recurse into sub directories
##Not needed

###################
#Add test case

## OnlyOnRoot_GatherWarning.  (Warning)
addProcessLocalTest ("OnlyOnRoot_GatherWarning" "OnlyOnRoot_GatherWarning.cpp" 2 
    "Warning:.*MPI_Gather@.*0: Argument 5 [(]recvcount[)] is zero, which is correct but unusual!"
    "Warning:.*MPI_Gather@.*1: Argument 5 [(]recvcount[)] is zero, which is correct but unusual!"
    )

## OnlyOnRoot_GathervWarning.  (Warning)
addProcessLocalTest ("OnlyOnRoot_GathervWarning" "OnlyOnRoot_GathervWarning.cpp" 2 
    "Warning:.*MPI_Gatherv@.*0: Argument 5 [(]recvcounts[)] is an array that contains zero value"
    "Warning:.*MPI_Gatherv@,*1: Argument 5 [(]recvcounts[)] is an array that contains zero value"
    )

## OnlyOnRoot_ScatterWarning.  (Warning)
addProcessLocalTest ("OnlyOnRoot_ScatterWarning" "OnlyOnRoot_ScatterWarning.cpp" 2 
    "Warning:.*MPI_Scatter@.*1: Argument 2 [(]sendcount[)] is zero,"
    "Warning:.*MPI_Scatter@.*0: Argument 2 [(]sendcount[)] is zero,"
    )

## OnlyOnRoot_noWarning.  (no-Warning)
addProcessLocalTest ("OnlyOnRoot_noWarning" "OnlyOnRoot_noWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

## OnlyOnRoot_IntegrityNegativeArrayError.  (Error)
addProcessLocalTest ("OnlyOnRoot_IntegrityNegativeArrayError" "OnlyOnRoot_IntegrityNegativeArrayError.cpp" 2 
    "Error:.*MPI_Gatherv@.*0: Argument 6 [(]displs[)] has to be an array of non-negative integers"
    "Error:.*MPI_Gatherv@.*1: Argument 6 [(]displs[)] has to be an array of non-negative integers"
    )
    
## OnlyOnRoot_IntegrityNullError.  (Error)
addProcessLocalTest ("OnlyOnRoot_IntegrityNullError" "OnlyOnRoot_IntegrityNullError.cpp" 2 
    "Error:.*MPI_Gatherv@.*0: Argument 6 [(]displs[)] is a NULL pointer where an allocated memory region with a size of"
    "Error:.*MPI_Gatherv@.*1: Argument 6 [(]displs[)] is a NULL pointer where an allocated memory region with a size of"
    )
    
## OnlyOnRoot_IntegrityNullCommSizeError.  (Error)
addProcessLocalTest ("OnlyOnRoot_IntegrityNullCommSizeError" "OnlyOnRoot_IntegrityNullCommSizeError.cpp" 2 
    "Error:.*MPI_Gatherv@.*0: Argument 5 [(]recvcounts[)] is a NULL pointer where an allocated memory region"
    "Error:.*MPI_Gatherv@.*1: Argument 5 [(]recvcounts[)] is a NULL pointer where an allocated memory region"
    )
    
## OnlyOnRoot_IntegrityNullNotMPIBottomError.  (Error)
addProcessLocalTest ("OnlyOnRoot_IntegrityNullNotMPIBottomError" "OnlyOnRoot_IntegrityNullNotMPIBottomError.cpp" 2 
    "Error:.*MPI_Gatherv@.*0: Argument 4 [(]recvbuf[)] is a NULL pointer where an allocated memory region with a size of"
    "Error:.*MPI_Gatherv@.*1: Argument 4 [(]recvbuf[)] is a NULL pointer where an allocated memory region with a size of"
    )

## OnlyOnRoot_noError.  (no-Error)
addProcessLocalTest ("OnlyOnRoot_noError" "OnlyOnRoot_noError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )