#!/bin/bash 

set -x

export bmode=""
if [ `uname -m` = "x86_64" -o `uname -m` = " x86-64" ]; then
    LIBDIR="lib64"
    ALTLIBDIR="lib"
    echo "UNAME IS X86_64 FAMILY: LIBDIR=$LIBDIR"
    export LIBDIR="lib64"
elif [ `uname -m` = "ppc64" ]; then
   if [ $CBTF_PPC64_BITMODE_32 ]; then
    LIBDIR="lib"
    ALTLIBDIR="lib64"
    echo "UNAME IS PPC64 FAMILY, but 32 bitmode: LIBDIR=$LIBDIR"
    export LIBDIR="lib"
    export bmode="--with-ppc64-bitmod=32"
   else
    LIBDIR="lib64"
    ALTLIBDIR="lib"
    echo "UNAME IS PPC64 FAMILY, with 64 bitmode: LIBDIR=$LIBDIR"
    export LIBDIR="lib64"
    export CFLAGS=" -m64 $CFLAGS "
    export CXXFLAGS=" -m64 $CXXFLAGS "
    export CPPFLAGS=" -m64 $CPPFLAGS "
    export bmode="--with-ppc64-bitmod=64"
   fi
elif [ `uname -m` = "ppc" ]; then
    LIBDIR="lib"
    ALTLIBDIR="lib64"
    echo "UNAME IS PPC FAMILY: LIBDIR=$LIBDIR"
    export LIBDIR="lib"
    export bmode="--with-ppc64-bitmod=32"
else
    LIBDIR="lib"
    ALTLIBDIR="lib64"
    export LIBDIR="lib"
    echo "UNAME IS X86 FAMILY: LIBDIR=$LIBDIR"
fi

sys=`uname -n `
export MACHINE=$sys
echo ""
echo '    machine: ' $sys

if [ -z "$CBTF_MPI_MPICH2" ]; then
 export CBTF_MPI_MPICH2=/usr
fi

if [ -z "$CBTF_MPI_MVAPICH" ]; then
 export CBTF_MPI_MVAPICH=/usr
fi

if [ -z "$CBTF_MPI_MVAPICH2" ]; then
 export CBTF_MPI_MVAPICH2=/usr
fi

if [ -z "$CBTF_MPI_MPT" ]; then
 export CBTF_MPI_MPT=/usr
fi

if [ -z "$CBTF_MPI_OPENMPI" ]; then
 export CBTF_MPI_OPENMPI=/usr
fi

echo "-------------------------------------------------------------"
echo "-- START BUILDING CBTF  -------------------------------------"
echo "-------------------------------------------------------------"

echo "-- BUILDING FRAMEWORK/LIBCBTF -----------------------------"
echo "-----------------------------------------------------------"
echo "-------------------------------------------------------------"
echo "-- BUILDING FRAMEWORK/LIBCBTF-XML ---------------------------"
echo "-------------------------------------------------------------"
echo "-------------------------------------------------------------"
echo "-- BUILDING FRAMEWORK/LIBCBTF-MRNET -------------------------"
echo "-------------------------------------------------------------"

if [ -f $CBTF_BOOST_ROOT/$LIBDIR/libboost_serialization.so -o -f $CBTF_BOOST_ROOT/$LIBDIR/libboost_serialization-mt.so ]; then
  export CBTF_BOOST_ROOT_LIB=$CBTF_BOOST_ROOT/$LIBDIR
  export BOOST_LIBRARYDIR=$CBTF_BOOST_ROOT/$LIBDIR
  export BOOST_ROOT=$CBTF_BOOST_ROOT
  export BOOSTROOT=$CBTF_BOOST_ROOT
  export Boost_DIR=$CBTF_BOOST_ROOT
else
  export CBTF_BOOST_ROOT_LIB=$CBTF_BOOST_ROOT/lib
  export BOOST_ROOT=$CBTF_BOOST_ROOT
  export BOOSTROOT=$CBTF_BOOST_ROOT
  export Boost_DIR=$CBTF_BOOST_ROOT
  export BOOST_LIBRARYDIR=$CBTF_BOOST_ROOT/lib
fi

#cd framework

if [ ! -d "build" ]; then
    mkdir build
fi
cd build

export CC=`which gcc`
export CXX=`which c++`
export CPLUSPLUS=`which c++`

if [ -z "$BOOST_ROOT" ]; then
   cmake -DCMAKE_CXX_COMPILE=$CPLUSPLUS -DCMAKE_INSTALL_PREFIX=$CBTF_PREFIX ..
else
    if [[ ${BOOST_ROOT:0:6} == '/usr/l' ]];
    then
        echo 'BOOST_ROOT starts with /usr/l, i.e. - in default location'
   	cmake -DCMAKE_CXX_COMPILE=$CPLUSPLUS -DCMAKE_INSTALL_PREFIX=$CBTF_PREFIX ..
    else
        echo 'BOOST_ROOT does NOT start with /usr/l, i.e. - not in default location'
	export Boost_NO_SYSTEM_PATHS=TRUE
        cmake -DBoost_NO_SYSTEM_PATHS=TRUE -DCMAKE_CXX_COMPILE=$CPLUSPLUS -DCMAKE_INSTALL_PREFIX=$CBTF_PREFIX ..
    fi
fi

make
make install

if [ -f $CBTF_PREFIX/$LIBDIR/libcbtf.so -a -f $CBTF_PREFIX/$LIBDIR/libcbtf-xml.so -a -f $CBTF_PREFIX/$LIBDIR/libcbtf-mrnet.so ]; then
   echo "CBTF SERVICES BUILT SUCCESSFULLY into $CBTF_PREFIX."
else
   echo "CBTF SERVICES FAILED TO BUILD - TERMINATING BUILD SCRIPT.  Please check for errors."
   exit
fi

cd ..

echo "-------------------------------------------------------------"
echo "-- END OF BUILDING CBTF  ------------------------------------"
echo "-------------------------------------------------------------"
