# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

INCLUDE(FindPackageMessage)


# Check that the externals directory is not included via 'ADD_SUBDIRECTORY()'.
#
# Some of MUST's dependencies (like GTI and PnMPI) provide their resources as
# IMPORTED targets. As these are not available in parent scope and the
# dependencies can't be searched multiple times because of their fallbacks from
# the submodules, this CMake file needs to be included in the project's root.
IF (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR)
    MESSAGE(FATAL_ERROR "The 'externals' directory MUST NOT be included via "
                        "ADD_SUBDIRECTORY but by including its CMakeLists.txt "
                        "via INCLUDE().")
ENDIF()

SET(SOURCE_DIR "${PROJECT_SOURCE_DIR}/externals")
SET(BINARY_DIR "${PROJECT_BINARY_DIR}/externals")


# Search for GTI.
#
# By default, an existing and installed version of GTI will be used for MUST.
# However, if GTI is not installed at this system, the GTI sources will be
# included as submodule.
#
# NOTE: The status of finding GTI in the system will be stored in a variable to
#       be used for deciding, if prebuilds can be build. These are not
#       compatible with building GTI from source right now.
#
# NOTE: If GTI will be build from sources, future calls of configure will NOT
#       search for an installed version of GTI to avoid a previously build
#       version in MUST's install prefix to be used.
IF (NOT DEFINED GTI_IS_SUBPROJECT)
    FIND_PACKAGE(GTI 1.6 QUIET
        HINTS
            "${GTI_INSTALL_PREFIX}"
        )
ENDIF ()
IF (GTI_FOUND)
    # As FIND_PACKAGE() needs to be called with the 'QUIET' flag to hide errors
    # about not found packages, we'll have to print the success message now.
    FIND_PACKAGE_MESSAGE(GTI "Found GTI: ${GTI_VERSION}"
                             "system.${GTI_VERSION}")
ELSE ()
    # If GTI is not installed at the system and its sources are not available in
    # the subdirectory, the user either needs to drop the PnMPI sources into
    # this subdirectory, or (if GTI sources are fetched from git) checkout the
    # git submodules.
    IF (NOT EXISTS ${SOURCE_DIR}/GTI/CMakeLists.txt)
      MESSAGE(FATAL_ERROR "Could NOT find GTI or its sources. Try updating the "
                          "git submodules or drop the sources of GTI into "
                          "${SOURCE_DIR}/GTI")
    ENDIF ()

    # Add GTI as subproject. As GTI's CMake configuration exports its full
    # configuration, no further steps are needed to use GTI.
    ADD_SUBDIRECTORY(${SOURCE_DIR}/GTI)
    SET(GTI_IS_SUBPROJECT TRUE CACHE BOOL "Wheter GTI sources will be used")
    MARK_AS_ADVANCED(GTI_IS_SUBPROJECT)
ENDIF ()


# Search for wrap.
#
# By default, an existing and installed version of wrap will be used for MUST.
# However, if wrap is not installed at this system, the wrap sources will be
# included as submodule.
#
# NOTE: If wrap has already been included by the GTI sources, it MUST NOT be
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
                            "the git submodules or drop the sources of wrap "
                            "into ${SOURCE_DIR}/wrap")
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
