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
#       CMake file for wrapper-generator directory.
#
# @author Tobias Hilbrich
# @date 11.08.2010

include(GNUInstallDirs)

#Look up the XML2 lib
FIND_PACKAGE(LibXml2 REQUIRED)

#TODO remove that once that P^nMPI stops to require MPI
INCLUDE_DIRECTORIES (${PnMPI_INCLUDE_DIR} ${MPI_C_INCLUDE_PATH})

FOREACH (lan C CXX Fortran)
    SET (${lan}_compiler_to_use ${CMAKE_${lan}_COMPILER})
    IF (MPI_${lan}_COMPILER)
        SET (${lan}_compiler_to_use ${MPI_${lan}_COMPILER})
    ENDIF ()
ENDFOREACH ()

SET(CMAKE_C_COMPILER ${C_compiler_to_use})
SET(CMAKE_CXX_COMPILER ${CXX__compiler_to_use})
SET(CMAKE_EXE_LINKER_FLAGS ${MPI_C_LINK_FLAGS})

#Add the target
INCLUDE_DIRECTORIES(
    "${LIBXML2_INCLUDE_DIR}" 
    "${PROJECT_SOURCE_DIR}/modules/interfaces"
    "${PROJECT_SOURCE_DIR}/modules/common"
    "${PROJECT_BINARY_DIR}/modules/common"
    "../weaver/utility")
#LINK_DIRECTORIES("${PnMPI_LIBRARY_DIR}")
ADD_EXECUTABLE(
    wrappgen 
    WrapperGenerator.cpp
    GeneratorBase.cpp
    "../weaver/utility/Verbose.cpp"
    "../weaver/utility/SpecificationNode.cpp")
TARGET_LINK_LIBRARIES(wrappgen ${LIBXML2_LIBRARIES})
TARGET_LINK_LIBRARIES(wrappgen dl)
SET_TARGET_PROPERTIES(wrappgen PROPERTIES COMPILE_FLAGS "${LIBXML2_DEFINITIONS}")

INSTALL(TARGETS wrappgen
    EXPORT wrappgen
    PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
        GROUP_EXECUTE GROUP_READ 
        WORLD_EXECUTE WORLD_READ
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
INSTALL(EXPORT wrappgen DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/GTI)
