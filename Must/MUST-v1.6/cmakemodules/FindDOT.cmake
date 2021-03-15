# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# - Find Dot
# This module looks for dot of the graphviz toolset.
# It will set the following variables:
#  DOT = the dot binary
#
# @author Mathias Korepkat
#

INCLUDE(FindPackageHandleStandardArgs)

FIND_PROGRAM (DOT NAMES dot PATH /usr/bin /bin /local/bin /usr/local/bin)
MARK_AS_ADVANCED(FORCE DOT)

find_package_handle_standard_args(DOT  DEFAULT_MSG DOT)
