# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.txt
#  CMake file for ELP directory
#
# @author Alexander Haase @date 06.05.2014
#

IF (NOT ENABLE_ELP_EXTENSIONS)
    RETURN()
ENDIF ()

# Recurse into sub directories
ADD_SUBDIRECTORY(data_dependencies)
ADD_SUBDIRECTORY(accelerator_race_condition)
ADD_SUBDIRECTORY(potential_deadlocks)
ADD_SUBDIRECTORY(api_missusage)
ADD_SUBDIRECTORY(allocation_failure)
