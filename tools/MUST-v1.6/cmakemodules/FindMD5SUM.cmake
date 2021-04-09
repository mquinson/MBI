# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details
# 
# - Find md5sum
# This module looks for the md5sum command. 
# It will set the following variables:
#  MD5SUM = the md5sum command if found
#
# @author Tobias Hilbrich
#

INCLUDE(FindPackageHandleStandardArgs)

FIND_PROGRAM (MD5SUM NAMES md5sum PATH /usr/bin /bin /local/bin /usr/local/bin)
MARK_AS_ADVANCED(FORCE MD5SUM)

find_package_handle_standard_args(MD5SUM  DEFAULT_MSG MD5SUM)