# This file is part of CMake-CChelper.
#
# Copyright (C)
#   2015 Alexander Haase <alexander.haase@rwth-aachen.de>
#
# See the LICENSE file in the base directory for details.
# All rights reserved.
#

include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckSymbolExists)
include(FindPackageHandleStandardArgs)


set(C99_FLAG_CANDIDATES
	"-std=gnu11"
	"-std=c11"
)


set(CMAKE_REQUIRED_QUIET ${C11_FIND_QUIETLY})

if (NOT CMAKE_C_COMPILER_LOADED)
	message(SEND_ERROR "C11 requires C language to be enabled")
endif ()


# Search for a working compiler flag.
if (NOT C11_FLAGS)
	foreach (FLAG ${C99_FLAG_CANDIDATES})
		if(NOT CMAKE_REQUIRED_QUIET)
			message(STATUS "Try C11 flag = [${FLAG}]")
		endif()

		unset(C11_FLAG_DETECTED CACHE)
		set(CMAKE_REQUIRED_FLAGS "${FLAG}")
		check_c_compiler_flag("${FLAG}" C11_FLAG_DETECTED)
		unset(CMAKE_REQUIRED_FLAGS)
		if (C11_FLAG_DETECTED)
			set(C11_FLAGS "${FLAG}" CACHE STRING "C compiler flags for C11")
			break()
		endif ()
	endforeach ()
endif ()


# Search for optional components.
if (C11_FLAGS)
	set(CMAKE_REQUIRED_FLAGS "${C11_FLAGS}")

	foreach (component IN LISTS C11_FIND_COMPONENTS)
		string(TOUPPER ${component} component)

		if (component STREQUAL "ATOMICS")
			# atomics need a special check, if stdatomic.h is available due a
			# bug in GCC versions before 4.9.
			check_c_source_compiles("
				#ifndef __STDC_NO_ATOMICS__
				#include <stdatomic.h>
				int main () {}
				#endif
				"
			C11_ATOMICS_FOUND)

		elseif (component STREQUAL "THREADS")
			# threads need a special check, if threads.h is available due a
			# bug in GCC versions before 4.7.
			check_c_source_compiles("
				#ifndef __STDC_NO_THREDAS__
				#include <threads.h>
				int main () {}
				#endif
				"
			C11_THREADS_FOUND)

		elseif (component STREQUAL "THREAD_LOCAL")
			# _Thread_local needs a special check, as some compilers (e.g. gcc)
			# do not support C11 threads but thread local storage.
			check_c_source_compiles("
				_Thread_local int i;
				int main () {}
				"
			C11_THREAD_LOCAL_FOUND)

		elseif (component STREQUAL "COMPLEX" OR component STREQUAL "VLA")
			check_c_source_compiles("
				#ifndef __STDC_NO_${component}__
				int main () {}
				#endif
				"
				C11_${component}_FOUND)

		elseif (component STREQUAL "ANALYZABLE" OR component STREQUAL "IEC_559"
		        OR component STREQUAL "IEC_559_COMPLEX")
			check_symbol_exists("__STDC_${component}__" ""
			                    C11_${component}_FOUND)

		else (component STREQUAL "BOUNDS")
			check_symbol_exists("__STDC_LIB_EXT1__" "" C11_BOUNDS_FOUND)
		endif ()
	endforeach ()

	unset(CMAKE_REQUIRED_FLAGS)
endif ()


find_package_handle_standard_args(C11 REQUIRED_VARS C11_FLAGS HANDLE_COMPONENTS)
mark_as_advanced(C11_FLAGS)
