# This file is part of CMake-CChelper.
#
# Copyright (C)
#   2015 Alexander Haase <alexander.haase@rwth-aachen.de>
#
# See the LICENSE file in the base directory for details.
# All rights reserved.
#

set(C99_FLAG_CANDIDATES
	"-std=gnu99"
	"-std=c99"
)


include(CheckCCompilerFlag)
include(FindPackageHandleStandardArgs)

set(CMAKE_REQUIRED_QUIET_SAVE ${CMAKE_REQUIRED_QUIET})
set(CMAKE_REQUIRED_QUIET ${C99_FIND_QUIETLY})


if (NOT CMAKE_C_COMPILER_LOADED)
	message(SEND_ERROR "C99 requires C language to be enabled")
endif ()

# If flags for this compiler were already found, do not try to find them
# again.
if (NOT C99_FLAGS)
	foreach (FLAG ${C99_FLAG_CANDIDATES})
		if(NOT CMAKE_REQUIRED_QUIET)
			message(STATUS "Try C99 flag = [${FLAG}]")
		endif()

		unset(C99_FLAG_DETECTED CACHE)
		set(CMAKE_REQUIRED_FLAGS "${FLAG}")
		check_c_compiler_flag("${FLAG}" C99_FLAG_DETECTED)
		unset(CMAKE_REQUIRED_FLAGS)
		if (C99_FLAG_DETECTED)
			set(C99_FLAGS "${FLAG}" CACHE STRING "C compiler flags for C99")
			break()
		endif ()
	endforeach ()
endif ()


find_package_handle_standard_args(C99 REQUIRED_VARS C99_FLAGS)
mark_as_advanced(C99_FLAGS)
