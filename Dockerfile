# Rebuild the docker image:
#      docker build -f Dockerfile -t mpi-bugs-initiative:latest .
#      docker build -f Dockerfile -t registry.gitlab.inria.fr/quinson/mbi2:latest .
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
# Personal notes: Push to the Inria gitlab
#      docker image tag mquinson/mbi registry.gitlab.inria.fr/quinson/mbi2
#      docker login registry.gitlab.inria.fr
#      docker push registry.gitlab.inria.fr/quinson/mbi2

#Parcoach needs llvm-15, so we cannot use debian:11 for the time being
FROM debian:testing
USER root
#RUN apt-get update
RUN apt-get update --fix-missing && \
    apt-get install -y p7zip p7zip-full apt-transport-https ca-certificates python-is-python3 python3-pip valgrind && \
    update-ca-certificates && \
    apt-get autoremove -yq && apt-get clean -yq && \
    rm -rf /MBI/builds /MBI-builds

# MPI-Checker dependencies
RUN pip3 install scan-build

# Plots dependencies
RUN pip3 install numpy matplotlib

# our code
COPY . /MBI

# Building ISP
# build error with openmpi:
# /usr/lib/x86_64-linux-gnu/openmpi/include/mpi.h:338:73: error: static assertion failed: MPI_Type_extent was removed in MPI-3.0.  Use MPI_Type_get_extent instead.
RUN apt-get -y install default-jdk-headless wget   gcc mpich libmpich-dev lsof && \
    /MBI/MBI.py -c build -x isp && \
    apt-get -y remove default-jdk-headless wget && \
    apt-get autoremove -yq && apt-get clean -yq && rm -rf /tmp/*

# Building CIVL
RUN apt-get -y install wget default-jre-headless cvc4 z3 && \
    /MBI/MBI.py -c build -x civl && \
    apt-get -y remove wget && \
    apt-get autoremove -yq && apt-get clean -yq && rm -rf /tmp/*

# Building Hermes
RUN apt-get -y install autoconf automake autotools-dev libz3-dev git  libz3-4 libtinfo-dev libtool mpich libmpich-dev && \
    /MBI/MBI.py -c build -x hermes && \
    apt-get -y remove autoconf automake autotools-dev libz3-dev git && \
    apt-get autoremove -yq && apt-get clean -yq && rm -rf /tmp/*

# Building MUST
RUN apt-get -y install cmake gfortran wget git   openmpi-bin libopenmpi-dev libdw-dev libxml2-dev && \
    /MBI/MBI.py -c build -x must && \
    apt-get -y remove cmake gfortran wget git && \
    apt-get autoremove -yq && apt-get clean -yq && rm -rf /tmp/*

# Building SimGrid
RUN /MBI/MBI.py -c build -x simgrid && rm -rf /tmp/*

# RUN MBI/MBI.py -c build -x simgrid-3.27,simgrid-3.28,simgrid-3.29,simgrid-3.30,simgrid-3.31,simgrid-3.32 && rm -rf /tmp/*

# Building Parcoach
RUN apt-get -y install cmake git   clang-15 clang++-15 clang-format openmpi-bin libopenmpi-dev && \
    ln -s /usr/bin/clang-15 /usr/bin/clang && \
    ln -s /usr/bin/clang++-15 /usr/bin/clang++ && \
    /MBI/MBI.py -c build -x parcoach && \
    apt-get -y remove cmake git && \
    apt-get autoremove -yq && apt-get clean -yq && rm -rf /tmp/*

