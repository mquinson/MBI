# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

##
# @file HelperMacros.cmake
#       Contains helpful macros.
#
# @author Tobias Hilbrich
# @date 16.03.2009

#=========================================================
# Macro from GTI
# Does:
#   Gets the name of a library, Gets the path to the 
#   P^nMPI patcher, Creates a CMake script in the binary
#   directory which executes the patcher for the given 
#   library. The result is placed in the P^nMPI library 
#   directory. The script is added as a install script.
#   (executed at install time)
#=========================================================
MACRO (GTI_MAC_PATCH_LIB targetname patcher)
    IF (NOT EXISTS ${PROJECT_BINARY_DIR}/patched-libs/)
        file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/patched-libs/)
    ENDIF ()

    #TODO: test whether this works with Windows pathes (spaces and such)
    SET (lib ${CMAKE_SHARED_MODULE_PREFIX}${targetname}${CMAKE_SHARED_MODULE_SUFFIX}) 

#     MESSAGE (\"Patching ${lib}\")
    ADD_CUSTOM_COMMAND ( TARGET ${targetname}
		POST_BUILD
		COMMAND ${patcher} ${LIBRARY_OUTPUT_PATH}/${lib} ${PROJECT_BINARY_DIR}/patched-libs/${lib}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/patched-libs/
		COMMENT "Patching ${lib}")

    INSTALL(FILES ${PROJECT_BINARY_DIR}/patched-libs/${lib}
       PERMISSIONS 
           OWNER_READ OWNER_WRITE OWNER_EXECUTE 
           GROUP_EXECUTE GROUP_READ 
           WORLD_EXECUTE WORLD_READ
       DESTINATION modules
       )
ENDMACRO (GTI_MAC_PATCH_LIB)

#=========================================================
# Macro
# Does:
#   Takes a targetname and a list of source files.
#   Adds a module with given name and sources, installs 
#   patches and versions it.
# 
# language: in C, CXX, Fortran
#=========================================================
MACRO (GTI_MAC_ADD_MODULE targetname sources language)
    #Add target and its dependency on the patcher
    ADD_LIBRARY(${targetname} MODULE ${sources})
    
    #For Apple set that undefined symbols should be looked up dynamically
    #(On linux this is already the default)
    IF (APPLE)
        SET_PROPERTY(TARGET ${targetname} APPEND_STRING PROPERTY LINK_FLAGS " -undefined dynamic_lookup")
    ENDIF (APPLE)
    IF (GTI_THREAD_SAFETY)
        TARGET_LINK_LIBRARIES(${targetname} GtiTLS)
    ENDIF (GTI_THREAD_SAFETY)
    
    ##Not necessary: the script with the patcher is run anyways
    #Install it with reasonable file permissions
    #INSTALL(TARGETS ${targetname}
    #    PERMISSIONS 
    #        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
    #        GROUP_EXECUTE GROUP_READ 
    #        WORLD_EXECUTE WORLD_READ
    #    RUNTIME DESTINATION bin
    #    LIBRARY DESTINATION modules
    #    ARCHIVE DESTINATION modules
    #    )
        
    #Patch it during installation
    GTI_MAC_PATCH_LIB (
        ${targetname}
        ${PnMPI_PATCHER}
        )       
    IF ( TARGET generate )
        add_dependencies(${targetname} generate)
    ENDIF ()
ENDMACRO (GTI_MAC_ADD_MODULE)

MACRO (GTI_MAC_ADD_MODULE_UNPATCHED targetname sources language)
    #Add target and its dependency on the patcher
    ADD_LIBRARY(${targetname} MODULE ${sources})
    
    #For Apple set that undefined symbols should be looked up dynamically
    #(On linux this is already the default)
    IF (APPLE)
        SET_TARGET_PROPERTIES(${targetname} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
    ENDIF (APPLE)
    IF (GTI_THREAD_SAFETY)
        TARGET_LINK_LIBRARIES(${targetname} GtiTLS)
    ENDIF (GTI_THREAD_SAFETY)
    
    ##Not necessary: the script with the patcher is run anyways
    #Install it with reasonable file permissions
    #INSTALL(TARGETS ${targetname}
    #    PERMISSIONS 
    #        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
    #        GROUP_EXECUTE GROUP_READ 
    #        WORLD_EXECUTE
    #    RUNTIME DESTINATION bin
    #    LIBRARY DESTINATION modules
    #    ARCHIVE DESTINATION modules
    #    )
        
    INSTALL(TARGETS ${targetname}
       PERMISSIONS 
           OWNER_READ OWNER_WRITE OWNER_EXECUTE 
           GROUP_EXECUTE GROUP_READ 
           WORLD_EXECUTE WORLD_READ
       DESTINATION modules
       )
    IF ( TARGET generate )
        add_dependencies(${targetname} generate)
    ENDIF ()
ENDMACRO (GTI_MAC_ADD_MODULE_UNPATCHED)


####################################################
## Macro featureTestMpi
##
## Creates a feature test with an MPI program. 
####################################################
MACRO (
    featureTestMpi 
        source # The source file name of the MPI program to test, must be in the folder cmakemodules/FeatureTests
        language # one of: C, CXX, Fortran
        successVar # Name of a variable to set to true iff the test was successful, will be set as a CACHE variable
        )
    IF (NOT ${successVar})
        #Organize the temporary src directory, copy source there
        SET (binDir "${CMAKE_CURRENT_BINARY_DIR}/${source}/BUILD")
        SET (srcDir "${CMAKE_CURRENT_BINARY_DIR}/${source}")
        CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/cmakemodules/FeatureTests/${source}" "${srcDir}/${source}" COPYONLY)
        
        #Determine the compilers to use
        FOREACH (lan C CXX Fortran)
            SET (${lan}_compiler_to_use ${CMAKE_${lan}_COMPILER})
            IF (MPI_${lan}_COMPILER)
                SET (${lan}_compiler_to_use ${MPI_${lan}_COMPILER})
            ENDIF ()
        ENDFOREACH ()

        #If MPI include directories are a list, we need to concatenate it and add quotes to handle potential space characters correctly
        SET (TEMP_INCS "")
        FOREACH (INC_PATH ${MPI_${language}_INCLUDE_PATH})
            SET (TEMP_INCS "${TEMP_INCS} \"${INC_PATH}\"")
        ENDFOREACH ()

        #Create CMakeLists.txt
        FILE(WRITE "${srcDir}/CMakeLists.txt" 
            "PROJECT (test ${language})\n"
            "cmake_minimum_required(VERSION 2.6)\n"
            "set(CMAKE_C_COMPILER ${C_compiler_to_use})\n"
            "set(CMAKE_CXX_COMPILER ${CXX_compiler_to_use})\n"
            "set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})\n"
            "STRING(REPLACE \";\" \" \" CMAKE_CXX_FLAGS
            \"${CMAKE_CXX_FLAGS}\")\n"
            "set(CMAKE_Fortran_COMPILER ${Fortran_compiler_to_use})\n"
            "if (NOT \"${MPI_${language}_INCLUDE_PATH}\" STREQUAL \"\")\n"
            "    include_directories(${TEMP_INCS})\n"
            "endif (NOT \"${MPI_${language}_INCLUDE_PATH}\" STREQUAL \"\")\n"
            "add_executable(test-gti \"${source}\")\n"
            "target_link_libraries(test-gti ${MPI_${language}_LIBRARIES})\n"
            )

        #Try compile and preserve the result in a cached variable
        try_compile(${successVar} "${binDir}" "${srcDir}"
                        test
                        OUTPUT_VARIABLE output)
                        
        SET (${successVar} ${${successVar}} CACHE INTERNAL "Result of feature testing ${source}")

        IF (GTI_VERBOSE)
            MESSAGE ("${output}")
        ENDIF (GTI_VERBOSE)
        
        #Add status
        SET (successStatus "success")
        IF (NOT ${successVar})
            SET (successStatus "failed")
        ENDIF (NOT ${successVar})
        
        MESSAGE (STATUS "Checking for ${source} ... ${successStatus}")
    ENDIF (NOT ${successVar})
ENDMACRO (featureTestMpi)

MACRO(COLORMESSAGE message failure)
string(ASCII 27 Esc)
set(ColorReset "${Esc}[m")
set(Red         "${Esc}[1;31m")
set(Green       "${Esc}[1;32m")
set(failure ${failure})
if (failure)
    message(STATUS "${Red}${message}${ColorReset}")
else ()
    message(STATUS "${Green}${message}${ColorReset}")
endif ()
ENDMACRO(COLORMESSAGE)
