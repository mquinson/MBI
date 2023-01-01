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
    apt-get -y -qq install autoconf automake autotools-dev build-essential clang clang-tools cmake cvc4 \
                           gcc-10 git mpich libboost-dev libcairo2 libdw-dev libboost-stacktrace-dev\
                           libelf-dev libevent-dev libncurses5 libunwind-dev libtinfo-dev \
                           libtool libxml2-dev libz3-dev clang-15 libllvm15 llvm-15 llvm-15-dev lsof default-jdk-headless psmisc \
                           python-is-python3 python3-pip quilt valgrind wget z3 zlib1g-dev clang-tidy &&\
    apt-get install p7zip p7zip-full wget apt-transport-https ca-certificates -y && update-ca-certificates && \
    apt-get autoremove -yq && \
    apt-get clean -yq

# MPI-Checker dependencies
RUN pip3 install scan-build

# Plots dependencies
RUN pip3 install numpy matplotlib

# RUN pip3 install drawSvg
COPY . /MBI

# Rebuild all tools (parcoach does not build right now because 'lit' cannot be found)
RUN cd /MBI ; ./MBI.py -c generate ; rm -rf builds ; ./MBI.py -c build -x civl,hermes,isp,must,simgrid; rm -rf /tmp/*
# RUN cd /MBI ;  ./MBI.py -c build -x simgrid-3.27,simgrid-3.28,simgrid-3.29,simgrid-3.30,simgrid-3.31,simgrid-3.32; rm -rf /tmp/*
