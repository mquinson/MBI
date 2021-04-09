# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for Integrity test.
#
# @author Tobias Hilbrich, Mathias Korepkat
# @date 27.05.2011
#

##################
#Recurse into sub directories
##Not needed

###################
#Add test case

##IntegrityIfNullConditionNoError   (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullConditionNoError" "IntegrityIfNullConditionNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##IntegrityIfNullConditionError      (ERROR)
addProcessLocalTest ("IntegrityIfNullConditionError" "IntegrityIfNullConditionError.cpp" 2 
    "Error:.*Argument.*[(]periods[)] is a NULL pointer where an allocated memory region"
    ""
    )

##IntegrityIfNullNoError   (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullNoError" "IntegrityIfNullNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##IntegrityIfNullError      (ERROR)
addProcessLocalTest ("IntegrityIfNullError" "IntegrityIfNullError.cpp" 2 
    "Error:.*Argument.*[(]inbuf[)] is a NULL pointer where an allocated memory region with a size"
    ""
    )
    
##IntegrityIfNullAndNotMpiBottomNoError   (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullAndNotMpiBottomNoError" "IntegrityIfNullAndNotMpiBottomNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##IntegrityIfNullAndNotMpiBottomError   (ERROR)
addProcessLocalTest ("IntegrityIfNullAndNotMpiBottomError" "IntegrityIfNullAndNotMpiBottomError.cpp" 2 
    "Error:.*Argument.*[(]buf[)] is a NULL pointer where an allocated memory region was expected"
    ""
    )
    
##IntegrityIfNullCommSizeNoError   (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullCommSizeNoError" "IntegrityIfNullCommSizeNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
        
##IntegrityIfNullCommSizeError   (ERROR)
addProcessLocalTest ("IntegrityIfNullCommSizeError" "IntegrityIfNullCommSizeError.cpp" 2 
    "Error:.*Argument.*[(]recvcounts[)].*is a NULL pointer where an allocated memory region was expected"
    ""
    )
        
##IntegrityIfNullNotMPIBottomCommSizeArrayError   (ERROR)
addProcessLocalTest ("IntegrityIfNullNotMPIBottomCommSizeArrayError" "IntegrityIfNullNotMPIBottomCommSizeArrayError.cpp" 2 
    "Error:.*Argument.*[(]recvbuf[)].*is a NULL pointer where an allocated region was expected!"
    ""
    )

##IntegrityIfNullNoWarning   (NO-WARNING)
addProcessLocalTest ("IntegrityIfNullNoWarning" "IntegrityIfNullNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )      
        
##IntegrityIfNullWarning   (WARNING)
addProcessLocalTest ("IntegrityIfNullWarning" "IntegrityIfNullWarning.cpp" 1 
    "Warning: from: call MPI_Keyval_create@.*: Argument 4 [(]extra_state[)] is a NULL pointer,  which is allowed but unusual"
    ""
    )
    
##IntegrityIfNullNoAtIndexMpiBottomNoError   (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullNoAtIndexMpiBottomNoError" "IntegrityIfNullNoAtIndexMpiBottomNoError.cpp" 2 
    ""
    "Error;ERROR"
    )
    
##IntegrityIfNullNoAtIndexMpiBottomError   (ERROR)
addProcessLocalTest ("IntegrityIfNullNoAtIndexMpiBottomError" "IntegrityIfNullNoAtIndexMpiBottomError.cpp" 2 
    "Error:.*Argument.*[(]recvbuf[)].*is a NULL pointer where an allocated memory region"
    ""
    )
    
##IntegrityIfNullStatusNoError  (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullStatusNoError" "IntegrityIfNullStatusNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
 
     
##IntegrityIfNullStatusError  (ERROR)
addProcessLocalTest ("IntegrityIfNullStatusError" "IntegrityIfNullStatusError.cpp" 2 
    "Error:.*Argument.*[(]status[)].*is a NULL pointer where an allocated memory region"
    ""
    )
    
##IntegrityIfNullStatusesNoError  (NO-ERROR)
addProcessLocalTest ("IntegrityIfNullStatusesNoError" "IntegrityIfNullStatusesNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegrityIfNullStatusesError  (ERROR)
addProcessLocalTest ("IntegrityIfNullStatusesError" "IntegrityIfNullStatusesError.cpp" 2 
    "Error:"
    ""
    )  