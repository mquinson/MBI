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
#       CMake file for system-builder/dtd directory.
#
# @author Tobias Hilbrich
# @date 03.01.2011

#Configure and install GTI specification
INSTALL (FILES analysis-specification.dtd api-specification.dtd gti-specification.dtd layout-specification.dtd module-config.dtd module-config-merge.dtd receival-gen-input.dtd receival-gen-output.dtd wrapp-gen-input.dtd wrapp-gen-output.dtd DESTINATION specifications)