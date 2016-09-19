rm -rf  build
mkdir build
pushd build

export CC=gcc
export CXX=g++

export KRELL_ROOT=/opt/DEVEL4/krellroot_v2.2.2
export CBTF_ROOT=/opt/DEBUG4/cbtf_v2.2.2
export MRNET_ROOT=/opt/DEVEL4/krellroot_v2.2.2

cmake .. \
        -DCMAKE_BUILD_TYPE=None \
        -DCMAKE_CXX_FLAGS="-g" \
        -DCMAKE_C_FLAGS="-g" \
        -DCMAKE_INSTALL_PREFIX=${CBTF_ROOT} \
        -DCMAKE_PREFIX_PATH=${KRELL_ROOT} \
        -DMRNET_DIR=${MRNET_ROOT}

make clean
make
make install

