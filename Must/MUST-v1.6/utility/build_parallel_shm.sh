#!/bin/bash

# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2014 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2014 Lawrence Livermore National Laboratories, United States of America
#   2013-2014 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

set -o errexit nounset

# fusionforge user
SVNUSER=${SVNUSER:-$USER}

# load build essencials
module load intel cmake/2.8.4
# pnmpi tarball
PNMPI_TAR=${PNMPI_TAR:-/fastfs/jprotze/mustkram/pnmpi-new.tar}
# workdir in ramfs
WORKDIR=${WORKDIR:-/dev/shm/$USER}
# installdir
PREFIX=${PREFIX:-$WORKDIR/usr/}
# parallel
PARCOUNT=${PARCOUNT:-32}

# include python-2.7 to path
export PATH=/home/jprotze/.root/usr/bin:$PATH
# include installdir to path
export PATH=$PREFIX/bin:$PATH

mkdir -p $PREFIX $WORKDIR

# pnmpi
if [[ ! -e $WORKDIR/pnmpi ]] ; then
  cd $WORKDIR
  tar -xf $PNMPI_TAR
fi
if [[ ! -e $WORKDIR/pnmpi/BUILD ]] ; then
  mkdir -p $WORKDIR/pnmpi/BUILD
  cd $WORKDIR/pnmpi/BUILD
  CC=icc CXX=icpc FC=ifort cmake ../ -DCMAKE_INSTALL_PREFIX=$PREFIX -DENABLE_TESTS=On -DCMAKE_BUILD_TYPE=Debug
fi
cd $WORKDIR/pnmpi/BUILD
make -j$PARCOUNT install

# gti
if [[ ! -e $WORKDIR/gti ]] ; then
  cd $WORKDIR
  svn checkout --username $SVNUSER https://fusionforge.zih.tu-dresden.de/svn/gti/trunk gti
else
  cd $WORKDIR/gti/
  svn up
fi
if [[ ! -e $WORKDIR/gti/BUILD ]] ; then
  mkdir -p $WORKDIR/gti/BUILD
  cd $WORKDIR/gti/BUILD
  CC=icc CXX=icpc FC=ifort cmake ../ -DCMAKE_INSTALL_PREFIX=$PREFIX -DPnMPI_HOME=$PREFIX -DENABLE_TESTS=On -DCMAKE_BUILD_TYPE=Debug
fi
cd $WORKDIR/gti/BUILD
make -j$PARCOUNT install

# must
if [[ ! -e $WORKDIR/must ]] ; then
  cd $WORKDIR
  svn checkout --username $SVNUSER https://fusionforge.zih.tu-dresden.de/svn/must/trunk must
else
  cd $WORKDIR/must/
  svn up
fi
if [[ ! -e $WORKDIR/must/BUILD ]] ; then
  mkdir -p $WORKDIR/must/BUILD
  cd $WORKDIR/must/BUILD
  CC=icc CXX=icpc FC=ifort cmake ../ -DCMAKE_INSTALL_PREFIX=$PREFIX -DGTI_HOME=$PREFIX -DENABLE_TESTS=On -DCMAKE_BUILD_TYPE=Debug
fi
cd $WORKDIR/must/BUILD
make -j$PARCOUNT install

echo
echo =================================================
echo
echo "build of pnmpi + gti + must completed, execute "
echo
echo "export PATH=${PREFIX}bin:\$PATH"
echo
echo "to make the tools available"
echo
echo "the BUILD is found in ${PREFIX}<tool>/BUILD"
