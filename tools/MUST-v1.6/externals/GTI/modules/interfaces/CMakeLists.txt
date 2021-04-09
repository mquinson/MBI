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
#       MUST file for the modules/interfaces directory.
#
# @author Tobias Hilbrich
# @date 16.03.2009

#Set the version of the interfaces
SET (I_COMM_PROTOCOL_VERSION 1 CACHE INTERNAL "Version of the comm protocol interface.")
SET (I_COMM_STRATEGY_VERSION 1 CACHE INTERNAL "Version of the comm strategy interface.")
SET (I_LOGGER_VERSION 1 CACHE INTERNAL "Version of the logger interface.")
SET (I_RECORD_GENERATION_VERSION 1 CACHE INTERNAL "Version of the logger interface.")

INSTALL(FILES 
            "I_CommProtocol.h" 
            "I_CommStrategyDown.h" 
            "I_CommStrategyUp.h"
            "I_CommStrategyIntra.h" 
            "I_Module.h"
            "I_Place.h"
            "I_PlaceReceival.h"
            "I_RecordGenerator.h"
            "I_ChannelId.h"
            "I_Reduction.h"
            "I_Continuous.h"
            "I_PanicListener.h"
    DESTINATION 
        "${CMAKE_INSTALL_PREFIX}/include"
    PERMISSIONS 
        OWNER_READ OWNER_WRITE 
        GROUP_READ 
        WORLD_READ
    )
