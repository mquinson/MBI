# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details


#
# The PnMPI Callpath module
#

ADD_WRAPPED_FILE(wrappers.cpp wrappers.w)

INCLUDE_DIRECTORIES (${PnMPI_INCLUDE_PATH})

#compile callpath module
GTI_MAC_ADD_MODULE (callpathModule "callpath_module.cpp;wrappers.cpp" "CXX")
TARGET_LINK_LIBRARIES(callpathModule callpath)

MUST_INSTALL(FILES callpath_module.h DESTINATION include)

