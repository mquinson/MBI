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

##################
#Recurse into sub directories
##Not needed

###################
#Add test cases

##IntegerGreaterCommSizeNoError   (NO-ERROR)
addProcessLocalTest ("IntegerGreaterCommSizeNoError" "IntegerGreaterCommSizeNoError.cpp" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerGreaterCommSizeError   (ERROR)
addProcessLocalTest ("IntegerGreaterCommSizeError" "IntegerGreaterCommSizeError.cpp" 4 
    "Error:.*Argument.*[(]nnodes[)].* specifies a rank that is greater then the size of the given communicator."
    ""
    )
    
##IntegerArrayProductGreaterCommSizeNoError   (NO-ERROR)
addProcessLocalTest ("IntegerArrayProductGreaterCommSizeNoError" "IntegerArrayProductGreaterCommSizeNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntegerArrayProductGreaterCommSizeError   (ERROR)
addProcessLocalTest ("IntegerArrayProductGreaterCommSizeError" "IntegerArrayProductGreaterCommSizeError.cpp" 2 
    "Error:.*Argument.*[(]dims[)].*specifies a cartesian layout that uses more ranks than available in the given communicator!"
    ""
    )
    
##IntegerArrayProductLessCommSizeWarning   (WARNING)
addProcessLocalTest ("IntegerArrayProductLessCommSizeWarning" "IntegerArrayProductLessCommSizeWarning.cpp" 2 
    "Warning:.*Argument.*[(]dims[)].*specifies a cartesian layout that uses less ranks than the given communicator"
    ""
    )
    
##CommNotKnownNoError   (NO-ERROR)
addProcessLocalTest ("CommNotKnownNoError" "CommNotKnownNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##CommNotKnownError   (ERROR)
addProcessLocalTest ("CommNotKnownError" "CommNotKnownError.cpp" 2 
    "Error:.*Argument.*[(]comm[)].*is an unknown communicator where a valid communicator was expected"
    ""
    )
    
##CommNullError   (ERROR)
addProcessLocalTest ("CommNullError" "CommNullError.cpp" 2 
    "Error:.*Argument.*[(]comm[)].*is MPI_COMM_NULL."
    ""
    )

## TODO: when intercomm tracking is available, this tests should be implemented    
##IntercommNoWarning   (NO-WARNING)
addProcessLocalTest ("IntercommNoWarning" "IntercommNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IntercommWarning   (WARNING)
addProcessLocalTest ("IntercommWarning" "IntercommWarning.cpp" 2 
    "Warning:.*Argument.*"
    ""
    )

##IsIntercommNoError   (NO-Error)
addProcessLocalTest ("IsIntercommNoError" "IsIntercommNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IsIntercommError   (Error)
addProcessLocalTest ("IsIntercommError" "IsIntercommError.cpp" 2 
    "Error:.*Argument.*"
    ""
    )

##CommNotCartNoError   (NO-ERROR)
addProcessLocalTest ("CommNotCartNoError" "CommNotCartNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )

##CommNotCartError   (ERROR)
addProcessLocalTest ("CommNotCartError" "CommNotCartError.cpp" 2 
    "Error:.*Argument.*[(]comm[)].*is a communicator with no cartesian topology"
    ""
    )

##CommNotGraphNoError   (NO-ERROR)
addProcessLocalTest ("CommNotGraphNoError" "CommNotGraphNoError.cpp" 4 
    ""
    "Error;ERROR;Warning;WARNING"
    ) 

##CommNotGraphError   (ERROR)
addProcessLocalTest ("CommNotGraphError" "CommNotGraphError.cpp" 4 
    "Error:.*Argument.*[(]comm[)].*is a communicator with no graph topology"
    ""
    ) 


## TODO: when intercomm tracking is available, this tests should be implemented    
##IsIntercommNoError   (NO-ERROR)
#addProcessLocalTest ("IsIntercommNoError" "IsIntercommNoError.cpp" 2 
#    ""
#    "Error;ERROR;Warning;WARNING"
#    )
      
##IsIntercommError   (ERROR)
#addProcessLocalTest ("IsIntercommError" "IsIntercommError.cpp" 2 
#    ""
#    ""
#    )

##CommHasTopologyNoWarning   (NO-WARNING)
addProcessLocalTest ("CommHasTopologyNoWarning" "CommHasTopologyNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##CommHasTopologyWarning   (WARNING)
addProcessLocalTest ("CommHasTopologyWarning" "CommHasTopologyWarning.cpp" 2 
    "Warning:.*Argument.*[(]comm_old[)].*is a communicator that already had a process topology"
    ""
    ) 
     
##RootNotInCommNoError   (NO-ERROR)
addProcessLocalTest ("RootNotInCommNoError" "RootNotInCommNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
     
##RootNotInCommError   (ERROR)
addProcessLocalTest ("RootNotInCommError" "RootNotInCommError.cpp" 2 
    "Error:.*Argument.*[(]root[)].*is a rank that is not in the communicator"
    ""
    )
     
##CommIsPredefinedNoError   (NO-ERROR)
addProcessLocalTest ("CommIsPredefinedNoError" "CommIsPredefinedNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
 
     
##CommIsPredefinedError   (ERROR)
addProcessLocalTest ("CommIsPredefinedError" "CommIsPredefinedError.cpp" 2 
    "Error:.*Argument.*[(]comm[)].*is a predefined communicator, which must not be freed"
    ""
    )
     
##CommIsNullNoWarning   (NO-WARNING)
addProcessLocalTest ("CommIsNullNoWarning" "CommIsNullNoWarning.cpp" 3 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##CommIsNullWarning   (WARNING)
addProcessLocalTest ("CommIsNullWarning" "CommIsNullWarning.cpp" 3 
    "Warning:.*Argument.*[(]comm2[)].*is MPI_COMM_NULL, which is allowed but unusual"
    ""
    )
     
##MaxDimsGreaterNDimsNoWarning   (NO-WARNING)
addProcessLocalTest ("MaxDimsGreaterNDimsNoWarning" "MaxDimsGreaterNDimsNoWarning.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##MaxDimsGreaterNDimsWarning  (WARNING)
addProcessLocalTest ("MaxDimsGreaterNDimsWarning" "MaxDimsGreaterNDimsWarning.cpp" 2 
    "Warning:.*Argument.*[(]maxdims[)].*topology.*which is greater then the number of dimensions in the given communicator"
    ""
    )
  
##DirectionGreaterNDimsNoError  (NO-Error)
addProcessLocalTest ("DirectionGreaterNDimsNoError" "DirectionGreaterNDimsNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##DirectionGreaterNDimsError  (Error)
addProcessLocalTest ("DirectionGreaterNDimsError" "DirectionGreaterNDimsError.cpp" 2 
    "Error:.*Argument.*[(]direction[)].*topology, but is out of range, a number from 0 to ndims-1 was expected"
    ""
    )
  
##MaxneighborsToSmallNoWarning   (NO-WARNING)
addProcessLocalTest ("MaxneighborsToSmallNoWarning" "MaxneighborsToSmallNoWarning.cpp" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##MaxneighborsToSmallWarning  (WARNING)
addProcessLocalTest ("MaxneighborsToSmallWarning" "MaxneighborsToSmallWarning.cpp" 4 
    "Warning:.*Argument.*[(]maxneighbors[)].*neighbors expected to be retrieved from this call, but is smaller then the actual number of neighbors"
    ""
    )

##MaxIndicesToSmallNoWarning   (NO-WARNING)
addProcessLocalTest ("MaxIndicesToSmallNoWarning" "MaxIndicesToSmallNoWarning.cpp" 4
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##MaxIndicesToSmallWarning  (WARNING)
addProcessLocalTest ("MaxIndicesToSmallWarning" "MaxIndicesToSmallWarning.cpp" 4 
    "Warning:.*Argument.*[(]maxindices[)].*graph topology, but is smaller then the actual number of indices"
    ""
    )
    
##MaxEdgesToSmallWarning  (WARNING)
addProcessLocalTest ("MaxEdgesToSmallWarning" "MaxEdgesToSmallWarning.cpp" 4 
    "Warning:.*Argument.*[(]maxedges[)].*graph topology, but is smaller then the number of edges in the given communicator"
    ""
    )
 
##IsNoIntercommNoError   (NO-Error)
addProcessLocalTest ("IsNoIntercommNoError" "IsNoIntercommNoError.cpp" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )
    
##IsNoIntercommError   (Error)
addProcessLocalTest ("IsNoIntercommError" "IsNoIntercommError.cpp" 2 
    "Error:.*Argument.*[(]intercomm[)].*intracommunicator and was used where an intercommunicator was expected"
    ""
    )
    
### TODO isIntercomm MPI1/MPI2