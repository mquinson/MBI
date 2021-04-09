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
		int i;
		__sync_fetch_and_add(&i, 0);
	}
	" HAVE_SYNC_FETCH_AND_ADD)

find_package_handle_standard_args(BuiltinSync FOUND_VAR BUILTINSYNC_FOUND
                                  REQUIRED_VARS HAVE_SYNC_FETCH_AND_ADD)
