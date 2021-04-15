# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

##
# @file FindOTF.cmake
#       Detects the OTF installation (optional feature).
#
# @author Tobias Hilbrich
# @date 14.04.2009

#Not yet found ...
SET(OTF_FOUND FALSE)
              
## TODO:
## This is currently only a placeholderans sets the 
## pathes to initial values that can be adapted manualy.
SET (OTF_LIB_DIR    "/home/hilbrich/local/otf/lib" 
     CACHE PATH     "The path for the OTF library.")
SET (OTF_LIBS       "-lotf"                  
     CACHE STRING   "The OTF Library.")
SET (OTF_INC_DIR    "/home/hilbrich/local/otf/include"
     CACHE PATH     "The directory that contains the OTF headers.")
SET (OTF_FOUND TRUE)

