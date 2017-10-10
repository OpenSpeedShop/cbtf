#!/bin/bash 

rm -rf  build
mkdir build
pushd build

export CC=gcc
export CXX=g++

export KRELL_ROOT=~/OSS/krellroot_v2.3.1
export CBTF_ROOT=~/OSS/cbtf_v2.3.1
export MRNET_ROOT=~/OSS/krellroot_v2.3.1
export BOOST_ROOT=~/OSS/krellroot_v2.3.1

cmake .. \
        -DCMAKE_BUILD_TYPE=None \
        -DCMAKE_CXX_FLAGS="-g -O2" \
        -DCMAKE_C_FLAGS="-g -O2" \
        -DCMAKE_INSTALL_PREFIX=${CBTF_ROOT} \
        -DCMAKE_PREFIX_PATH=${KRELL_ROOT} \
        -DMRNET_DIR=${MRNET_ROOT} \
	-DCBTF_BOOST_ROOT=_ROOT=${BOOST_ROOT}

make clean
make
make install

