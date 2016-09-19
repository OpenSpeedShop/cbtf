rm -rf  build
mkdir build
pushd build

export CC=gcc
export CXX=g++

export KRELL_ROOT=/opt/DEVEL4/krellroot_v2.2.3
export CBTF_ROOT=/opt/DEVEL4/cbtf_v2.2.4
export MRNET_ROOT=/opt/DEVEL4/krellroot_v2.2.3

cmake3 .. \
        -DCMAKE_BUILD_TYPE=None \
        -DCMAKE_CXX_FLAGS="-g -O2" \
        -DCMAKE_C_FLAGS="-g -O2" \
        -DCMAKE_INSTALL_PREFIX=${CBTF_ROOT} \
        -DCMAKE_PREFIX_PATH=${KRELL_ROOT} \
        -DMRNET_DIR=${MRNET_ROOT}

make clean
make
make install

