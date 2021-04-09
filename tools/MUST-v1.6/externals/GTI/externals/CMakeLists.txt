# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

INCLUDE(FindPackageMessage)


# Check that the externals directory is not included via 'ADD_SUBDIRECTORY()'.
#
# Some of GTI's dependencies (like PnMPI) provide their resources as IMPORTED
# targets. As these are not available in parent scope and the dependencies can't
# be searched multiple times because of their fallbacks from the submodules,
# this CMake file needs to be included in the project's root.
IF (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR)
    MESSAGE(FATAL_ERROR "The 'externals' directory MUST NOT be included via "
                        "ADD_SUBDIRECTORY but by including its CMakeLists.txt "
                        "via INCLUDE().")
ENDIF()

SET(SOURCE_DIR "${PROJECT_SOURCE_DIR}/externals")
SET(BINARY_DIR "${PROJECT_BINARY_DIR}/externals")


# Search for PnMPI.
#
# By default, an existing and installed version of PnMPI will be used for GTI.
# However, if PnMPI is not installed at this system, the whole PnMPI project
# will be included as submodule.
#
# NOTE: As PnMPI is licensed under the LGPL, contents of the submodule MUST NOT
#       be shipped along with GTI (e.g. as tarball) and therefore must be
#       excluded from any packed version of GTI. However, users may still drop
#       the contents of PnMPI into the subdirectory to import it as a fallback.
#
# NOTE: If PnMPI will be build from sources, future calls of configure will NOT
#       search for an installed version of PnMPI to avoid a previously build
#       version in GTI's install prefix to be used.
IF (NOT DEFINED PnMPI_IS_SUBPROJECT)
    FIND_PACKAGE(PnMPI 1.7.1 QUIET
        HINTS
            "${PnMPI_INSTALL_PREFIX}"
        )
ENDIF ()
IF (PnMPI_FOUND)
    # As FIND_PACKAGE() needs to be called with the 'QUIET' flag to hide errors
    # about not found packages, we'll have to print the success message now.
    FIND_PACKAGE_MESSAGE(PnMPI "Found PnMPI: ${PnMPI_VERSION}"
                               "system.${PnMPI_VERSION}")
ELSE ()
    # If PnMPI is not installed at the system and its sources are not available
    # in the subdirectory, the user either needs to drop the PnMPI sources into
    # this subdirectory, or (if GTI sources are fetched from git) checkout the
    # git submodules.
    IF (NOT EXISTS ${SOURCE_DIR}/PnMPI/CMakeLists.txt)
      MESSAGE(FATAL_ERROR "Could NOT find PnMPI or its sources. Try updating "
                          "the git submodules or drop the sources of PnMPI "
                          "into ${SOURCE_DIR}/PnMPI")
    ENDIF ()

    # Add PnMPI as subproject. As PnMPI's CMake configuration exports its full
    # configuration, no further steps are needed to use PnMPI.
    ADD_SUBDIRECTORY(${SOURCE_DIR}/PnMPI)
    SET(PnMPI_IS_SUBPROJECT TRUE CACHE BOOL "Wheter PnMPI sources will be used")
    MARK_AS_ADVANCED(PnMPI_IS_SUBPROJECT)
ENDIF ()


# Search for wrap.
#
# By default, an existing and installed version of wrap will be used for GTI.
# However, if wrap is not installed at this system, the wrap sources will be
# included as submodule.
#
# NOTE: If wrap has already been included by the PnMPI sources, it MUST NOT be
#       included a second time to avoid "duplicate target" errors by CMake.
FIND_PACKAGE(wrap QUIET)
IF (wrap_FOUND)
    # As FIND_PACKAGE() needs to be called with the 'QUIET' flag to hide errors
    # about not found packages, we'll have to print the success message now.
    FIND_PACKAGE_MESSAGE(wrap "Found wrap: ${Wrap_EXECUTABLE}"
                              "system.${Wrap_EXECUTABLE}")
ELSE ()
    # If wrap is not installed at the system and its sources are not available
    # in the subdirectory, the user either needs to drop the wrap sources into
    # this subdirectory, or (if GTI sources are fetched from git) checkout the
    # git submodules.
    IF (NOT EXISTS ${SOURCE_DIR}/wrap/CMakeLists.txt)
      MESSAGE(FATAL_ERROR "Could NOT find wrap or its sources. Try updating "
                          "the git submodules or drop the sources of wrap into "
                          "${SOURCE_DIR}/wrap")
    ENDIF ()

    # Add wrap as submodule and print a message with its path to CMake's
    # console. This message will be printed only once.
    ADD_SUBDIRECTORY(${SOURCE_DIR}/wrap EXCLUDE_FROM_ALL)
    FIND_PACKAGE_MESSAGE(wrap
      "Found wrap: ${SOURCE_DIR}/wrap/wrap.py"
      "submodule.${SOURCE_DIR}/wrap/wrap.py")

    # Initialize the wrap configuration and include the generated configuration
    # file. This includes the required 'add_wrapped_file()' function.
    SET(WRAP ${SOURCE_DIR}/wrap/wrap.py)
    SET(GETSTATUSSIZE ${SOURCE_DIR}/wrap/getStatusSize.f)
    CONFIGURE_FILE(${SOURCE_DIR}/wrap/wrap-config.cmake.in
                   ${BINARY_DIR}/wrap/wrap-config.cmake
                   @ONLY)
    INCLUDE(${BINARY_DIR}/wrap/wrap-config.cmake)
ENDIF ()
