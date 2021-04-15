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
#       CMake file for the weaver directory.
#
# @author Tobias Hilbrich
# @date 12.01.2010

include(GNUInstallDirs)

#Look up the XML2 lib
FIND_PACKAGE(LibXml2 REQUIRED)

#Gather sources for individual namespaces
SET (ANALYSES_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/Analyses.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/Analysis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/AnalysisModule.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/InputDescription.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/AnalysisGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/Operation.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses/Calculation.cpp"
      CACHE INTERNAL ""
    )
    
SET (CALLS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/ApiCalls.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/ApiGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/Argument.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/Call.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/Input.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/Mapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/Record.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/ArgumentInput.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/CallNameInput.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/CallReturnInput.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/CallIdInput.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/ArrayArgument.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/ArrayArgumentOp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls/OperationInput.cpp"
      CACHE INTERNAL ""
    )
    
SET (GTI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/Gti.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/RequiresApi.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/Prepended.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/Configurable.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/CommProtocol.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/Module.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/CommStrategy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/CommStrategyIntra.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/SettingsDescription.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/EnumList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti/Place.cpp"
      CACHE INTERNAL ""
    )   
    
SET (LAYOUT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/layout/Setting.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/layout/Layout.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/layout/Communication.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/layout/Level.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/layout/Adjacency.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/layout/CallProperties.cpp"
      CACHE INTERNAL ""
    ) 
    
SET (UTILITY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/utility/Verbose.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/utility/SpecificationNode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utility/Printable.cpp"
      CACHE INTERNAL ""
    )

SET (GENERATION_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/generation/GenerationBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/generation/Wrapper.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/generation/ReceiveForwarding.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/generation/ModuleConfig.cpp"
      CACHE INTERNAL ""
    )

SET (SOURCES
    ${ANALYSES_SOURCES}
    ${CALLS_SOURCES}
    ${GTI_SOURCES}
    ${LAYOUT_SOURCES}
    ${UTILITY_SOURCES}
    ${GENERATION_SOURCES}
    "main.cpp")

#Add the target
SET (WEAVER_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/analyses"
    "${CMAKE_CURRENT_SOURCE_DIR}/calls"
    "${CMAKE_CURRENT_SOURCE_DIR}/gti"
    "${CMAKE_CURRENT_SOURCE_DIR}/layout"
    "${CMAKE_CURRENT_SOURCE_DIR}/utility"
    "${CMAKE_CURRENT_SOURCE_DIR}/generation"
    "${LIBXML2_INCLUDE_DIR}" 
    "${PROJECT_SOURCE_DIR}/modules/interfaces"
    "${PROJECT_SOURCE_DIR}/modules/common"
    "${PROJECT_BINARY_DIR}/system-builder/weaver" CACHE INTERNAL "")

#Configure the main.h header from main.h.in
CONFIGURE_FILE ("main.h.in" "${PROJECT_BINARY_DIR}/system-builder/weaver/main.h" ESCAPE_QUOTES @ONLY)
  
INCLUDE_DIRECTORIES(
    ${WEAVER_INCLUDE_DIRS})

FOREACH (bin weaver weaver-no-build)

   ADD_EXECUTABLE(${bin} ${SOURCES})
   TARGET_LINK_LIBRARIES(${bin} ${LIBXML2_LIBRARIES})
   SET_TARGET_PROPERTIES(${bin} PROPERTIES COMPILE_FLAGS "${LIBXML2_DEFINITIONS}")

   INSTALL(TARGETS ${bin}
       EXPORT weaver
       PERMISSIONS 
           OWNER_READ OWNER_WRITE OWNER_EXECUTE 
           GROUP_EXECUTE GROUP_READ 
           WORLD_EXECUTE WORLD_READ
       RUNTIME DESTINATION bin
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib
       )
ENDFOREACH (bin)

target_compile_definitions(weaver-no-build PUBLIC "-DPOST_BUILD_ONLY")

INSTALL(EXPORT weaver DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/GTI)
