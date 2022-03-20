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
RUN apt-get update --fix-missing && \
    apt-get -y -qq install autoconf automake autotools-dev build-essential clang clang-tools cmake cvc4 \
                           gcc-10 git mpich libboost-dev libcairo2 libdw-dev libboost-stacktrace-dev\
                           libelf-dev libevent-dev libllvm9 libncurses5 libunwind-dev libtinfo-dev \
                           libtool libxml2-dev libz3-dev llvm-9 llvm-9-dev lsof default-jdk-headless psmisc \
                           python-is-python3 python-jinja2 python2.7 python3-pip quilt valgrind wget z3 zlib1g-dev &&\
    apt-get install apt-transport-https ca-certificates -y && update-ca-certificates && \
    apt-get autoremove -yq && \
    apt-get clean -yq

# RUN pip3 install drawSvg
COPY . /MBI
