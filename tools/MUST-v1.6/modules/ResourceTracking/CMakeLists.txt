# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# @file CMakeLists.cmake
#       CMake file for the modules/ResourceTrace resource directory.
#
# @author Tobias Hilbrich
# @date 21.01.2011

#Add the subdirectories as includes
INCLUDE_DIRECTORIES(Comm Datatype Err Group Keyval Op Request Utility)

#Install headers
MUST_INSTALL(
    FILES 
        Utility/ResourceApi.h
        Utility/I_TrackBase.h
        Request/I_Request.h
        Request/I_RequestTrack.h
        Comm/I_Comm.h
        Comm/I_CommTrack.h
        Comm/I_CommPredefs.h
        Comm/I_CommReduction.h
        Op/I_Op.h
        Op/I_OpTrack.h
        Keyval/I_Keyval.h
        Keyval/I_KeyvalTrack.h
        Err/I_Err.h
        Err/I_ErrTrack.h
        Group/I_Group.h
        Group/I_GroupTrack.h
        Group/I_GroupTable.h
        Utility/I_Destructable.h
        Datatype/I_Datatype.h
        Datatype/I_DatatypeTrack.h
        Datatype/I_DatatypePredefs.h
        Utility/I_BaseConstants.h
        Datatype/StridedBlock.h
    DESTINATION 
        include)

#Fortran
SET (FortranExtraSrcs "")
IF (GTI_ENABLE_FORTRAN)
    CONFIGURE_FILE(Datatype/DatatypePredefsFortran.f.in "${CMAKE_CURRENT_BINARY_DIR}/DatatypePredefsFortran.f" @ONLY)
    SET (FortranExtraSrcs ";${CMAKE_CURRENT_BINARY_DIR}/DatatypePredefsFortran.f")
ENDIF (GTI_ENABLE_FORTRAN)

#Build and install modules 
GTI_MAC_ADD_MODULE (requestTrack "Request/RequestTrack.cpp;Request/Request.cpp;Utility/HandleInfoBase.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (datatypeTrack "Datatype/StridedBlock.cpp;Datatype/DatatypeTrack.cpp;Datatype/DatatypeTrackDerivedStorage.cpp;Datatype/DatatypeTrackHelpers.cpp;Datatype/Datatype.cpp;Datatype/DatatypeDotTree.cpp;Utility/HandleInfoBase.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (datatypePredefs "Datatype/DatatypePredefs.cpp${FortranExtraSrcs};${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (commTrack "Comm/CommTrack.cpp;Comm/Comm.cpp;Utility/HandleInfoBase.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (commPredefs "Comm/CommPredefs.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (commReduction "Comm/CommReduction.cpp;${GTI_SOURCE_PATH}/CompletionTree.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (groupTrack "Group/GroupTrack.cpp;Group/GroupTable.cpp;Group/Group.cpp;Utility/HandleInfoBase.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (baseConstants "Utility/BaseConstants.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (opTrack "Op/OpTrack.cpp;Op/Op.cpp;Utility/HandleInfoBase.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (keyvalTrack "Keyval/KeyvalTrack.cpp;Keyval/Keyval.cpp;Utility/HandleInfoBase.cpp;${BASE_SOURCES}" "CXX")
GTI_MAC_ADD_MODULE (errTrack "Err/ErrTrack.cpp;Err/Err.cpp;Utility/HandleInfoBase.cpp;${BASE_SOURCES}" "CXX")
