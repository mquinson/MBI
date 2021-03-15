# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# - Find AWK
# This module looks for AWK.
# It will set the following variables:
#  AWK = the awk binary
#
# @author Tobias Hilbrich
#

INCLUDE(FindPackageHandleStandardArgs)

FIND_PROGRAM (AWK NAMES awk gawk PATH /usr/bin /bin /local/bin /usr/local/bin)
MARK_AS_ADVANCED(FORCE AWK)

find_package_handle_standard_args(AWK  DEFAULT_MSG AWK)
