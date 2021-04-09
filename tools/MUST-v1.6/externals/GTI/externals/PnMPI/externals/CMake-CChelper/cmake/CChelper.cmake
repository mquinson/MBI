# This file is part of CMake-CChelper.
#
# Copyright (C)
#   2015 Alexander Haase <alexander.haase@rwth-aachen.de>
#
# See the LICENSE file in the base directory for details.
# All rights reserved.
#

## \brief Helper function to check if \p LANG compiler supports \p FLAG.
#
#
# \param FLAG The flag to be checked.
# \param LANG The language to be checked.
# \param [out] VARIABLE Where to store the result.
#
function (cchelper_check_compiler_flag FLAG LANG VARIABLE)
	if (${LANG} STREQUAL "C")
		include(CheckCCompilerFlag)
		check_c_compiler_flag("${FLAG}" ${VARIABLE})

	elseif (${LANG} STREQUAL "CXX")
		include(CheckCXXCompilerFlag)
		check_cxx_compiler_flag("${FLAG}" ${VARIABLE})

	elseif (${LANG} STREQUAL "Fortran")
		# CheckFortranCompilerFlag was introduced in CMake 3.x. To be compatible
		# with older Cmake versions, we will check if this module is present
		# before we use it. Otherwise we will define Fortran coverage support as
		# not available.
		include(CheckFortranCompilerFlag OPTIONAL RESULT_VARIABLE INCLUDED)
		if (INCLUDED)
			check_fortran_compiler_flag("${FLAG}" ${VARIABLE})
		elseif (NOT CMAKE_REQUIRED_QUIET)
			message(STATUS "Performing Test ${VARIABLE}")
			message(STATUS "Performing Test ${VARIABLE}"
			               " - Failed (Check not supported)")
		endif ()
	endif()
endfunction ()


## \brief Check an array of possible flags for all available compilers.
#
# \details This macro checks possible compiler flags for the available
#  compilers. If a suitable flag has been found, it will be added to the
#  language specific compile flags.
#
#
# \param NAME What flags will be searched, e.g. Wall.
# \param FLAGS List of possible flags.
#
macro (cchelper_check_for_flags FLAG_CANDIDATES NAME)
	# Iterate over the list of enabled languages. For each enabled language the
	# list of compiler flags will be checked.
	get_property(ENABLED_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
	foreach (LANG ${ENABLED_LANGUAGES})
		# The flags are not dependend on language, but the used compiler. So
		# instead of searching flags foreach language, search flags foreach
		# compiler used.
		set(COMPILER ${CMAKE_${LANG}_COMPILER_ID})
		if (NOT DEFINED ${NAME}_${COMPILER}_FLAGS AND COMPILER)
			foreach (FLAG ${FLAG_CANDIDATES})
				if (NOT CMAKE_REQUIRED_QUIET)
					message(STATUS "Try ${COMPILER} ${NAME} flag = [${FLAG}]")
				endif()

				set(CMAKE_REQUIRED_FLAGS "${FLAG}")
				unset(${NAME}_FLAG_DETECTED CACHE)
				cchelper_check_compiler_flag("${FLAG}" ${LANG}
				                             ${NAME}_FLAG_DETECTED)

				if (${NAME}_FLAG_DETECTED)
					set(${NAME}_${COMPILER}_FLAGS "${FLAG}" CACHE STRING
						"${NAME} flags for ${COMPILER} compiler.")
					mark_as_advanced(${NAME}_${COMPILER}_FLAGS)
					break()
				endif ()
			endforeach ()

			if (NOT ${NAME}_FLAG_DETECTED)
				message("${NAME} is not available for ${COMPILER} compiler.")
			endif ()
		endif ()

		# Add the compiler flags to the list of compiler flags to be used by
		# following targets.
		if (DEFINED ${NAME}_${COMPILER}_FLAGS)
			set(CMAKE_${LANG}_FLAGS
			    "${CMAKE_${LANG}_FLAGS} ${${NAME}_${COMPILER}_FLAGS}")
		endif ()
	endforeach ()
endmacro ()




# Enable compiler warnings. A default option can be set via the
# CCHELPER_ENABLE_WARNINGS variable. Set it before including this file.
if (NOT DEFINED CCHELPER_ENABLE_WARNINGS)
	set(CCHELPER_ENABLE_WARNINGS true)
endif ()

option(ENABLE_WARNINGS
	"Selects whether compiler warnings are enabled."
	${CCHELPER_ENABLE_WARNINGS}
)

if (ENABLE_WARNINGS)
	cchelper_check_for_flags("-Wall;/W3" warning)
endif (ENABLE_WARNINGS)


# Enable pedantic warnings. A default option can be set via the
# CCHELPER_ENABLE_PEDANTIC variable. Set it before including this file.
if (NOT DEFINED CCHELPER_ENABLE_PEDANTIC)
	set(CCHELPER_ENABLE_PEDANTIC true)
endif ()

option(ENABLE_PEDANTIC
	"Selects whether pedantic warnings are enabled."
	${CCHELPER_ENABLE_PEDANTIC}
)

if (ENABLE_PEDANTIC)
	cchelper_check_for_flags("-pedantic" pedantic)
endif (ENABLE_PEDANTIC)


# Enable warning to error conversion. A default option can be set via the
# CCHELPER_ENABLE_WARNING_TO_ERROR variable. Set it before including this file.
if (NOT DEFINED CCHELPER_ENABLE_WARNINGS_TO_ERRORS)
	set(CCHELPER_ENABLE_WARNINGS_TO_ERRORS true)
endif ()

option(ENABLE_WARNINGS_TO_ERRORS
	"Selects whether warnings should be converted to errors."
	${CCHELPER_ENABLE_WARNINGS_TO_ERRORS}
)

if (ENABLE_WARNINGS_TO_ERRORS)
	cchelper_check_for_flags("-Werror" warning_to_error)
endif (ENABLE_WARNINGS_TO_ERRORS)
