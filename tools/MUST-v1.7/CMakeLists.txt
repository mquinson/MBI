# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details
#
# @file CMakeLists.cmake
#       MUST core CMake file.
#
# @author Tobias Hilbrich
# @author Alexander Haase <alexander.haase@rwth-aachen.de>
#
# @date 23.06.2014

CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

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

PROJECT(MUST C CXX)

SET(CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmakemodules"
    "${CMAKE_SOURCE_DIR}/externals/CMake-gitinfo/cmake"
    "${CMAKE_SOURCE_DIR}/externals/CMake-gitpack/cmake"
    "${CMAKE_SOURCE_DIR}/externals/CMake-MPIhelper/cmake"
    ${CMAKE_MODULE_PATH}
)

# Get the version info from git.
FIND_PACKAGE(GitInfo REQUIRED)
GIT_VERSION_INFO(MUST REQUIRED)

# Optional Fortran support.
#
# By default MUST and its dependencies will be built with Fortran support
# enabled. This requires a Fortran compiler and Fortran-enabled versions of MPI,
# GTI and PnMPI. If one of the conditions is not matched, or one wants to build
# MUST without Fortran support, 'ENABLE_FORTRAN' needs to be disabled at
# configuration time.
OPTION(ENABLE_FORTRAN "Build MUST with Fortran support." ON)
IF (ENABLE_FORTRAN)
    ENABLE_LANGUAGE(Fortran)
ENDIF ()


#===================================
#Warnings
#===================================
OPTION (ENABLE_WARNINGS "Selects whether compiler warnings are enabled." FALSE)
IF (ENABLE_WARNINGS)
  SET(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
ENDIF (ENABLE_WARNINGS)


#Include helper modules
INCLUDE (MTHelperMacros)

#Set prefix
#CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT is a hack and can be googled, not sure whether it is going to stay
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  IF (WIN32)
    SET(CMAKE_INSTALL_PREFIX "$ENV{PROGRAMFILES}/must" CACHE PATH "Installation directory" FORCE)
  ELSE (WIN32)
    SET(CMAKE_INSTALL_PREFIX "/usr/local/must" CACHE PATH "Installation directory" FORCE)
  ENDIF (WIN32)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#Binary dir stuff
SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Directory for built libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for built executables.")
MARK_AS_ADVANCED (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

#Backup of compilers
SET (MUST_C_COMPILER ${CMAKE_C_COMPILER} CACHE INTERNAL "")
SET (MUST_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE INTERNAL "")
SET (MUST_Fortran_COMPILER ${CMAKE_Fortran_COMPILER} CACHE INTERNAL "")

OPTION(USE_CALLPATH "Enable stacktraces with the stackwalker from dyninst, this requires you to set multiple paths that start with \"CALLPATH_\", as there is no easy way to link to that yet." FALSE)


#===================================
# Compiler options
#===================================

# Set MPI skip flags for C++ code
#
# Some MPI implementations use a legacy C++ header for mapping MPI C calls to
# C++ methods. Sometimes, code compiled with this "automatic feature" fails when
# executed due unresolvable symbols. The following flags will disable this
# "feature" entirely for all C++ code compiled in this project.
set(MPI_CXX_SKIP_FLAGS "-DMPI_NO_CPPBIND"      # SGI
                       "-DOMPI_SKIP_MPICXX"    # OpenMPI
                       "-D_MPICC_H"            # HP-MPI
                       "-DMPICH_SKIP_MPICXX"   # MPICH
                       "-DMPIBULL_SKIP_MPICXX" # BUll-MPI
    CACHE STRING "Flags to skip C++ MPI")
string(REPLACE ";" " " CMAKE_CXX_FLAGS
       "${CMAKE_CXX_FLAGS} ${MPI_CXX_SKIP_FLAGS}")


#===================================
# System inspection
#===================================

# Search for general required packages.
#
# Some packages are required in several subdirectories. These will be searched
# here inside the project's root, so all subdirectories may use the cached
# results. Otherwise CMake may slow down, as e.g. 'FIND_PACKAGE(MPI)' gets
# called several times for the feature tests, which can't share the results if
# called inside the feature test.
FIND_PACKAGE(MPI REQUIRED)

# Search for dependencies with fallbacks in the 'externals' subdirectory.
#
# Some of MUST's dependencies (like GTI and PnMPI) provide their resources as
# IMPORTED targets. As these are not available in parent scope and the
# dependencies can't be searched multiple times because of their fallbacks from
# the submodules, these need to be searched the project's root. By including the
# CMakeLists of 'externals' subdirectory, these will become visible for the
# entire project.
#
# NOTE: These dependencies need to be searched before recusing into the
#       subdirectories, so these are visible in them.
include(externals/CMakeLists.txt)

# Check if GTI matches the configuration of MUST.
#
# If GTI doesn't support features required by the current MUST configuration,
# abort configuring MUST and tell the user what features can't be used.
IF (ENABLE_FORTRAN AND NOT GTI_ENABLE_FORTRAN)
    MESSAGE(FATAL_ERROR "The Fortran features of MUST require a version of GTI "
                        "with Fortran support enabled.")
ENDIF ()


#===================================
#Find necessary software/libraries/environmental stuff
#===================================
FIND_PACKAGE(AWK REQUIRED)
FIND_PACKAGE(LDD QUIET)
FIND_PACKAGE(OTOOL QUIET)
FIND_PACKAGE(MD5SUM QUIET)
FIND_PACKAGE(BASH REQUIRED)
FIND_PACKAGE(DOT)
FIND_PACKAGE(OpenMP QUIET)
IF (NOT OPENMP_FOUND)
  MESSAGE (STATUS "Compiler support for OpenMP support not found - disabling optional OpenMP features")
ENDIF (NOT OPENMP_FOUND)

IF ( NOT OTOOL_FOUND AND NOT LDD_FOUND )
      MESSAGE(FATAL_ERROR  "Could neither find a ldd nor an otool, please install either one!")
ENDIF( NOT OTOOL_FOUND AND NOT LDD_FOUND )
IF (USE_CALLPATH) ##The prefix/postifx is used in XML specification to adapt them to the availability of callpaths
	FIND_PACKAGE(Stackwalker)
ENDIF()


#===================================
# Set DTD-Path for specifications
#===================================
GET_FILENAME_COMPONENT(MUST_SPECIFICATION_DTD_PATH
                       "${GTI_SPECIFICATION}" DIRECTORY CACHE)


#===================================
#Include sub-directories
#===================================
IF (USE_CALLPATH) ##The prefix/postifx is used in XML specification to adapt them to the availability of callpaths
    SET (USE_CALLPATH_XML_PREFIX "")
    SET (USE_CALLPATH_XML_SUFFIX "")
ELSE (USE_CALLPATH)
    SET (USE_CALLPATH_XML_PREFIX "<!--")
    SET (USE_CALLPATH_XML_SUFFIX "-->")
ENDIF (USE_CALLPATH)
INCLUDE (MustFeaturetests)
ADD_SUBDIRECTORY(specifications)
ADD_SUBDIRECTORY(doxygen)
ADD_SUBDIRECTORY(utility)
ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(modules)
ADD_SUBDIRECTORY(prebuild)

#===================================
#Tests
#===================================
OPTION (ENABLE_TESTS "Selects whether tests are built." FALSE)
IF (ENABLE_TESTS)
    OPTION (DISABLE_UMPIRE_TESTS "Selects whether umpire tests are disabled." FALSE)
    OPTION (DISABLE_MARMOT_TESTS "Selects whether marmot tests are disabled." FALSE)
    INCLUDE (CTest)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests)
ENDIF (ENABLE_TESTS)


# CPack configuration for packaging sources.
#
# If MUST is NOT included into other projects (via add_subdirectory()),
# configure CPack to pack the sources. The package may be generated by building
# the 'package_deploy' target.
if (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CPACK_PACKAGE_VERSION          "${MUST_VERSION}")
  set(CPACK_PACKAGE_VERSION_MAJOR    ${MUST_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR    ${MUST_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH    ${MUST_VERSION_PATCH})
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "MUST-${MUST_VERSION}")

  include(GitPack)
  include(CPackDeploy)
endif ()
