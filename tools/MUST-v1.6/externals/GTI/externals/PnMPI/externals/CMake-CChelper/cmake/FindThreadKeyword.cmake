# This file is part of CMake-CChelper.
#
# Copyright (C)
#   2015 Alexander Haase <alexander.haase@rwth-aachen.de>
#
# See the LICENSE file in the base directory for details.
# All rights reserved.
#

include(CheckCSourceCompiles)
include(FindPackageHandleStandardArgs)


check_c_source_compiles("
	int main ()
	{
		static __thread int i;
	}
" HAVE_THREAD_KEYWORD)

find_package_handle_standard_args(ThreadKeyword FOUND_VAR THREADKEYWORD_FOUND
                                  REQUIRED_VARS HAVE_THREAD_KEYWORD)
