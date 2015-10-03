#!/bin/bash

git clone https://github.com/scalability-llnl/spack.git

cd spack

git checkout develop

./bin/spack install dyninst
./bin/spack install hwloc

export Dyninst_DIR=$(./bin/spack location -i dyninst)/lib/cmake/Dyninst
export hwloc_INC=$(./bin/spack location -i hwloc)/include
export hwloc_LIB=$(./bin/spack location -i hwloc)/lib

export C_INCLUDE_PATH=$hwloc_INC
export CPLUS_INCLUDE_PATH=$hwloc_INC
export LD_LIBRARY_PATH=$hwloc_LIB

cd ..
