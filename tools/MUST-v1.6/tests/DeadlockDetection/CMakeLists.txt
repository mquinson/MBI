# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for deadlock detection tests.
#
# @author Tobias Hilbrich
# @date 26.07.2011
#

##################
#Recurse into sub directories
ADD_SUBDIRECTORY(P2PMatch)
ADD_SUBDIRECTORY(CollectiveMatch)
ADD_SUBDIRECTORY(DeadlockDetection)
ADD_SUBDIRECTORY(DeadlockView)
ADD_SUBDIRECTORY(DP2PMatch)
ADD_SUBDIRECTORY(DCollectiveMatch)

#Only go into NBC tests if we got all of them
IF (HAVE_MPI_IALLGATHER AND 
    HAVE_MPI_IALLGATHERV AND
    HAVE_MPI_IALLREDUCE AND
    HAVE_MPI_IALLTOALL AND 
    HAVE_MPI_IALLTOALLV AND 
    HAVE_MPI_IALLTOALLW AND
    HAVE_MPI_IBARRIER AND
    HAVE_MPI_IBCAST AND
    HAVE_MPI_IGATHER AND
    HAVE_MPI_IGATHERV AND
    HAVE_MPI_IREDUCE AND
    HAVE_MPI_IREDUCE_SCATTER AND
    HAVE_MPI_IREDUCE_SCATTER_BLOCK AND
    HAVE_MPI_ISCAN AND
    HAVE_MPI_IEXSCAN AND
    HAVE_MPI_ISCATTER AND
    HAVE_MPI_ISCATTERV)
    ADD_SUBDIRECTORY(NbcCollectiveMatch)
ENDIF ()