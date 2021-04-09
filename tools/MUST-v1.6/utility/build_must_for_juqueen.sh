#!/bin/bash

# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2014 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2014 Lawrence Livermore National Laboratories, United States of America
#   2013-2014 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

module load cmake

#Exit if something goes wrong
set -e

# gforge user
SVNUSER="anonsvn --password anonsvn"

CMAKE_OPTIONS=${CMAKE_OPTIONS:-"-DENABLE_TESTS=Off -DCMAKE_BUILD_TYPE=RelWithDebInfo"}
# pnmpi tarball
PNMPI_TAR=${PNMPI_TAR:-pnmpi.tar}
# workdir in ramfs
WORKDIR=${WORKDIR:-$HOME/MUST3/}
# installdir
PREFIX=${PREFIX:-$HOME/.usr-must3/}
# parallel
PARCOUNT=${PARCOUNT:-4}

# include installdir to path
export PATH=$PREFIX/bin:$PATH

mkdir -p $WORKDIR
mkdir -p $PREFIX

# pnmpi
if [[ ! -e $WORKDIR/pnmpi ]] ; then
  cd $WORKDIR
  svn checkout --username $SVNUSER https://141.30.75.37/svn/must/pnmpi/trunk pnmpi
else
  cd $WORKDIR/pnmpi/
  svn up --username $SVNUSER
fi
rm -f $WORKDIR/pnmpi/cmakemodules/FindMPI.cmake
if [[ ! -e $WORKDIR/pnmpi/BUILD ]] ; then
  mkdir -p $WORKDIR/pnmpi/BUILD
  cd $WORKDIR/pnmpi/BUILD
  CC=bgxlc CXX=bgxlc++ FC=bgxlf cmake ../\
  	-DBFD_FOUND=False -DPNMPI_HAVE_BFD=False -DBFD_LIBRARIES="/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.a"\
  	-DPATCHER_FRONTEND_COMPILER=`which gcc` -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=Release -DMPIEXEC_NUMPROC_FLAG="--np" -DCMAKE_SHARED_LINKER_FLAGS="-qnostaticlink -qnostaticlink=libgcc"\
  	-DMPI_CXX_COMPILER=`which mpixlcxx` -DMPI_Fortran_COMPILER=`which mpixlf90` -DMPI_C_COMPILER=`which mpixlc` -DCMAKE_LINKER=/bgsys/drivers/ppcfloor/gnu-linux/powerpc64-bgq-linux/bin/ld -DF77SYMBOL=symbol\
  	-DMPI_CXX_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichcxx-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_C_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_Fortran_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichf90-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"
  CC=bgxlc CXX=bgxlc++ FC=bgxlf cmake ../\
  	-DBFD_FOUND=False -DPNMPI_HAVE_BFD=False -DBFD_LIBRARIES="/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.a"\
  	-DPATCHER_FRONTEND_COMPILER=`which gcc` -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=Release -DMPIEXEC_NUMPROC_FLAG="--np" -DCMAKE_SHARED_LINKER_FLAGS="-qnostaticlink -qnostaticlink=libgcc"\
  	-DMPI_CXX_COMPILER=`which mpixlcxx` -DMPI_Fortran_COMPILER=`which mpixlf90` -DMPI_C_COMPILER=`which mpixlc` -DCMAKE_LINKER=/bgsys/drivers/ppcfloor/gnu-linux/powerpc64-bgq-linux/bin/ld -DF77SYMBOL=symbol\
  	-DMPI_CXX_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichcxx-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_C_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_Fortran_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichf90-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"
  	
fi
cd $WORKDIR/pnmpi/BUILD
make -j$PARCOUNT install


# gti
if [[ ! -e $WORKDIR/gti ]] ; then
  cd $WORKDIR
  svn checkout --username $SVNUSER https://141.30.75.37/svn/gti/trunk gti
else
  cd $WORKDIR/gti/
  svn up --username $SVNUSER
fi
# for JUQUEEN remove FindMPI, to use systems version!
rm -f $WORKDIR/gti/cmakemodules/FindMPI.cmake
if [[ ! -e $WORKDIR/gti/BUILD-frontend ]] ; then
  mkdir -p $WORKDIR/gti/BUILD-frontend
  cd $WORKDIR/gti/BUILD-frontend
  CC=`which gcc` CXX=`which g++` FC=`which gfortran` cmake ../ \
	-DCMAKE_INSTALL_PREFIX=$PREFIX -DPnMPI_HOME=$PREFIX -DCMAKE_BUILD_TYPE=Release 	-DGTI_ONLY_BUILD_FRONTEND=TRUE
fi
if [[ ! -e $WORKDIR/gti/BUILD-backend ]] ; then
  mkdir -p $WORKDIR/gti/BUILD-backend
  cd $WORKDIR/gti/BUILD-backend
  CC=bgxlc CXX=bgxlc++ FC=bgxlf cmake ../ \
	-DCMAKE_INSTALL_PREFIX=$PREFIX -DPnMPI_HOME=$PREFIX \
	-DCMAKE_BUILD_TYPE=Release\
	-DGTI_ONLY_BUILD_BACKEND=TRUE -DMPIEXEC_NUMPROC_FLAG="--np"\
	-DMPIEXEC=`which runjob` -DMPIEXEC_POSTFLAGS="--env-all"\
  	-DMPI_CXX_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichcxx-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_C_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_Fortran_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichf90-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"
  CC=bgxlc CXX=bgxlc++ FC=bgxlf cmake ../ \
	-DCMAKE_INSTALL_PREFIX=$PREFIX -DPnMPI_HOME=$PREFIX \
	-DCMAKE_BUILD_TYPE=Release\
	-DGTI_ONLY_BUILD_BACKEND=TRUE -DMPIEXEC_NUMPROC_FLAG="--np"\
	-DMPIEXEC=`which runjob` -DMPIEXEC_POSTFLAGS="--env-all"\
  	-DMPI_CXX_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichcxx-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_C_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"\
  	-DMPI_Fortran_LIBRARIES="/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpichf90-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpich-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libopa-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libmpl-xl.a;/bgsys/drivers/V1R2M1/ppc64/comm/lib/libpami-gcc.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI.a;/bgsys/drivers/V1R2M1/ppc64/spi/lib/libSPI_cnk.a;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/librt.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.so;/bgsys/drivers/toolchain/V1R2M1_base/gnu-linux/powerpc64-bgq-linux/lib/libpthread.so"
fi
cd $WORKDIR/gti/BUILD-backend
make -j$PARCOUNT install

cd $WORKDIR/gti/BUILD-frontend
make -j$PARCOUNT install


# must
if [[ ! -e $WORKDIR/must ]] ; then
  cd $WORKDIR
  svn checkout --username $SVNUSER https://141.30.75.37/svn/must/trunk must
else
  cd $WORKDIR/must/
  svn up
fi
if [[ ! -e $WORKDIR/must/BUILD ]] ; then
  mkdir -p $WORKDIR/must/BUILD
  cd $WORKDIR/must/BUILD
  CC=bgxlc CXX=bgxlc++ FC=bgxlf cmake ../ \
	-DCMAKE_INSTALL_PREFIX=$PREFIX -DGTI_HOME=$PREFIX \
	-DCMAKE_BUILD_TYPE=Release -DMPIEXEC_NUMPROC_FLAG="--np" -DMPIEXEC=`which runjob` -DMPIEXEC_POSTFLAGS="--env-all"

fi
cd $WORKDIR/must/BUILD
make -j$PARCOUNT install

# patch mustrun for JUQUEEN:
# suppose pnmpi to be always linked
sed -i $PREFIX/bin/mustrun -e 's/do_pnmpi_linked:=0/do_pnmpi_linked:=1/' 

#compiler wrapper for JUQUEEN
for i in c cxx
do
    echo "mpixl$i -Wl,--whole-archive $PREFIX/lib/libpnmpi.a -Wl,--no-whole-archive -qnostaticlink -qnostaticlink=libgcc -Wl,--export-dynamic \$*" > $PREFIX/bin/must$i
    chmod +x $PREFIX/bin/must$i
done
for i in f77 f90 f95 f2003
do
    echo "mpixl$i -Wl,--whole-archive $PREFIX/lib/libpnmpif.a -Wl,--no-whole-archive -qnostaticlink -qnostaticlink=libgcc -Wl,--export-dynamic \$*" > $PREFIX/bin/must$i
    chmod +x $PREFIX/bin/must$i
done


echo
echo =================================================
echo
echo "build of pnmpi + gti + must completed, execute "
echo
echo "export PATH=$PREFIX/bin:\$PATH"
echo
echo "to make the tools available"
