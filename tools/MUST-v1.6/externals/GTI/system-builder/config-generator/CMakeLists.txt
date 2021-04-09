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
#       CMake file for config-generator directory.
#
# @author Tobias Hilbrich
# @date 02.11.2010

include(GNUInstallDirs)

#Look up the XML2 lib
FIND_PACKAGE(LibXml2 REQUIRED)

#Add the target
INCLUDE_DIRECTORIES(
    "${LIBXML2_INCLUDE_DIR}" 
    "${PROJECT_SOURCE_DIR}/modules/common"
    "../weaver/utility")
ADD_EXECUTABLE(
    modconfgen 
    ModConfGen.cpp
    "../weaver/utility/Verbose.cpp"
    "../weaver/utility/SpecificationNode.cpp")
TARGET_LINK_LIBRARIES(modconfgen ${LIBXML2_LIBRARIES})
SET_TARGET_PROPERTIES(modconfgen PROPERTIES COMPILE_FLAGS "${LIBXML2_DEFINITIONS}")

INSTALL(TARGETS modconfgen
    EXPORT modconfgen
    PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
        GROUP_EXECUTE GROUP_READ 
        WORLD_EXECUTE WORLD_READ
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
INSTALL(EXPORT modconfgen DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/GTI)
