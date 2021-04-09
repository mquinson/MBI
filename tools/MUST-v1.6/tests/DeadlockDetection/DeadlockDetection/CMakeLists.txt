# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for P2P matching tests that are part of deadlock detection.
#
# @author Tobias Hilbrich
# @date 26.07.2011
#

###################
#Add test case
## simpleNoDl
addProcessLocalTest ("simpleNoDl" "simpleNoDl.c"  2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitNoDl
addProcessLocalTest ("waitNoDl" "waitNoDl.c"  2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitallNoDl
addProcessLocalTest ("waitallNoDl" "waitallNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitAnySourceNoDl
addProcessLocalTest ("waitAnySourceNoDl" "waitAnySourceNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitallAnySourceNoDl
addProcessLocalTest ("waitallAnySourceNoDl" "waitallAnySourceNoDl.c"  4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitAnySourceEx1NoDl
addProcessLocalTest ("waitAnySourceEx1NoDl" "waitAnySourceEx1NoDl.c"  2
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitanyNoDl
addProcessLocalTest ("waitanyNoDl" "waitanyNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitanyNoDlEx1
addProcessLocalTest ("waitanyNoDlEx1" "waitanyNoDlEx1.c"   4 
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitsomeNoDl
addProcessLocalTest ("waitsomeNoDl" "waitsomeNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitallNullReqNoDl
addProcessLocalTest ("waitallNullReqNoDl" "waitallNullReqNoDl.c"  4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## bsendNoDl
addProcessLocalTest ("bsendNoDl" "bsendNoDl.c"  4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## commNoDl
addProcessLocalTest ("commNoDl" "commNoDl.c"  3
    ""
    "Error;ERROR"
    )


## testanyNoDl
addProcessLocalTest ("testanyNoDl" "testanyNoDl.c" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## sendrecvNoDl
addProcessLocalTest ("sendrecvNoDl" "sendrecvNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## persistentNoDl
addProcessLocalTest ("persistentNoDl" "persistentNoDl.c"  4
    ""
    "Error;ERROR"
    )


## reduceBcastCommNoDl
addDDlTest ("reduceBcastCommNoDl" "reduceBcastCommNoDl.c"  4
    ""
    "Error;ERROR"
    )


## sendDl
addProcessLocalTest ("sendDl" "sendDl.c"  2
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Send"
    ""
    )


## recvDl
addProcessLocalTest ("recvDl" "recvDl.c"  2
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## persistentDl
addProcessLocalTest ("persistentDl" "persistentDl.c"  4
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall"
    ""
    )


## sendrecvDl
addProcessLocalTest ("sendrecvDl" "sendrecvDl.c"  4
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Sendrecv"
    ""
    )


## waitallAnySourceDl
addProcessLocalTest ("waitallAnySourceDl" "waitallAnySourceDl.c"  3
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall"
    ""
    )


## waitallDl
addProcessLocalTest ("waitallDl" "waitallDl.c"  4
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall"
    ""
    )


## waitanyDl
addProcessLocalTest ("waitanyDl" "waitanyDl.c"  3
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitany"
    ""
    )


## waitallProcNullNoDl
addProcessLocalTest ("waitallProcNullNoDl" "waitallProcNullNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## sendProcNullNoDl
addProcessLocalTest ("sendProcNullNoDl" "sendProcNullNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## sendrecvProcNullNoDl
addProcessLocalTest ("sendrecvProcNullNoDl" "sendrecvProcNullNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitAnySourceEx2Dl
addProcessLocalTest ("waitAnySourceEx2Dl" "waitAnySourceEx2Dl.c"  2
    "Information.*MUST issued a deadlock detection while a wildcard receive call.*The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## waitAnySourceEx3Dl
addProcessLocalTest ("waitAnySourceEx3Dl" "waitAnySourceEx3Dl.c"  2
    "Information.*MUST issued a deadlock detection while a wildcard receive call.*The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## waitAnySourceEx4Dl
addProcessLocalTest ("waitAnySourceEx4Dl" "waitAnySourceEx4Dl.c"  4
    "Information.*MUST issued a deadlock detection while a wildcard receive call.*The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## waitAnySourceEx5NoDlError
addProcessLocalTest ("waitAnySourceEx5NoDlError" "waitAnySourceEx5NoDlError.c"  4
    "The application fails to match a point-to-point operation before"
    ""
    )

    
###################
# SAME TEST CASES AS ABOVE, BUT NOW WITH DISTRIBUTED WAIT STATE!
## simpleNoDl
addDDlTest ("DDlsimpleNoDl" "simpleNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## waitNoDl
addDDlTest ("DDlwaitNoDl" "waitNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )

## waitallNoDl
addDDlTest ("DDlwaitallNoDl" "waitallNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitAnySourceNoDl
addDDlTest ("DDlwaitAnySourceNoDl" "waitAnySourceNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitallAnySourceNoDl
addDDlTest ("DDlwaitallAnySourceNoDl" "waitallAnySourceNoDl.c"  4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitAnySourceEx1NoDl
addDDlTest ("DDlwaitAnySourceEx1NoDl" "waitAnySourceEx1NoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitanyNoDl
addDDlTest ("DDlwaitanyNoDl" "waitanyNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitanyNoDlEx1
addDDlTest ("DDlwaitanyNoDlEx1" "waitanyNoDlEx1.c"   4 
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitsomeNoDl
addDDlTest ("DDlwaitsomeNoDl" "waitsomeNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitallNullReqNoDl
addDDlTest ("DDlwaitallNullReqNoDl" "waitallNullReqNoDl.c"  4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## bsendNoDl
addDDlTest ("DDlbsendNoDl" "bsendNoDl.c"  4
    ""
    "Error;ERROR;Warning;WARNING"
    )


## commNoDl
addDDlTest ("DDlcommNoDl" "commNoDl.c"  3
    ""
    "Error;ERROR"
    )


## testanyNoDl
addDDlTest ("DDltestanyNoDl" "testanyNoDl.c" 3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## sendrecvNoDl
addDDlTest ("DDlsendrecvNoDl" "sendrecvNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## persistentNoDl
addDDlTest ("DDlpersistentNoDl" "persistentNoDl.c"  4
    ""
    "Error;ERROR"
    )


## reduceBcastCommNoDl
addDDlTest ("DDlreduceBcastCommNoDl" "reduceBcastCommNoDl.c"  4
    ""
    "Error;ERROR"
    )
    

## sendDl
addDDlTest ("DDlsendDl" "sendDl.c"  3
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Send"
    ""
    )


## recvDl
addDDlTest ("DDlrecvDl" "recvDl.c"  3
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## persistentDl
addDDlTest ("DDlpersistentDl" "persistentDl.c"  4
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall"
    ""
    )


## sendrecvDl
addDDlTest ("DDlsendrecvDl" "sendrecvDl.c"  4
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Sendrecv"
    ""
    )


## waitallAnySourceDl
addDDlTest ("DDlwaitallAnySourceDl" "waitallAnySourceDl.c"  3
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall"
    ""
    )


## waitallDl
addDDlTest ("DDlwaitallDl" "waitallDl.c"  4
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitall"
    ""
    )


## waitanyDl
addDDlTest ("DDlwaitanyDl" "waitanyDl.c"  3
    "The application issued a set of MPI calls that can cause a deadlock!.*MPI_Waitany"
    ""
    )


## waitallProcNullNoDl
addDDlTest ("DDlwaitallProcNullNoDl" "waitallProcNullNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## sendProcNullNoDl
addDDlTest ("DDlsendProcNullNoDl" "sendProcNullNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## sendrecvProcNullNoDl
addDDlTest ("DDlsendrecvProcNullNoDl" "sendrecvProcNullNoDl.c"  3
    ""
    "Error;ERROR;Warning;WARNING"
    )


## waitAnySourceEx2Dl
## TODO this is currently not supported!
addDDlTest ("DDlwaitAnySourceEx2Dl" "waitAnySourceEx2Dl.c"  3
    "Information.*MUST issued a deadlock detection while a wildcard receive call.*The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## waitAnySourceEx3Dl
## TODO this is currently not supported!
addDDlTest ("DDlwaitAnySourceEx3Dl" "waitAnySourceEx3Dl.c"  3
    "Information.*MUST issued a deadlock detection while a wildcard receive call.*The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## waitAnySourceEx4Dl
## TODO this is currently not supported!
addDDlTest ("DDlwaitAnySourceEx4Dl" "waitAnySourceEx4Dl.c"  4
    "Information.*MUST issued a deadlock detection while a wildcard receive call.*The application issued a set of MPI calls that can cause a deadlock!.*MPI_Recv"
    ""
    )


## waitAnySourceEx5NoDlError
## TODO this is currently not supported!
addDDlTest ("DDlwaitAnySourceEx5NoDlError" "waitAnySourceEx5NoDlError.c"  4
    "The application fails to match a point-to-point operation before"
    ""
    )

    
    