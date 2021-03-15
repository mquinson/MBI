# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

##
# @file CMakeLists.cmake
#       CMake file for utility directory.
#
# @author Tobias Hilbrich
# @date 18.07.2012

OPTION (GTI_BUILD_PROFILE_EVALUATOR "Adds GTI's evaluator for its profiling data to the build, experimental." False)

ADD_SUBDIRECTORY (profile-evaluator)
