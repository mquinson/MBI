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
#       GTI core CMake file.
#
# @author Tobias Hilbrich
# @date 26.02.2009

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

#===================================
# Policies
#===================================
FOREACH (POL CMP0048 CMP0074)
    IF (POLICY ${POL})
        CMAKE_POLICY(SET ${POL} NEW)
    ENDIF ()
ENDFOREACH ()


#===================================
# Basic initialization
#===================================

PROJECT(GTI C CXX)

SET(CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmakemodules"
    "${PROJECT_SOURCE_DIR}/externals/CMake-gitinfo/cmake"
    "${PROJECT_SOURCE_DIR}/externals/CMake-gitpack/cmake"
    ${CMAKE_MODULE_PATH})

# Get the version info from git.
FIND_PACKAGE(GitInfo REQUIRED)
GIT_VERSION_INFO(GTI REQUIRED)


#===================================
#Fortran
#===================================
# Workaround for this CMake issue: http://public.kitware.com/Bug/view.php?id=9220
# Ensures that enable_language(Fortran OPTIONAL) works correctly.  Annoying.
if (DEFINED CMAKE_Fortran_COMPILER AND CMAKE_Fortran_COMPILER MATCHES "^$")
  set(CMAKE_Fortran_COMPILER CMAKE_Fortran_COMPILER-NOTFOUND)
endif()
if (ENABLE_FORTRAN)
enable_language(Fortran OPTIONAL)

IF (CMAKE_Fortran_COMPILER_WORKS)
  SET (GTI_ENABLE_FORTRAN ON)
ELSE (CMAKE_Fortran_COMPILER_WORKS)
  SET (GTI_ENABLE_FORTRAN OFF)
ENDIF (CMAKE_Fortran_COMPILER_WORKS)
endif()
#===================================
#Frontend/Backend
#===================================
OPTION (GTI_ONLY_BUILD_FRONTEND "For frontend/backend systems use two build directories that install to the same install prefix; Use GTI_ONLY_BUILD_FRONTEND in the first (with frontend compilers), and use GTI_ONLY_BUILD_BACKEND in the second (with the cross compilers)." FALSE)
OPTION (GTI_ONLY_BUILD_BACKEND "For frontend/backend systems use two build directories that install to the same install prefix; Use GTI_ONLY_BUILD_FRONTEND in the first (with frontend compilers), and use GTI_ONLY_BUILD_BACKEND in the second (with the cross compilers)." FALSE)

SET (BUILD_FRONTEND TRUE)
SET (BUILD_BACKEND TRUE)

IF (GTI_ONLY_BUILD_FRONTEND)
    SET (BUILD_BACKEND FALSE)
ENDIF()

IF (GTI_ONLY_BUILD_BACKEND)
    SET (BUILD_FRONTEND FALSE)
ENDIF()

IF (GTI_ONLY_BUILD_FRONTEND AND GTI_ONLY_BUILD_BACKEND)
    MESSAGE (FATAL_ERROR "Either specify GTI_ONLY_BUILD_FRONTEND or GTI_ONLY_BUILD_BACKEND!")
ENDIF()


#Include helper modules
INCLUDE (HelperMacros)

#Set prefix
#CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT is a hack and can be googled, not sure whether it is going to stay
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  IF (WIN32)
    SET(CMAKE_INSTALL_PREFIX "$ENV{PROGRAMFILES}/gti" CACHE PATH "Installation directory")
  ELSE (WIN32)
    SET(CMAKE_INSTALL_PREFIX "/usr/local/gti" CACHE PATH "Installation directory")
  ENDIF (WIN32)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#Binary dir stuff
SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Directory for built libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for built executables.")
MARK_AS_ADVANCED (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

#===================================
# Compiler options
#===================================

# Set MPI skip flags for C++ code
#
# Some MPI implementations use a legacy C++ header for mapping MPI C calls to
# C++ methods. Sometimes, code compiled with this "automatic feature" fails when
# executed due unresolvable symbols. The following flags will disable this
# "feature" entirely for all C++ code compiled in this project.
SET(MPI_CXX_SKIP_FLAGS "-DMPI_NO_CPPBIND"      # SGI
                       "-DOMPI_SKIP_MPICXX"    # OpenMPI
                       "-D_MPICC_H"            # HP-MPI
                       "-DMPICH_SKIP_MPICXX"   # MPICH
                       "-DMPIBULL_SKIP_MPICXX" # BUll-MPI
    CACHE STRING "Flags to skip C++ MPI")
STRING(REPLACE ";" " " CMAKE_CXX_FLAGS
       "${CMAKE_CXX_FLAGS} ${MPI_CXX_SKIP_FLAGS}")


#===================================
# System inspection
#===================================
IF (BUILD_BACKEND)
    if (DEFINED ENV{MPICC})
        set(MPI_C_COMPILER "$ENV{MPICC}" CACHE FILEPATH "MPI C Compiler")
    endif()
    if (DEFINED ENV{MPICXX})
        set(MPI_CXX_COMPILER "$ENV{MPICXX}" CACHE FILEPATH "MPI C++ Compiler")
    endif()
    if (DEFINED ENV{MPIFC})
        set(MPI_Fortran_COMPILER "$ENV{MPIFC}" CACHE FILEPATH "MPI Fortran
    Compiler")
    endif()
    if (DEFINED ENV{MPIEXEC})
        set(MPIEXEC "$ENV{MPIEXEC}" CACHE FILEPATH "mpiexec Command")
    endif()

    FIND_PACKAGE(MPI REQUIRED)
    FOREACH (lan C CXX Fortran)
        SET (MPI_${lan}_LIB_PATHS "")
        SET (MPI_${lan}_LIB_PATHS_COLUMN "")
        FOREACH (lib ${MPI_C_LIBRARIES})
            STRING (REGEX REPLACE "([/].*[/])[^/]*" "\\1" temp "${lib}")
            SET (MPI_${lan}_LIB_PATHS ${MPI_${lan}_LIB_PATHS} ${temp})
            IF (MPI_${lan}_LIB_PATHS_COLUMN)
                SET (MPI_${lan}_LIB_PATHS_COLUMN "${MPI_${lan}_LIB_PATHS_COLUMN}:${temp}")
            ELSE ()
                SET (MPI_${lan}_LIB_PATHS_COLUMN "${temp}")
            ENDIF ()
        ENDFOREACH ()
    ENDFOREACH ()
ENDIF (BUILD_BACKEND)

# Search for dependencies with fallbacks in the 'externals' subdirectory.
#
# Some of GTI's dependencies (like PnMPI) provide their resources as IMPORTED
# targets. As these are not available in parent scope and the dependencies can't
# be searched multiple times because of their fallbacks from the submodules,
# these need to be searched the project's root. By including the CMakeLists of
# 'externals' subdirectory, these will become visible for the entire project.
#
# NOTE: These dependencies need to be searched before recusing into the
#       subdirectories, so these are visible in them.
INCLUDE(externals/CMakeLists.txt)


#This would make sure we locate pthreads correctly; Not integrated yet!
## Additionally we need an: TARGET_LINK_LIBRARIES ( test ${CMAKE_THREAD_LIBS_INIT} )
## for all our modules to make sure we include a dependency to a potential pthread library.
#FIND_PACKAGE (Threads OPTIONAL)

#===================================
#Run feature tests
#===================================
INCLUDE (GtiFeaturetests)


#===================================
#Warnings
#===================================
OPTION (ENABLE_WARNINGS "Selects whether compiler warnings are enabled." FALSE)
IF (ENABLE_WARNINGS)
  SET(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")  
  #SET(MPI_CFLAGS "-Wall ${MPI_CFLAGS}")
ENDIF (ENABLE_WARNINGS)

IF(CMAKE_VERSION VERSION_LESS 3.1)
  OPTION (GTI_THREAD_SAFETY "Selects whether GTI should be built thread-safe." FALSE)
ELSE ()
  OPTION (GTI_THREAD_SAFETY "Selects whether GTI should be built thread-safe." TRUE)
ENDIF ()
IF (GTI_THREAD_SAFETY)
  CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
  SET(CMAKE_CXX_STANDARD 11)
  SET(CMAKE_CXX_STANDARD_REQUIRED true)
ENDIF (GTI_THREAD_SAFETY)


#===================================
# Recurse to sub-directories
#===================================
ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(legacy)
ADD_SUBDIRECTORY(modules)

IF (BUILD_FRONTEND)
    ADD_SUBDIRECTORY(system-builder)
    ADD_SUBDIRECTORY(utility)

    # The guis are not available in release tarballs, as these aren't ready for
    # production yet, so a check for their existance is needed before including
    # the subdirectory.
    IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/guis")
        ADD_SUBDIRECTORY(guis)
    ENDIF ()
ENDIF (BUILD_FRONTEND)

# Add GTI test cases.
#
# GTI test cases will be managed in the 'tests/' subdirectory. As
# 'ENABLE_TESTING()' needs to be called in the project's root directory,
# including the directory and related options will be managed here instead of
# inside the directory.
#
# NOTE: If GTI is included into other projects via 'add_subdirectory()', testing
#       will be disabled by CMake. Therefore, this directory doesn't need to be
#       included to avoid test binaries to be built.
OPTION(ENABLE_TESTS "Selects whether tests are built." OFF)
IF (ENABLE_TESTS AND BUILD_BACKEND
                 AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests)
ENDIF ()


# Install CMake project files
#
# Additional files will be installed, so other CMake projects are able to find
# (and use) GTI by simply calling 'find_package(GTI)'.
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GNUInstallDirs)

SET(GTI_VERSION_CONCAT "${GTI_VERSION_MAJOR}" "${GTI_VERSION_MINOR}"
                       "${GTI_VERSION_PATCH}" "${GTI_VERSION_TWEAK}")
STRING(REPLACE ";" "." GTI_VERSION_CONCAT "${GTI_VERSION_CONCAT}")
WRITE_BASIC_PACKAGE_VERSION_FILE(
    ${PROJECT_BINARY_DIR}/GTIConfigVersion.cmake
    VERSION ${GTI_VERSION_CONCAT}
    COMPATIBILITY SameMajorVersion
)
CONFIGURE_PACKAGE_CONFIG_FILE(
        GTIConfig.cmake.in
        GTIConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}
)

INSTALL(
    FILES
        ${PROJECT_BINARY_DIR}/GTIConfigVersion.cmake
        ${PROJECT_BINARY_DIR}/GTIConfig.cmake
        cmakemodules/HelperMacros.cmake
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/cmake/GTI
)


# Populate cache with GTI configuration.
#
# If GTI is included into other projects via add_subdirectory(), additional
# variables need to be populated in the cache, so GTI is able to be used from
# the parent directories.
IF (NOT PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    SET(GTI_FOUND         TRUE PARENT_SCOPE)
    SET(GTI_IS_SUBPROJECT TRUE)
    INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/GTIConfig.cmake")

    # If GTI is built with thread safety, the parent project also needs to be
    # build with C++11 standard for compatibility with GTI's headers.
    IF (GTI_IS_THREAD_SAFE)
        CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
        SET(CMAKE_CXX_STANDARD          11   PARENT_SCOPE)
        SET(CMAKE_CXX_STANDARD_REQUIRED TRUE PARENT_SCOPE)
    ENDIF ()
endif ()


# CPack configuration for packaging sources.
#
# If GTI is NOT included into other projects (via ADD_SUBDIRECTORY()), configure
# CPack to pack the sources. The package may be generated by building the
# 'package_deploy' target.
IF (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    SET(CPACK_PACKAGE_VERSION          "${GTI_VERSION}")
    SET(CPACK_PACKAGE_VERSION_MAJOR    ${GTI_VERSION_MAJOR})
    SET(CPACK_PACKAGE_VERSION_MINOR    ${GTI_VERSION_MINOR})
    SET(CPACK_PACKAGE_VERSION_PATCH    ${GTI_VERSION_PATCH})

    STRING(REGEX MATCH "^v[0-9]+.[0-9]+(.[0-9]+)*(-rc[0-9]+)"
                       GTI_VERSION_SHORT "${GTI_VERSION}")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "GTI-${GTI_VERSION_SHORT}-full")

    INCLUDE(GitPack)
    INCLUDE(CPackDeploy)
ENDIF ()

#===================================
#Write out the configuration of the found packages 
#such that the dynamic build tools work
#===================================
IF (BUILD_BACKEND)
    CONFIGURE_FILE(cmakemodules/InstallationConfiguration.cmake.in ${PROJECT_BINARY_DIR}/cmakemodules/InstallationConfiguration.cmake @ONLY)
    INSTALL(FILES cmakemodules/HelperMacros.cmake ${PROJECT_BINARY_DIR}/cmakemodules/InstallationConfiguration.cmake DESTINATION share/cmake/GTI)
ENDIF (BUILD_BACKEND)
