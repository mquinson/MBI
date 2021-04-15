# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2014 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2014 Lawrence Livermore National Laboratories, United States of America
#   2013-2014 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# Configure mustrun.
#
# As the mustrun script needs paths of GTI binaries and one can't simply get
# these via the target properties (see CMake CMP0026), generator expressions
# will be used to get the location of the binaries. Therefore, mustrun will be
# configured first to replace all variables inside the script. Then, the
# generator expressions will be evaluated.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mustrun2.sh"
               "${CMAKE_CURRENT_BINARY_DIR}/mustrun.configured" @ONLY)
FILE(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mustrun"
              INPUT  "${CMAKE_CURRENT_BINARY_DIR}/mustrun.configured")

add_library(PrintMemUsage SHARED
  printMemoryusage.c
)

add_library(XPrintMemUsage SHARED
  printMemoryusage.c
)

make_mpi_target(PrintMemUsage)
make_mpi_target(XPrintMemUsage)

if(APPLE)
    get_target_property(TEMP_FLAG XPrintMemUsage LINK_FLAGS)
    string(TOUPPER "${TEMP_FLAG}" TEMP_CAPITAL_FLAG)
    if(${TEMP_CAPITAL_FLAG} STREQUAL "TEMP_FLAG-NOTFOUND")
        set(TEMP_FLAG "")
    endif()
    set(TEMP_FLAG "${TEMP_FLAG} -undefined dynamic_lookup")
    set_target_properties(XPrintMemUsage PROPERTIES LINK_FLAGS ${TEMP_FLAG})
endif()

target_compile_definitions( XPrintMemUsage PRIVATE -DPMPIPREFIX=XMPI)

INSTALL(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mustrun"
        "timeout"
        "cleanup_shm"
    DESTINATION bin 
    PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE  
        GROUP_READ GROUP_EXECUTE  
        WORLD_READ WORLD_EXECUTE
    )

INSTALL(TARGETS
	PrintMemUsage XPrintMemUsage
    DESTINATION modules
    PERMISSIONS 
        OWNER_READ OWNER_WRITE 
        GROUP_READ 
        WORLD_READ 
    )

INSTALL(
    FILES "error_generator.c" 
    DESTINATION src
    )

SET (MUST_TIMEOUT_SCRIPT "${CMAKE_INSTALL_PREFIX}/bin/timeout" CACHE INTERNAL "")

