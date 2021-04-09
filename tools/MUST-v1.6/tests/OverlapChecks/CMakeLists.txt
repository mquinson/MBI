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

## Using a datatype for communication, that can be repeated 5 times without overlap and overlaps for 6 times repetition
addProcessLocalTest ("VectorOverlap" "vectorOverlap.c" 1 
    "Warning[:] from[:] call MPI_Type_commit@.*0[:] Argument 1 .datatype. is selfoverlapping !.*Error[:] from[:] call MPI_Pack@.*0[:] Argument 3 .datatype. is selfoverlapping after repetition of 1 !"
    ""
    )

## overlapping requests
addProcessLocalTest ("CommOverlap" "commOverlap.c" 2  
    "Error. from. call MPI_Irecv@.*1. The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!"
    ""
    )

## overlapping persistent requests
addProcessLocalTest ("CommOverlapInit" "commOverlapInit.c" 2 
    "Error[:] from[:] call MPI_Startall@.*1[:] A start of a persistent request will transfer memory regions that are still used by a pending non-blocking communication!.*References.*[:] Reference 1[:] call MPI_Recv_init@.*1"
    ""
    )

## Test for selfoverlapping on repetition in communication
addProcessLocalTest ("CommSelfOverlap" "commSelfOverlap.c" 1 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error[:] from[:] call MPI_Pack@.*0[:] Argument 3 .datatype. is selfoverlapping after repetition of 6 !.*Error[:] from[:] call MPI_Unpack@.*0[:] Argument 6 .datatype. is selfoverlapping after repetition of 6 !"
    ""
    )

## overlapping persistent requests
addProcessLocalTest ("CommOverlapInitWaitall" "commOverlapInitWaitall.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error[:] from[:] call MPI_Startall@.*1[:] A start of a persistent request will transfer memory regions that are still used by a pending non-blocking communication!.*References.*[:] Reference 1[:] call MPI_Recv_init@.*1"
    ""
    )

## overlapping requests
addProcessLocalTest ("CommOverlapWaitall" "commOverlapWaitall.c" 2 
    "Error[:] from[:] call MPI_Irecv@.*1[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!"
    ""
    )

## Bcast that overlaps a pending receive request
addProcessLocalTest ("OverlapBcast" "overlapBcast.c" 2 
    "Error[:] from[:] call MPI_Bcast@.*1[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!"
    ""
    )

## Allreduce that overlaps a pending receive request
addProcessLocalTest ("OverlapAllreduce" "overlapAllreduce.c" 2 
    "Error[:] from[:] call MPI_Allreduce@.*1[:] The memory regions to be transfered by this send operation overlap with regions spanned by a pending non-blocking receive operation!.*References.*[:] Reference 1[:] call MPI_Irecv@.*1;.*Error[:] from[:] call MPI_Allreduce@.*1[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Irecv@.*1.*"
    ""
    )

## Scatter that overlaps a pending receive request
addProcessLocalTest ("OverlapScatter" "overlapScatter.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error[:] from[:] call MPI_Scatter@.*1[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Irecv@.*1.*Error[:] from[:] call MPI_Scatter@.*0[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Isend@.*0;Error[:] from[:] call MPI_Scatter@.*0[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Isend@.*0.*Error[:] from[:] call MPI_Scatter@.*1[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Irecv@.*1"
    ""
    )

## Gather that overlaps a pending receive request
addProcessLocalTest ("OverlapGather" "overlapGather.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error[:] from[:] call MPI_Gather@.*1[:] The memory regions to be transfered by this send operation overlap with regions spanned by a pending non-blocking receive operation!.*References.*[:] Reference 1[:] call MPI_Irecv@.*1.*Error[:] from[:] call MPI_Gather@.*0[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Isend@.*0;Error[:] from[:] call MPI_Gather@.*0[:] The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!.*References.*[:] Reference 1[:] call MPI_Isend@.*0.*Error[:] from[:] call MPI_Gather@.*1[:] The memory regions to be transfered by this send operation overlap with regions spanned by a pending non-blocking receive operation!.*References.*[:] Reference 1[:] call MPI_Irecv@.*1"
    ""
    )

## Gather with overlapping send and receive buffer
addProcessLocalTest ("SendRecvOverlapGather" "sendRecvOverlapGather.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error[:] from[:] call MPI_Gather@.*0[:] The memory regions spanned by the recv part overlaps at the 0[(]th[)] repetition of datatype at its position [(]MPI_INT[)] with regions spanned by the send part of this operation!"
    ""
    )

## Sendrecv with overlapping send and receive buffer
addProcessLocalTest ("SendRecvOverlap" "sendRecvOverlap.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error[:] from[:] call MPI_Sendrecv@.*1[:] The memory regions spanned by the recv part overlaps at the 0[(]th[)] repetition of datatype at its position [(]MPI_INT[)] with regions spanned by the send part of this operation!.*Error[:] from[:] call MPI_Sendrecv@.*0[:] The memory regions spanned by the recv part overlaps at the 0[(]th[)] repetition of datatype at its position [(]MPI_INT[)] with regions spanned by the send part of this operation!;Error[:] from[:] call MPI_Sendrecv@.*0[:] The memory regions spanned by the recv part overlaps at the 0[(]th[)] repetition of datatype at its position [(]MPI_INT[)] with regions spanned by the send part of this operation!"
    ""
    )

## Sendrecv with overlapping send and receive buffer
addProcessLocalTest ("SendRecvNoOverlap" "sendRecvNoOverlap.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    ""
    "Error;ERROR;Warning;WARNING"
    )

## Sendrecv with overlapping send and receive buffer
addProcessLocalTest ("SendRecvOverlapIndexed" "sendRecvOverlapIndexed.c" 2 
    #Error: from: call MPI_Type_commit@rank 0: Argument 1 (datatype) is MPI_DATATYPE_NULL!
    "Error;ERROR;Warning;WARNING"
    ""
    )

# ################################
# ## No error Check
# ## Bcast should NOT overlap a pending receive request
addProcessLocalTest ("NoOverlapBcast" "noOverlapBcast.c" 2 
    ""
    "Error;ERROR;Warning;WARNING"
    )


# ################################
# ## No error Check
# ## Creation, Commit, Communication with no error
# addProcessLocalTest ("NotValidForCxxxx_NoErRoR" "NotValidForCxxxx_NoErRoR.cpp" 1 
#     ""
#     "Error;ERROR;Warning;WARNING"
#     )
