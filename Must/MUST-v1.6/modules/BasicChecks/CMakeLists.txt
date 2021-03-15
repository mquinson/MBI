# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details
#
# @file CMakeLists.cmake
#       CMake file for modules of directory modules/BasicChecks.
#
# @author Mathias Korepkat, Tobias Hilbrich
# @date 02.03.2011
#

#Install headers
MUST_INSTALL(
    FILES 
        I_IntegerChecks.h 
        I_BasicIntegrities.h
        I_OperationChecks.h
    DESTINATION include)

#Build and install modules 
GTI_MAC_ADD_MODULE (integerChecks "IntegerChecks.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (basicIntegrities "BasicIntegrities.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (operationChecks "OperationChecks.cpp;${BASE_SOURCES}" "CXX")
