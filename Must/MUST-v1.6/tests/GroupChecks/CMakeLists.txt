# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for GroupChecks test.
#
# @author Tobias Hilbrich,Mathias Korepkat
# @date 23.05.2011
#

##################
#Recurse into sub directories
##Not needed

###################
#Add test cases

##GroupNotKnownNoError   (NO-ERROR)
addProcessLocalTest ("GroupNotKnownNoError" "GroupNotKnownNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##GroupNotKnownError   
addProcessLocalTest ("GroupNotKnownError" "GroupNotKnownError.cpp" 2 
    "Error:.*Argument.*[(]group[)].*is an unknown group"
    ""
    )

##GroupNullError   
addProcessLocalTest ("GroupNullError" "GroupNullError.cpp" 2 
    "Error:.*Argument.*[(]group[)].*is null, where a valid group was expected"
    ""
    )

##IntegerGreaterGrpSizeNoError   
addProcessLocalTest ("IntegerGreaterGrpSizeNoError" "IntegerGreaterGrpSizeNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##IntegerGreaterGrpSizeError   
addProcessLocalTest ("IntegerGreaterGrpSizeError" "IntegerGreaterGrpSizeError.cpp" 2 
    "Error:.*Argument.*[(]n[)].*is greater then the size of the MPI group"
    ""
    )

##IntegerArrayElementGreaterGrpSizeError   
addProcessLocalTest ("IntegerArrayElementGreaterGrpSizeError" "IntegerArrayElementGreaterGrpSizeError.cpp" 2 
    "Error:.*Argument.*[(]ranks1[)].*array of ranks that must be in the given MPI group.*entries do not match this criteria"
    ""
    )

##RankFromRangesNotInGroupNoError   
addProcessLocalTest ("RankFromRangesNotInGroupNoError" "RankFromRangesNotInGroupNoError.cpp" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##RankFromRangesNotInGroupError   
addProcessLocalTest ("RankFromRangesNotInGroupError" "RankFromRangesNotInGroupError.cpp" 4 
    "Error:.*Argument.*[(]ranges[)].*array of triplets of the form.*ranks must be in the MPI group.*fellowing triplets do not"
    ""
    ) 

##GroupIsEmptyNoWarning  
addProcessLocalTest ("GroupIsEmptyNoWarning" "GroupIsEmptyNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##GroupIsEmptyWarning  
addProcessLocalTest ("GroupIsEmptyWarning" "GroupIsEmptyWarning.cpp" 2 
    "Warning:.*Argument.*[(]group[)].*is an empty group, which is allowed but unusual"
    ""
    )

##GroupNullNoWarning  
addProcessLocalTest ("GroupNullNoWarning" "GroupNullNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##GroupNullWarning  
addProcessLocalTest ("GroupNullWarning" "GroupNullWarning.cpp" 2 
    "Warning:.*Argument.*[(]group1[)].*is MPI_GROUP_NULL, which is allowed but unusual"
    ""
    )
 
##GroupRankNotInCommNoError  
addProcessLocalTest ("GroupRankNotInCommNoError" "GroupRankNotInCommNoError.cpp" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )

##GroupRankNotInCommError  
addProcessLocalTest ("GroupRankNotInCommError" "GroupRankNotInCommError.cpp" 4
    "Error:.*Argument.*[(]group[)].*group which should be a subset of argument.*ranks are in the group but not in the communicator"
    ""
    )
    