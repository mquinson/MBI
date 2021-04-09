# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for LeakChecks directory.
#
# @author Tobias Hilbrich, Mathias Korepkat
# @date 19.05.2011
#

##################
#Recurse into sub directories
##Not needed

###################
#Add test cases

##No Error
addProcessLocalTest ("LeaksNoError" "LeaksNoError.c" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
## Error: leaked comm
addProcessLocalTest ("LeakCommError" "LeakCommError.c" 2 
    "There are 1 communicators that are not freed when MPI_Finalize"
    ""
    )
    
## Error: leaked group
addProcessLocalTest ("LeakGroupError" "LeakGroupError.c" 2 
    "There are 1 groups that are not freed when MPI_Finalize was issued"
    ""
    )
    
## Error: leaked errorhandler
addProcessLocalTest ("LeakErrError" "LeakErrError.c" 2 
    "There are 1 error handlers that are not freed when MPI_Finalize was"
    ""
    )
    
## Error: leaked keyvalue
addProcessLocalTest ("LeakKeyvalError" "LeakKeyvalError.c" 2 
    "There are 1 keys that are not freed when MPI_Finalize was issued"
    ""
    )
    
## Error: leaked operation
addProcessLocalTest ("LeakOpError" "LeakOpError.c" 2 
    "There are 1 operations that are not freed when MPI_Finalize"
    ""
    )

## Error: leaked request
addProcessLocalTest ("LeakRequestError" "LeakRequestError.c" 2 
    "There are 1 requests that are not freed when MPI_Finalize "
    ""
    )
    
## Error: leaked datatype
addProcessLocalTest ("LeakTypeError" "LeakTypeError.c" 2 
    "There are 1 datatypes that are not freed when MPI_Finalize"
    ""
    )