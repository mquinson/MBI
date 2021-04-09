# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.txt
#       CMake file for modules of directory modules/Preconditioner.
#
# @author Joachim Protze, Tobias Hilbrich
# @date 06.06.2011
#

#Install headers
MUST_INSTALL(FILES 
            I_RequestCondition.h 
            RequestConditionApi.h
            I_CollectiveCondition.h 
            CollectiveConditionApi.h
            I_OnlyOnRootCondition.h 
            OnlyOnRootConditionApi.h
            I_CompletionCondition.h 
            CompletionConditionApi.h
        DESTINATION include)


#Build and install modules 
GTI_MAC_ADD_MODULE (requestCondition "RequestCondition.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (collectiveCondition "CollectiveCondition.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (onlyOnRootCondition "OnlyOnRootCondition.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (completionCondition "CompletionCondition.cpp;${BASE_SOURCES}" "CXX")