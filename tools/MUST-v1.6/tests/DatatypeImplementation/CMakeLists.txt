# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for OverlapChecks test.
#
# @author Joachim Protze
# @date 3.06.2011
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


# ################################
# ## Simple Checks
# ## contiguous
addProcessLocalTest ("simpleContiguous" "simpleContiguous.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## vector
addProcessLocalTest ("simpleVector" "simpleVector.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## hvector
addProcessLocalTest ("simpleHVector" "simpleHVector.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## indexed
addProcessLocalTest ("simpleIndexed" "simpleIndexed.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## hindexed
addProcessLocalTest ("simpleHIndexed" "simpleHIndexed.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## struct
addProcessLocalTest ("simpleStruct" "simpleStruct.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## indexed block
addProcessLocalTest ("simpleIndexedBlock" "simpleIndexedBlock.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## subarray
addProcessLocalTest ("simpleSubarray" "simpleSubarray.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## darray
addProcessLocalTest ("simpleDarray" "simpleDarray.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )


# ################################
# ## Tree
# ## recursion depth 2
addProcessLocalTest ("treeRecursion2" "treeRecursion2.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## recursion depth 4
addProcessLocalTest ("treeRecursion4" "treeRecursion4.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )

# ## recursion depth 6
addProcessLocalTest ("treeRecursion6" "treeRecursion6.c;datatypetest.c" 1 
    ""
    "Error;ERROR;Warning;WARNING"
    )


# ################################
# ## LB / UB
# ## reduce size by indexed
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeReduceByIndexed" "resizeReduceByIndexed.c;datatypetest.c" 1 
        ""
        "Error;ERROR" #warnings are permitted
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## reduce size by struct w/o limits
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeReduceByStruct" "resizeReduceByStruct.c;datatypetest.c" 1 
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## reduce size by struct w/ limits
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeReduceByStructNoLimits" "resizeReduceByStructNolimit.c;datatypetest.c" 1 
        ""
        "Error;ERROR"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## reduce size by resized
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeReduceByResized" "resizeReduceByResized.c;datatypetest.c" 1 
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## expand size by indexed
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeExpandByIndexed" "resizeExpandByIndexed.c;datatypetest.c" 1 
        ""
        "Error;ERROR"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## expand size by struct w/o limits
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeExpandByStruct" "resizeExpandByStruct.c;datatypetest.c" 1 
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## expand size by struct w/ limits
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeExpandByStructNoLimits" "resizeExpandByStructNolimit.c;datatypetest.c" 1 
        ""
        "Error;ERROR"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ## expand size by resized
IF (HAVE_MPI_TYPE_CREATE_RESIZED)
    addProcessLocalTest ("resizeExpandByResized" "resizeExpandByResized.c;datatypetest.c" 1 
        ""
        "Error;ERROR;Warning;WARNING"
        )
ENDIF (HAVE_MPI_TYPE_CREATE_RESIZED)

# ################################
# ## No error Check
# ## Creation, Commit, Communication with no error
# addProcessLocalTest ("NotValidForCxxxx_NoErRoR" "NotValidForCxxxx_NoErRoR.cpp" 1 
#     ""
#     "Error;ERROR;Warning;WARNING"
#     )
