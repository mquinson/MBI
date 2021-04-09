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

##IntegerNoError   (NO-ERROR)
addProcessLocalTest ("IntegerNoError" "IntegerNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##IntegerZeroWarning
addProcessLocalTest ("IntegerZeroWarning" "IntegerZeroWarning.cpp" 2 
    "Warning.*count.*is"
    ""
    )
    
##IntegerNegativeError
addProcessLocalTest ("IntegerNegativeError" "IntegerNegativeError.cpp" 2 
    "Error.*count.*is negative"
    ""
    )

##IntegerZeroNoError
addProcessLocalTest ("IntegerZeroNoError" "IntegerZeroNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
        
##IntegerZeroError
addProcessLocalTest ("IntegerZeroError" "IntegerZeroError.cpp" 2 
    "Error.*Argument 3 [(]disp[)] is zero!"
    ""
    )

##IntegerNotOneOrZeroNoWarning
addProcessLocalTest ("IntegerNotOneOrZeroNoWarning" "IntegerNotOneOrZeroNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerNotOneOrZeroWarning
addProcessLocalTest ("IntegerNotOneOrZeroWarning" "IntegerNotOneOrZeroWarning.cpp" 2 
    "Warning.*logical argument 5 [(]reorder[)] is neither 1 or 0"
    ""
    )

##IntegerNegativeArrayNoError
addProcessLocalTest ("IntegerNegativeArrayNoError" "IntegerNegativeArrayNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
        
##IntegerNegativeArrayError
addProcessLocalTest ("IntegerNegativeArrayError" "IntegerNegativeArrayError.cpp" 2 
    "Error.*Argument 3 [(]dims[)].*non-negative.*dims.1.=-1"
    ""
    )

##IntegerZeroArrayNoWarning
addProcessLocalTest ("IntegerZeroArrayNoWarning" "IntegerZeroArrayNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
        
##IntegerZeroArrayWarning
addProcessLocalTest ("IntegerZeroArrayWarning" "IntegerZeroArrayWarning.cpp" 2 
    "Warning[:] from[:] call MPI_Gatherv@.*0.*[:] Argument 5 [(]recvcounts[)] is an array that contains zero value[(]s[)], which is correct but unusual"
    ""
    )
    
##GroupRangeNoError
addProcessLocalTest ("GroupRangeNoError" "GroupRangeNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
            
##GroupRangeStrideZeroError
addProcessLocalTest ("GroupRangeStrideZeroError" "GroupRangeStrideZeroError" 2 
    "Error:.*Argument.*[(]ranges[)].*triplets that contains zero for a stride, which must be greater or smaller than 0"
    ""
    )
    
##IntegerEntryGreaterEqualArrayNoError
addProcessLocalTest ("IntegerEntryGreaterEqualArrayNoError" "IntegerEntryGreaterEqualArrayNoError" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerEntryGreaterEqualArrayError
addProcessLocalTest ("IntegerEntryGreaterEqualArrayError" "IntegerEntryGreaterEqualArrayError" 4 
    "Error:.*Argument.*[(]edges[)].*, the following entries list higher node indices"
    ""
    )   
       
##IntegerNotOneOrZeroArrayNoWarning
addProcessLocalTest ("IntegerNotOneOrZeroArrayNoWarning" "IntegerNotOneOrZeroArrayNoWarning" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
 
        
##IntegerNotOneOrZeroArrayWarning
addProcessLocalTest ("IntegerNotOneOrZeroArrayWarning" "IntegerNotOneOrZeroArrayWarning" 2 
    "Warning:.*The array of logical arguments.*[(]periods[)].*has entries that are neither 1 or 0, which is valid but"
    ""
    )
   
        
##IntegerNegativeNotProcNullAnySourceNoError
addProcessLocalTest ("IntegerNegativeNotProcNullAnySourceNoError" "IntegerNegativeNotProcNullAnySourceNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
        
##IntegerNegativeNotProcNullAnySourceError
addProcessLocalTest ("IntegerNegativeNotProcNullAnySourceError" "IntegerNegativeNotProcNullAnySourceError" 2 
    "Error:.*Argument.*[(]source[)].*has to be a rank in the given communicator, MPI_PROC_NULL, or MPI_ANY_SOURCE, but is a negative value"
    ""
    )
 
##IntegerNegativeNotProcNullNoError
addProcessLocalTest ("IntegerNegativeNotProcNullNoError" "IntegerNegativeNotProcNullNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
 
##IntegerNegativeNotProcNullError
addProcessLocalTest ("IntegerNegativeNotProcNullError" "IntegerNegativeNotProcNullError" 2 
    "Error:.*Argument.*[(]dest[)].*has to be a rank in the given communicator or MPI_PROC_NULL, but is negative"
    ""
    )
    
##IntegerNotWithinZeroTagUbNoError
addProcessLocalTest ("IntegerNotWithinZeroTagUbNoError" "IntegerNotWithinZeroTagUbNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerNotWithinZeroTagUbError
addProcessLocalTest ("IntegerNotWithinZeroTagUbError" "IntegerNotWithinZeroTagUbError" 2 
    "Error:.*Argument.*[(]tag[)].*is a tag, which is outside the range of valid values"
    ""
    )

    
##IntegerLessTagUbNoWarning
addProcessLocalTest ("IntegerLessTagUbNoWarning" "IntegerLessTagUbNoWarning" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
   
     
##IntegerLessTagUbWarning
addProcessLocalTest ("IntegerLessTagUbWarning" "IntegerLessTagUbWarning" 2 
    "Warning:.*Argument.*[(]tag[)].*is a tag that is larger than 32767 and may thus only be supported by some MPI implementations"
    ""
    )
   
##IntegerNegativeNotProcNullArrayNoError
addProcessLocalTest ("IntegerNegativeNotProcNullArrayNoError" "IntegerNegativeNotProcNullArrayNoError" 3 
    ""
    "Error;ERROR;Warning;WARNING"
    )
   
##IntegerNegativeNotProcNullArrayError
addProcessLocalTest ("IntegerNegativeNotProcNullArrayError" "IntegerNegativeNotProcNullArrayError" 3 
    "Error:.*Argument.*[(]ranks1[)].*rank that must be in the given communicator or MPI_PROC_NULL"
    ""
    )

##IntegerNegativeProcNullAnySourceNoError
addProcessLocalTest ("IntegerNegativeProcNullAnySourceNoError" "IntegerNegativeProcNullAnySourceNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerNegativeProcNullAnySourceError
addProcessLocalTest ("IntegerNegativeProcNullAnySourceError" "IntegerNegativeProcNullAnySourceError" 2 
    "Error:.*Argument.*[(]root[)].*is a rank that must be in the given communicator, but it is either negative, MPI_PROC_NULL, or MPI_ANY_SOURCE"
    ""
    )

##IntegerNegativeNotUndefinedNoError
addProcessLocalTest ("IntegerNegativeNotUndefinedNoError" "IntegerNegativeNotUndefinedNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerNegativeNotUndefinedError
addProcessLocalTest ("IntegerNegativeNotUndefinedError" "IntegerNegativeNotUndefinedError" 2 
    "Error:.*Argument.*[(]color[)].*non-negative integer or MPI_Undefined but it is:"
    ""
    )
    
##NoDuplicatedRanksArrayNoError
addProcessLocalTest ("NoDuplicatedRanksArrayNoError" "NoDuplicatedRanksArrayNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##NoDuplicatedRanksArrayError
addProcessLocalTest ("NoDuplicatedRanksArrayError" "NoDuplicatedRanksArrayError" 2 
    "Error:.*Argument.*[(]ranks[)].*array of ranks where no duplications are allowed.*entries are duplicated"
    ""
    )
    
##NoDuplicatedRanksTripletArrayNoError
addProcessLocalTest ("NoDuplicatedRanksTripletArrayNoError" "NoDuplicatedRanksTripletArrayNoError" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##NoDuplicatedRanksTripletArrayError
addProcessLocalTest ("NoDuplicatedRanksTripletArrayError" "NoDuplicatedRanksTripletArrayError" 4 
    "Error:.*Argument.*[(]ranges[)].*array of triplets.*where no duplication of ranks is allowed.*contain equal ranks"
    ""
    )
    
##OpIsPredefinedNoError
addProcessLocalTest ("OpIsPredefinedNoError" "OpIsPredefinedNoError" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##OpIsPredefinedError
addProcessLocalTest ("OpIsPredefinedError" "OpIsPredefinedError" 2 
    "Error:.*Argument.*[(]op[)].*is a predefined operation where a user-defined operation was expected"
    ""
    ) 
    
##OpIsNotKnownError
addProcessLocalTest ("OpIsNotKnownError" "OpIsNotKnownError" 2 
    "Error:.*Argument.*[(]op[)].*is a unknown operation where a valid operation was expected"
    ""
    ) 
    
##OpIsNullError
addProcessLocalTest ("OpIsNullError" "OpIsNullError" 2 
    "Error:.*Argument.*[(]op[)] is MPI_OP_NULL where a valid operation was expected"
    ""
    ) 
    