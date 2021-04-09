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
# @author Tobias Hilbrich,Mathias Korepkat
# @date 5.04.2011
#

##################
#Recurse into sub directories
##Not needed

###################
#Add test cases

##RequestNoError   (NO-ERROR)
addProcessLocalTest ("RequestNoError" "RequestNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##RequestNotKnownError    
addProcessLocalTest ("RequestNotKnownError" "RequestNotKnownError.cpp" 2 
    "Error:.*Argument.*[(]request[)].*is a unknown request"
    ""
    )

##RequestNullNoWarning    
addProcessLocalTest ("RequestNullNoWarning" "RequestNullNoWarning.cpp" 2 
    ""
   "Error;ERROR;Warning;WARNING"
    )
    
##RequestNullWarning    
addProcessLocalTest ("RequestNullWarning" "RequestNullWarning.cpp" 2 
    "Warning:.*Argument.*[(]request[)].*is MPI_REQUEST_NULL was this intended?"
    ""
    )
    
##RequestInActiveNoWarning    
addProcessLocalTest ("RequestInActiveNoWarning" "RequestInActiveNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##RequestInActiveWarning    
addProcessLocalTest ("RequestInActiveWarning" "RequestInActiveWarning.cpp" 2 
    "Warning:.*Argument.*[(]request[)].*activePersistent point-to-point request created at reference"
    ""
    )
    
##RequestNotKnownArrayNoError    
addProcessLocalTest ("RequestNotKnownArrayNoError" "RequestNotKnownArrayNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##RequestNotKnownArrayError    
addProcessLocalTest ("RequestNotKnownArrayError" "RequestNotKnownArrayError.cpp" 2 
    "Error:.*Argument.*[(]array_of_requests[)].*the following entries are unknown requests:"
    ""
    )
    
##RequestNullOrInActiveArrayNoWarning    
addProcessLocalTest ("RequestNullOrInActiveArrayNoWarning" "RequestNullOrInActiveArrayNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##RequestNullOrInActiveArrayWarning    
addProcessLocalTest ("RequestNullOrInActiveArrayWarning" "RequestNullOrInActiveArrayWarning.cpp" 2 
    "Warning:.*Argument.*[(]array_of_requests[)].*array of requests where all request are either in-active or null"
    ""
    )
    
##RequestNullNoError    
addProcessLocalTest ("RequestNullNoError" "RequestNullNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##RequestNullError    
addProcessLocalTest ("RequestNullError" "RequestNullError.cpp" 2 
    "Error:.*Argument.*[(]request[)].* is MPI_REQUEST_NULL!"
    ""
    )

##RequestNullArrayNoError    
addProcessLocalTest ("RequestNullArrayNoError" "RequestNullArrayNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##RequestNullArrayError    
addProcessLocalTest ("RequestNullArrayError" "RequestNullArrayError.cpp" 2 
    "Error:.*Argument.*[(]array_of_requests[)].*however, the following entries are MPI_REQUEST_NULL"
    ""
    )
    
##RequestPersistentButInactiveNoError
addProcessLocalTest ("RequestPersistentButInactiveNoError" "RequestPersistentButInactiveNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##RequestPersistentButInactiveError
addProcessLocalTest ("RequestPersistentButInactiveError" "RequestPersistentButInactiveError.cpp" 2 
    "Error:.*Argument.*[(]request[)].*is a persistent but in-active request!"
    ""
    )
    
##RequestCanceledNoWarning
addProcessLocalTest ("RequestCanceledNoWarning" "RequestCanceledNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##RequestCanceledWarning
addProcessLocalTest ("RequestCanceledWarning" "RequestCanceledWarning.cpp" 2 
    "Warning:.*Argument.*[(]request[)] was already marked for cancelation!"
    ""
    )

##RequestActiveRecvNoWarning
addProcessLocalTest ("RequestActiveRecvNoWarning" "RequestActiveRecvNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##RequestActiveRecvWarning
addProcessLocalTest ("RequestActiveRecvWarning" "RequestActiveRecvWarning.cpp" 2 
    "Warning:.*Argument.*[(]request[)].*is an active receive request, that should never be freed"
    ""
    ) 

##RequestActiveNoError
addProcessLocalTest ("RequestActiveNoError" "RequestActiveNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##RequestActiveError
addProcessLocalTest ("RequestActiveError" "RequestActiveError.cpp" 2 
    "Error:.*Argument.*[(]request[)].*is already an active request."
    ""
    )
 
##RequestActiveArrayNoError
addProcessLocalTest ("RequestActiveArrayNoError" "RequestActiveArrayNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##RequestActiveArrayError
addProcessLocalTest ("RequestActiveArrayError" "RequestActiveArrayError.cpp" 2 
    "Error:.*Argument.*[(]array_of_requests[)].*array of non-active requests, the following entries are already active:"
    ""
    ) 