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
#       CMake file for the doc directory.
#
# @author Tobias Hilbrich
# @date 07.06.2011

CONFIGURE_FILE(developer.doxyfile ${CMAKE_CURRENT_BINARY_DIR}/developer.doxyfile @ONLY)
