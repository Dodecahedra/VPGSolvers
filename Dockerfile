FROM ubuntu:latest

COPY . /VPGSolvers
RUN apt-get update && DEBIAN_FRONTEND="noninteractive" apt-get install -y cmake g++ gcc wget build-essential
RUN wget https://sourceforge.net/projects/buddy/files/buddy/BuDDy%202.4/buddy-2.4.tar.gz
RUN tar -xf buddy-2.4.tar.gz
RUN buddy-2.4/configure && cd buddy-2.4
RUN make && make install && cd ..
RUN cd /VPGSolvers && rm -r build
# Remove GTest library and only make the VPGSolver binary
RUN cmake -DIN_DOCKER=ON -S /VPGSolvers -B build
RUN cmake --build build