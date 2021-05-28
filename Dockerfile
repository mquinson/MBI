# Rebuild the docker image:
#      docker build -f Dockerfile -t mpi-bugs-initiative:latest .
# Start it locally (the local repo is copied under /MBI/):
#      docker run -it mpi-bugs-initiative bash
#
# Personal notes: Push the image to the public Docker Hub
#      docker image tag mpi-bugs-initiative:latest registry.hub.docker.com/mquinson/mbi
#      docker push registry.hub.docker.com/mquinson/mbi
# Personal notes: Push the image to the gitlab.com registery   It's failing on me :( 
#      docker login registry.gitlab.com -u mquinson -p <token from journal.org>
#      docker image tag mpi-bugs-initiative:latest registry.gitlab.com/mquinson/mbi:latest
#      docker push registry.gitlab.com/mquinson/mbi
FROM ubuntu:20.04
USER root
RUN apt-get update
RUN apt-get -y -qq install software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update --fix-missing
RUN apt-get -y -qq install autoconf
RUN apt-get -y -qq install automake
RUN apt-get -y -qq install autotools-dev
RUN apt-get -y -qq install build-essential
RUN apt-get -y -qq install clang
RUN apt-get -y -qq install clang-tools
RUN apt-get -y -qq install cmake
RUN apt-get -y -qq install cvc4
RUN apt-get -y -qq install gcc-10
RUN apt-get -y -qq install git
RUN apt-get -y -qq install mpich
RUN apt-get -y -qq install libboost-dev
RUN apt-get -y -qq install libboost-context-dev
RUN apt-get -y -qq install libcairo2
RUN apt-get -y -qq install libdw-dev
RUN apt-get -y -qq install libelf-dev
RUN apt-get -y -qq install libevent-dev
RUN apt-get -y -qq install libllvm9
RUN apt-get -y -qq install libncurses5
RUN apt-get -y -qq install libunwind-dev
RUN apt-get -y -qq install libtinfo-dev
RUN apt-get -y -qq install libtool
RUN apt-get -y -qq install libxml2-dev
RUN apt-get -y -qq install libz3-dev
RUN apt-get -y -qq install llvm-9
RUN apt-get -y -qq install llvm-9-dev
RUN apt-get -y -qq install lsof
RUN apt-get -y -qq install openjdk-14-jdk
RUN apt-get -y -qq install psmisc
RUN apt-get -y -qq install python-is-python2
RUN apt-get -y -qq install python-jinja2
RUN apt-get -y -qq install python2.7
RUN apt-get -y -qq install python3-pip
RUN apt-get -y -qq install quilt
RUN apt-get -y -qq install z3
RUN apt-get -y -qq install zlib1g-dev

RUN pip3 install drawSvg
RUN apt-get autoremove -yq
RUN apt-get clean -yq
COPY . /MBI
