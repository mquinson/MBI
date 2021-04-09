# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for modules of DatatypeTrack test.
#
# @author Tobias Hilbrich
# @date 23.02.2011
#

#Install headers
INSTALL(FILES I_PrintDatatype.h DESTINATION include)

#Build and install modules 
GTI_MAC_ADD_MODULE (printDatatype "PrintDatatype.cpp;${BASE_SOURCES}" "CXX")

