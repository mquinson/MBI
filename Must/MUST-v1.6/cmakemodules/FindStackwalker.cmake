# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details
#
# @file FindStackwalker.cmake
#       Detects the dyninst/stackwalkerAPI installation.
#
# @author Joachim Protze
# @date 13.12.2012

#
# -DUSE_CALLPATH=ON
# -DSTACKWALKER_INSTALL_PREFIX=<stackwalker_prefix>  or  -DCALLPATH_STACKWALKER_HOME=<stackwalker_prefix>
#
# tries to autodetect the installation sceme of dyninst-7 an dyninst-8
# could fail when having builts for multiple architectures
# in this case set:
#       -DCALLPATH_STACKWALKER_LIB_PATH=<stackwalker_prefix>/<arch>/lib
#


# use an explicitly given stackwalk path first
FIND_PATH(CALLPATH_STACKWALKER_INCLUDE_PATH walker.h 
            PATHS ${STACKWALKER_INSTALL_PREFIX} ${CALLPATH_STACKWALKER_HOME} /usr /usr/local
            PATH_SUFFIXES   include include/dyninst NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(CALLPATH_STACKWALKER_INCLUDE_PATH walker.h)

# use an explicitly given stackwalk path first
FIND_PATH(CALLPATH_STACKWALKER_LIB_PATH libstackwalk.so 
            PATHS ${STACKWALKER_INSTALL_PREFIX} ${CALLPATH_STACKWALKER_HOME} /usr /usr/local
            PATH_SUFFIXES   lib 
                            amd64-unknown-freebsd7.2/lib
                            i386-unknown-freebsd7.2/lib
                            i386-unknown-linux2.4/lib
                            i386-unknown-vxworks6.x/lib
                            i386-unknown-winXP/lib
                            ppc32_bgl_compute/lib
                            ppc32_bgl_ion/lib
                            ppc32_bgp/lib
                            ppc32_bgp_compute/lib
                            ppc32_bgp_ion/lib
                            ppc32_linux/lib
                            ppc32-unknown-vxworks6.x/lib
                            ppc64_linux/lib
                            ppc64-unknown-linux2.4/lib
                            rs6000-ibm-aix5.1/lib
                            rs6000-ibm-aix64-5.2/lib
                            sparc-sun-solaris2.8/lib
                            sparc-sun-solaris2.9/lib
                            x86_64_catamount/lib
                            x86_64_cnl/lib
                            x86_64-unknown-linux2.4/lib 
                            lib/dyninst
                            lib64/dyninst
		NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(CALLPATH_STACKWALKER_LIB_PATH libstackwalk.so)

# collect all libraries (for the ubuntu users :)
set(CALLPATH_STACKWALKER_LIBRARIES "")

set(CALLPATH_STACKWALKER_LIBRARY_DEPENDENCIES
        libdynElf.so 
        libstackwalk.so 
        libsymtabAPI.so 
        libcommon.so 
        libparseAPI.so 
        libsymLite.so 
        libinstructionAPI.so 
        libdynDwarf.so
        libpcontrol.so )

FOREACH ( LIB ${CALLPATH_STACKWALKER_LIBRARY_DEPENDENCIES} )
    FIND_LIBRARY(${LIB}_LOC ${LIB} 
            PATHS ${CALLPATH_STACKWALKER_LIB_PATH} NO_DEFAULT_PATH)
    IF (${LIB}_LOC)
        set(CALLPATH_STACKWALKER_LIBRARIES ${CALLPATH_STACKWALKER_LIBRARIES} "${${LIB}_LOC}" )
    ENDIF (${LIB}_LOC)
ENDFOREACH ( LIB )

set(CALLPATH_STACKWALKER_EXTRA_LIBRARIES  "" CACHE STRING "Additional libraries needed for compiling with the Dyninst stackwalker, this is usually required if you have a static library for libdwarf.")


find_package_handle_standard_args(STACKWALKER  DEFAULT_MSG  CALLPATH_STACKWALKER_INCLUDE_PATH CALLPATH_STACKWALKER_LIBRARIES )

