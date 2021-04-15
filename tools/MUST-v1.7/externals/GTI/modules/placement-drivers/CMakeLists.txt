# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

##
# @file CMakeLists.cmake
#       MUST file for the modules/placement-drivers.
#
# @author Tobias Hilbrich
# @date 16.03.2009

SET (BASE_SOURCES)


GTI_MAC_ADD_MODULE ( threadedMpiPlace "ThreadedMpiPlace.cpp;SuspensionBufferTree.cpp;../utility/GtiHelper.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE ( threadedAppPlace "ThreadedAppPlace.cpp;SuspensionBufferTree.cpp;../utility/GtiHelper.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE ( threadedAppStartup "ThreadedAppStartup.cpp;${BASE_SOURCES}" "CXX")

GTI_MAC_ADD_MODULE ( appPlace "AppPlace.cpp;${BASE_SOURCES}" "CXX")
target_link_libraries(  appPlace GtiTLS )
