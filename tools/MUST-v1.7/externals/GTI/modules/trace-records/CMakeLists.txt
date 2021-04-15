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
#       GTI file for the modules/trace-records directory.
#
# @author Tobias Hilbrich
# @date 16.03.2009

SET (BASE_SOURCES)

ADD_LIBRARY(gtiRecordGenImpl MODULE GtiRecordGenImplementation.cpp ${BASE_SOURCES})
INSTALL(TARGETS gtiRecordGenImpl DESTINATION modules PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
