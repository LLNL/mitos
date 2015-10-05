#!/bin/csh

git clone https://github.com/scalability-llnl/spack.git

cd spack

git checkout develop

./bin/spack install dyninst
./bin/spack install hwloc

setenv Dyninst_DIR $(./bin/spack location -i dyninst)/lib/cmake/Dyninst
setenv hwloc_INC $(./bin/spack location -i hwloc)/include
setenv hwloc_LIB $(./bin/spack location -i hwloc)/lib
 
setenv C_INCLUDE_PATH $hwloc_INC:$C_INCLUDE_PATH
setenv CPLUS_INCLUDE_PATH $hwloc_INC:$CPLUS_INCLUDE_PATH
setenv LD_LIBRARY_PATH $hwloc_LIB:$LD_LIBRARY_PATH

cd ..
