# This file is part of CMake-gitpack.
#
# Copyright (C)
#   2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details.

# Enhance `CPack` with ignore patterns from `.gitattributes`.
#
# By default, `CPack` just packs the source directory's contents. However,
# projects using git want to exclude the files with `export-ignore` attribute
# set. Including this file instead of `CPack` will ensure, the specified files
# will be ignored for packaging.
#
# NOTE: If this file is used from sources already packed, the ignores will have
#       no effect and it will behave just like the regular `CPack` file.

# Set the minimum required CMake version.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Ignore some basic files, like the build and install directories and the git
# directory (only neccessary, if packing from inside a git repositoy).
#
# NOTE: CPack packs EVERYTHING that's inside the source directory. To prevent
#       custom files from being packed, add these to 'CPACK_SOURCE_IGNORE_FILES'
#       at configuration time.
SET(CPACK_SOURCE_IGNORE_FILES
    "\\\\.git"
    ${PROJECT_BINARY_DIR}
    ${CMAKE_INSTALL_PREFIX}
    ${CPACK_SOURCE_IGNORE_FILES})

# Ignore all files with 'export-ignore' attribute set.
#
# Instead of mainaining two sets of exclude lists, get all ignores defined in
# the '.gitattributes' files for the project's repositoy and all of its
# submodules.
#
# NOTE: If no '.gitattributes' file is available (i.e. the project is build
#       from packed sources), these ignore patterns simply have no effect, as
#       the files are already excluded from the sources.
FILE(GLOB_RECURSE ATTRIBUTE_FILES ".gitattributes")
IF (ATTRIBUTE_FILES)
    FOREACH (ATTRIBUTE_FILE ${ATTRIBUTE_FILES})
        GET_FILENAME_COMPONENT(GIT_ROOT "${ATTRIBUTE_FILE}" PATH)

        # Read the '.gitattributes' file and convert its contents into a CMake
        # list (where each element in the list is one line of the file).
        #
        # This solution has been taken from the CMake mailing list at:
        # https://cmake.org/pipermail/cmake/2007-May/014222.html
        FILE(READ "${ATTRIBUTE_FILE}" ATTRIBUTES)
        STRING(REGEX REPLACE ";" "\\\\;" ATTRIBUTES "${ATTRIBUTES}")
        STRING(REGEX REPLACE "\n" ";" ATTRIBUTES "${ATTRIBUTES}")

        # Iterate over the lines in the '.gitattributes' file and add all
        # paterns to be ignored to CPack's ignore list. The patterns will be
        # prefixed with the repository's root to only affect files inside this
        # repositoy.
        FOREACH (LINE ${ATTRIBUTES})
            IF (LINE MATCHES "(.*)export-ignore")
                STRING(STRIP "${CMAKE_MATCH_1}" PATTERN)
                IF (PATTERN MATCHES "^/")
                    SET(PATTERN "${GIT_ROOT}${PATTERN}")
                ELSE ()
                    SET(PATTERN "${GIT_ROOT}.*/${PATTERN}")
                ENDIF ()

                LIST(APPEND CPACK_SOURCE_IGNORE_FILES "${PATTERN}")
            ENDIF ()
        ENDFOREACH ()
    ENDFOREACH ()
ENDIF ()

# Include the regular CPack configuration file. This will initialize the default
# CPack targets and configuration files.
INCLUDE(CPack)
