# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

INCLUDE(GNUInstallDirs)


IF (BUILD_BACKEND)
#    CONFIGURE_FILE(HelperMacros.cmake.in HelperMacros.cmake)
#    CONFIGURE_FILE(InstallationConfiguration.cmake.in
#                   InstallationConfiguration.cmake)
#
#    INSTALL(FILES
#            ${CMAKE_CURRENT_BINARY_DIR}/InstallationConfiguration.cmake
#            ${CMAKE_CURRENT_BINARY_DIR}/HelperMacros.cmake
#        DESTINATION cmakemodules)
ENDIF ()
