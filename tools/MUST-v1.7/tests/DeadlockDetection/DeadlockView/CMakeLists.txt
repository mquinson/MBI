# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for deadlock view tests that are part of deadlock detection.
#
# IMPORTANT: these tests are only semi-automatic, the format and appearence of the deadlock
#                     view is stressed here, which is not automatically checked.
#
# @author Tobias Hilbrich
# @date 17.10.2012
#

###################
#Add test cases
addProcessLocalTest ("DLViewCommmismatch" "DLViewCommmismatch.c"  2
    "The application issued a set of MPI calls that can cause a deadlock!."
    ""
    )

addProcessLocalTest ("DLViewTagmismatch" "DLViewTagmismatch.c"  2
    "The application issued a set of MPI calls that can cause a deadlock!."
    ""
    )

addProcessLocalTest ("DLViewMulticommMultitag" "DLViewMulticommMultitag.c"  2
    "The application issued a set of MPI calls that can cause a deadlock!."
    ""
    )

addProcessLocalTest ("DLViewMultiLocationInMsgQueue" "DLViewMultiLocationInMsgQueue.c"  2
    "The application issued a set of MPI calls that can cause a deadlock!."
    ""
    )
    