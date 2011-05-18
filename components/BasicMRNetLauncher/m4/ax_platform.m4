################################################################################
# Copyright (c) 2011 Krell Institute. All Rights Reserved.
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

AC_DEFUN([AX_PLATFORM], [

    AC_ARG_WITH(ppc64_bitmode,
                AC_HELP_STRING([--with-ppc64-bitmode=<32,64>],
                               [Specify ppc64 library bit mode @<:@32@:>@]),
                ppc64_bitmode=$withval, ppc64_bitmode="32")

    AC_MSG_NOTICE([applying platform-specific settings])

    case "$host" in

        powerpc-*-linux*) 
            abi_libdir="lib"
            alt_abi_libdir="lib64"
            if test x"$libdir" == x'${exec_prefix}/lib64'; then
                libdir='${exec_prefix}/lib'
            fi
            LDFLAGS="-L/usr/lib -DLIB_DIR=lib $LDFLAGS"
            ;;

        powerpc64-*-*)
            if test "$ppc64_bitmode" = "64" ; then
		        abi_libdir="lib64"
		        alt_abi_libdir="lib"
		        if test x"$libdir" == x'${exec_prefix}/lib'; then
		            libdir='${exec_prefix}/lib64'
		        fi
		        LDFLAGS="-L/usr/lib64 -DLIB_DIR=lib64 $LDFLAGS"
		        CFLAGS="-m64 $CFLAGS"
		        CXXFLAGS="-m64 $CXXFLAGS"
		        CPPFLAGS="-m64 $CPPFLAGS"
            elif test "$ppc64_bitmode" = "32" ; then
                abi_libdir="lib"
                alt_abi_libdir="lib64"
		        if test x"$libdir" == x'${exec_prefix}/lib64'; then
                    libdir='${exec_prefix}/lib'
		        fi
                LDFLAGS="-L/usr/lib -DLIB_DIR=lib $LDFLAGS"
            fi
            ;;

        x86_64-*-linux*)
            if test x"$libdir" == x'${exec_prefix}/lib'; then
                libdir='${exec_prefix}/lib64'
            fi
            abi_libdir="lib64"
            alt_abi_libdir="lib"
            LDFLAGS="-L/usr/lib64 $LDFLAGS"
            ;;

        *)
            abi_libdir="lib"
            alt_abi_libdir="lib64"
            LDFLAGS="-L/usr/lib -DLIB_DIR=lib $LDFLAGS"
            ;;

    esac

])
