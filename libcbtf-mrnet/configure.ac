################################################################################
# Copyright (c) 2010 Krell Institute. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

AC_INIT([libcbtf-mrnet], [1.0])
AC_COPYRIGHT([[Copyright (c) 2010 Krell Institute. All Rights Reserved.]])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

AH_TOP([
/*******************************************************************************
** Copyright (c) 2010 Krell Institute. All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/
])

AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PATH_PROG([RPCGEN], rpcgen)

LT_INIT()

################################################################################
# Handle PowerPC and X86-64 Special Cases
################################################################################
AC_ARG_WITH(ppc64_bitmode,
            AC_HELP_STRING([--with-ppc64-bitmode=<32,64>],
                           [Specify ppc64 library bit mode @<:@32@:>@]),
            ppc64_bitmode=$withval, ppc64_bitmode="32")

AC_MSG_CHECKING([Configuring for host = $host  with ppc64_bitmode = $ppc64_bitmode])
AC_MSG_RESULT(yes)

case "$host" in
    powerpc-*-linux*) 
        AC_MSG_CHECKING([Configuring for powerpc-star-linux])

        abi_libdir="lib"
        alt_abi_libdir="lib64"
        if test x"$libdir" == x'${exec_prefix}/lib64'; then
            libdir='${exec_prefix}/lib'
        fi
        LDFLAGS="-L/usr/lib -DLIB_DIR=lib $LDFLAGS"
	AC_MSG_RESULT(yes)
        ;;
    powerpc64-*-*) 

	AC_MSG_CHECKING([Configuring for powerpc64-star-linux with ppc64_bitmode = $ppc64_bitmode])
	if test "$ppc64_bitmode" = "64" ; then
		AC_MSG_CHECKING([Configuring for 64 bit powerpc64-star-linux host])
		abi_libdir="lib64"
		alt_abi_libdir="lib"
		if test x"$libdir" == x'${exec_prefix}/lib'; then
		    libdir='${exec_prefix}/lib64'
		fi
		LDFLAGS="-L/usr/lib64 -DLIB_DIR=lib64 $LDFLAGS"
		CFLAGS="-m64 $CFLAGS"
		CXXFLAGS="-m64 $CXXFLAGS"
		CPPFLAGS="-m64 $CPPFLAGS"
	        AC_MSG_RESULT(yes)
	elif test "$ppc64_bitmode" = "32" ; then
		AC_MSG_CHECKING([Configuring for 32 bit powerpc64-star-linux host])
		abi_libdir="lib"
		alt_abi_libdir="lib64"
		if test x"$libdir" == x'${exec_prefix}/lib64'; then
		    libdir='${exec_prefix}/lib'
		fi
		LDFLAGS="-L/usr/lib -DLIB_DIR=lib $LDFLAGS"
	        AC_MSG_RESULT(yes)
	fi
	;;

    x86_64-*-linux*)
        AC_MSG_CHECKING([Configuring for x86_64--star-linux host])
        if test x"$libdir" == x'${exec_prefix}/lib'; then
            libdir='${exec_prefix}/lib64'
        fi
        abi_libdir="lib64"
        alt_abi_libdir="lib"
        LDFLAGS="-L/usr/lib64 $LDFLAGS"
        AC_MSG_RESULT(yes)
        ;;

    *)
        AC_MSG_CHECKING([Configuring for default host])
        abi_libdir="lib"
        alt_abi_libdir="lib64"
        LDFLAGS="-L/usr/lib -DLIB_DIR=lib $LDFLAGS"
        AC_MSG_RESULT(yes)
        ;;
esac

AX_BOOST_BASE([1.35.0])
AX_BOOST_DATE_TIME()
AX_BOOST_SYSTEM()
AX_BOOST_THREAD()
AX_BOOST_UNIT_TEST_FRAMEWORK()
AX_CBTF()

AX_XERCESC()

AC_MSG_CHECKING([for libxerces-c >= 3.0.0])
AC_LANG_PUSH(C++)

xerces_saved_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $LIBXERCES_C_CPPFLAGS"
AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[
    #include "xercesc/util/XercesVersion.hpp"
    ]], [[
    #if _XERCES_VERSION < 3000
        #error "libxerces-c version is too old!"
    #endif
    ]]), AC_MSG_RESULT(yes), [ AC_MSG_RESULT(no)
    AC_MSG_FAILURE(libxerces-c version isn't 3.0.0 or higher.)
    ]
)
CPPFLAGS=$xerces_saved_CPPFLAGS

AX_CBTF_XML()
AX_MRNET()

AC_CONFIG_FILES([
    Makefile
    backend/Makefile
    data/Makefile
    filter/Makefile
    include/Makefile
    src/Makefile
    test/Makefile
])

AC_OUTPUT
