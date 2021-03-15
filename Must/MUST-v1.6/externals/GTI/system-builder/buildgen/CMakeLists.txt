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
#       CMake file for buildgen directory.
#
# @author Tobias Hilbrich
# @date 19.08.2010

include(GNUInstallDirs)

#Look up the XML2 lib
FIND_PACKAGE(LibXml2 REQUIRED)

#Add the target
CONFIGURE_FILE(config.h.in config.h)
INCLUDE_DIRECTORIES(
    "${LIBXML2_INCLUDE_DIR}" 
    "${PROJECT_SOURCE_DIR}/modules/common"
    "../weaver/utility"
    ${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(
    buildgen 
    BuildGen.cpp
    "../weaver/utility/Verbose.cpp"
    "../weaver/utility/SpecificationNode.cpp")
TARGET_LINK_LIBRARIES(buildgen ${LIBXML2_LIBRARIES})
SET_TARGET_PROPERTIES(buildgen PROPERTIES COMPILE_FLAGS "${LIBXML2_DEFINITIONS}")

INSTALL(TARGETS buildgen
    EXPORT buildgen
    PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
        GROUP_EXECUTE GROUP_READ 
        WORLD_EXECUTE WORLD_READ
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
INSTALL(EXPORT buildgen DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/GTI)
