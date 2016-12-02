#!/bin/bash 

rm -rf  build
mkdir build
pushd build

export CC=gcc
export CXX=g++

export KRELL_ROOT=/opt/STABLE/krellroot_v2.3.0
export CBTF_ROOT=/opt/STABLE/cbtf_v2.3.x
export MRNET_ROOT=/opt/STABLE/krellroot_v2.3.0
export BOOST_ROOT=/opt/boost-1.59.0

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

