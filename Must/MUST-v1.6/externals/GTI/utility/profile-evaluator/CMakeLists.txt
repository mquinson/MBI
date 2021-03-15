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
#       CMake file for profile-evaluator directory.
#
# @author Tobias Hilbrich
# @date 18.07.2012

#
# The Profile evaluator can show heat maps for the profiled values,
# it uses the CImg.h header to do so. As this header comes with a 
# CECILL license that appears to be a bit incompatible with our BSD
# license, we do not include this header.
# As a result, the default is you get a version of the profile evaluator
# that only outputs gnuplot scripts and data files, if you happen to
# put a CImg.h version into this directory yourself, you will get 
# enhanced functionality.
#
SET (EXTRA_DEFINES)
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CImg.h")
    SET (EXTRA_DEFINES "-DHAVE_CIMG")
ENDIF ()

ADD_DEFINITIONS("-Dcimg_display=0" ${EXTRA_DEFINES}) ## Disables CImgs capcability to make GUI windows, which we don't need (Otherwise we need to puzzle out all sorts of X11 libs)
ADD_EXECUTABLE (gti_eval_profile GtiEvalProfile.cpp)

INSTALL(TARGETS gti_eval_profile
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
