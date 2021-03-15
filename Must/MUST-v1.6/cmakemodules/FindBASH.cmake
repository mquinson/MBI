# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# - Find BASH
# This module looks for the bash shell.
# It will set the following variables:
#  Bash = the bash shell binary
#
# @author Tobias Hilbrich
#

INCLUDE(FindPackageHandleStandardArgs)

FIND_PROGRAM (BASH NAMES bash PATH /usr/bin /bin /local/bin /usr/local/bin)
MARK_AS_ADVANCED(FORCE BASH)

find_package_handle_standard_args(BASH  DEFAULT_MSG BASH)
