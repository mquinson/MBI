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
		__atomic_fetch_add(&i, 0, __ATOMIC_SEQ_CST);
	}
	" HAVE_ATOMIC_FETCH_ADD)

find_package_handle_standard_args(BuiltinAtomic FOUND_VAR BUILTINATOMIC_FOUND
                                  REQUIRED_VARS HAVE_ATOMIC_FETCH_ADD)
