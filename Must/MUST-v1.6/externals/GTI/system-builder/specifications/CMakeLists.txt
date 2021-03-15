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
#       CMake file for system-builder/specifications directory.
#
# @author Tobias Hilbrich
# @date 03.01.2011

#Configure and install GTI specification
CONFIGURE_FILE(gti-specification.xml ${PROJECT_BINARY_DIR}/system-builder/specifications/gti-specification.xml @ONLY)
CONFIGURE_FILE(gti-implicit-analysis-spec.xml ${PROJECT_BINARY_DIR}/system-builder/specifications/gti-implicit-analysis-spec.xml @ONLY)
CONFIGURE_FILE(gti-implicit-api-spec.xml ${PROJECT_BINARY_DIR}/system-builder/specifications/gti-implicit-api-spec.xml @ONLY)

INSTALL (FILES ${PROJECT_BINARY_DIR}/system-builder/specifications/gti-specification.xml DESTINATION specifications)
INSTALL (FILES ${PROJECT_BINARY_DIR}/system-builder/specifications/gti-implicit-analysis-spec.xml DESTINATION specifications)
INSTALL (FILES ${PROJECT_BINARY_DIR}/system-builder/specifications/gti-implicit-api-spec.xml DESTINATION specifications)
