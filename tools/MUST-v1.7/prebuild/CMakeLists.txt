# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# MUST is able to prebuild GTI's infrastructure. If this feature is not enabled,
# this directory will not be processed.
#
# NOTE: As this feature is not compatible with using GTI as submodule at the
#       moment, this feature is disabled by default.
OPTION(ENABLE_PREBUILD "Enable prebuilds for GTI infrastructure" On)
IF (NOT ENABLE_PREBUILD)
    RETURN()
ENDIF ()


MACRO ( MUST_PREBUILD_CONFIGURATION targetname mustparameter )
    FILE (GLOB specs "${CMAKE_SOURCE_DIR}/specifications/*.xml")
    IF (NOT CONFIGURATION_NUM)
      SET (CONFIGURATION_NUM 1)
    ELSE ()
      MATH(EXPR CONFIGURATION_NUM "${CONFIGURATION_NUM}+1") 
    ENDIF ()
    
    ADD_CUSTOM_TARGET (
        ${targetname}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${targetname}
        COMMENT "Prebuilding common MUST configuration #${CONFIGURATION_NUM}"
    )
    ADD_CUSTOM_COMMAND ( 
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${targetname} 
        COMMAND ${BASH} ${CMAKE_BINARY_DIR}/utility/mustrun ${mustparameter}
        --must:inbuilddir --must:mode prepare --must:quiet
        --must:inst ${CMAKE_CURRENT_BINARY_DIR}/${targetname} 
        --must:temp ${CMAKE_CURRENT_BINARY_DIR}/temp_${targetname}
# Make sure that the timestamp of the ${targetname} directory gets updated
# if it already exists:
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/${targetname}
        DEPENDS ${CMAKE_SOURCE_DIR}/utility/mustrun2.sh ${specs}
    )
    INSTALL( 
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${targetname} 
        DESTINATION modules 
        COMPONENT prebuild-dirs
	EXCLUDE_FROM_ALL
        USE_SOURCE_PERMISSIONS )
    add_dependencies(prebuilds ${targetname})
#    INSTALL( 
#	CODE "execute_process(COMMAND ${BASH}
#		${CMAKE_BINARY_DIR}/utility/mustrun ${mustparameter}
#                --must:inbuilddir --must:mode prepare --must:quiet
#                --must:inst ${CMAKE_INSTALL_PREFIX}/modules/${targetname}
#                --must:temp ${CMAKE_CURRENT_BINARY_DIR}/temp_${targetname})"
#    )
ENDMACRO ( MUST_PREBUILD_CONFIGURATION )

OPTION (REDUCED_PREBUILDS "Selects whether only a reduced set of prebuilds
should be used." TRUE)

ADD_CUSTOM_TARGET (
    prebuilds
    COMMENT "Prebuilding common MUST"
)

ADD_CUSTOM_TARGET (
    install-prebuilds DEPENDS prebuilds
    COMMENT "Prebuilding common MUST"
)

ADD_CUSTOM_COMMAND (
    TARGET install-prebuilds
    COMMAND ${CMAKE_COMMAND} -DCOMPONENT=prebuild-dirs -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
    DEPENDS prebuilds
)


# for i in range(2,9):
#     print 2**(32/i)-2 , (2**(32/i)-2)**(i-1), (2**(32/i)-2)**(i-2)
# 
# 65534 65534 1
# 1022 1044484 1022
# 254 16387064 64516
# 62 14776336 238328
# 30 24300000 810000
# 14 7529536 537824
# 14 105413504 7529536


# Mode 1
MUST_PREBUILD_CONFIGURATION(mode1-layer2 "-np 65534")
MUST_PREBUILD_CONFIGURATION(mode1-layer3-hybrid "-np 1022 --must:hybrid")

# Mode 2
MUST_PREBUILD_CONFIGURATION(mode2-layer3 "-np 1044484 --must:nodesize 1023")

# Mode 3
MUST_PREBUILD_CONFIGURATION(mode3-layer2 "-np 65534 --must:nocrash")

# Mode 4
MUST_PREBUILD_CONFIGURATION(mode4-layer3 "-np 1044484 --must:nodesize 1023 --must:fanin 1022")
MUST_PREBUILD_CONFIGURATION(mode4-layer3-hybrid "-np 1022 --must:nodesize 1023 --must:fanin 1022 --must:hybrid")

# Mode 5
MUST_PREBUILD_CONFIGURATION(mode5-layer3 "-np 1044484 --must:fanin 1022")
MUST_PREBUILD_CONFIGURATION(mode5-layer3-hybrid "-np 1022 --must:fanin 1022 --must:hybrid")

IF (NOT REDUCED_PREBUILDS)
# Mode 4
    MUST_PREBUILD_CONFIGURATION(mode4-layer4 "-np 16387064 --must:nodesize 255 --must:fanin 254")
    MUST_PREBUILD_CONFIGURATION(mode4-layer5 "-np 14776336 --must:nodesize 63 --must:fanin 62")
    MUST_PREBUILD_CONFIGURATION(mode4-layer6 "-np 24300000 --must:nodesize 31 --must:fanin 30")
    MUST_PREBUILD_CONFIGURATION(mode4-layer7 "-np 7529536 --must:nodesize 15 --must:fanin 14")

# Mode 5
    MUST_PREBUILD_CONFIGURATION(mode5-layer4 "-np 16387064 --must:fanin 254")
    MUST_PREBUILD_CONFIGURATION(mode5-layer5 "-np 14776336 --must:fanin 62")
    MUST_PREBUILD_CONFIGURATION(mode5-layer6 "-np 24300000 --must:fanin 30")
    MUST_PREBUILD_CONFIGURATION(mode5-layer7 "-np 7529536 --must:fanin 14")
    MUST_PREBUILD_CONFIGURATION(mode5-layer8 "-np 105413504 --must:fanin 14")
    MUST_PREBUILD_CONFIGURATION(mode5-layer4-hybrid "-np 64516 --must:fanin 254")
    MUST_PREBUILD_CONFIGURATION(mode5-layer5-hybrid "-np 238328 --must:fanin 62")
    MUST_PREBUILD_CONFIGURATION(mode5-layer6-hybrid "-np 810000 --must:fanin 30")
#    MUST_PREBUILD_CONFIGURATION(mode5-layer7-hybrid "-np 537824 --must:fanin 14")
#    MUST_PREBUILD_CONFIGURATION(mode5-layer8-hybrid "-np 7529536 --must:fanin 14")

ENDIF ()
