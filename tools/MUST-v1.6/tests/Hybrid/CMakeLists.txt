# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for CommChecks test.
#
# @author Tobias Hilbrich,Mathias Korepkat
# @date 14.04.2011
#

IF (NOT (GTI_IS_THREAD_SAFE AND OpenMP_FOUND))
    RETURN()
ENDIF ()


##################
#Recurse into sub directories
##Not needed

###################
#Add test cases

find_package(OpenMP)

##IntegerGreaterCommSizeNoError   (NO-ERROR)
addHybridTest ("CommSizeOnWorkerThread" "CommSizeOnWorkerThread.cpp" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##MpiInitInParallel   (NO-ERROR)
addHybridTest ("MpiInitInParallel" "MpiInitInParallel.cpp" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
