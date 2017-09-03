#!/bin/bash 

rm -rf  build
mkdir build
pushd build

export CC=gcc
export CXX=g++

export KRELL_ROOT=/u/glschult/OSS/osscbtf_v2.3.1
export CBTF_ROOT=/u/glschult/OSS/osscbtf_v2.3.1
export MRNET_ROOT=/u/glschult/OSS/osscbtf_v2.3.1
export BOOST_ROOT=/u/glschult/OSS/osscbtf_v2.3.1
#export BOOST_ROOT=/nasa/pkgsrc/sles12/2016Q4/views/boost/1.62
#export ICU_PATH=/nasa/pkgsrc/2016Q2

cmake .. \
        -DCMAKE_BUILD_TYPE=None \
        -DCMAKE_CXX_FLAGS="-g -O2" \
        -DCMAKE_C_FLAGS="-g -O2" \
        -DCMAKE_INSTALL_PREFIX=${CBTF_ROOT} \
        -DCMAKE_PREFIX_PATH=${KRELL_ROOT} \
        -DMRNET_DIR=${MRNET_ROOT} \
	-DICU_ROOT_DIR=${ICU_PATH} \
	-DCMAKE_FIND_ROOT_PATH=${BOOST_ROOT} 

make clean
make
make install

