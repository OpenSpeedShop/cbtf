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

cmake_minimum_required(VERSION 2.8.0)

project(CBTF)


if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc*")
    set(CMAKE_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib)
elseif(EXISTS /usr/lib64)
    set(LIB_SUFFIX 64)
    set(CMAKE_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Boost 1.35.0 REQUIRED
    COMPONENTS date_time filesystem system thread unit_test_framework
    )
find_package(MRNet 3.1)
find_package(Threads)
find_package(XercesC 3.0)

add_subdirectory(libcbtf)

if(XERCESC_FOUND)
    add_subdirectory(libcbtf-xml)
endif()

if(XERCESC_FOUND AND MRNET_FOUND)
    add_subdirectory(libcbtf-mrnet)
    add_subdirectory(libcbtf-mrnet-backend)
    add_subdirectory(libcbtf-mrnet-filter)
    add_subdirectory(BasicMRNetLaunchers)
endif()

add_subdirectory(examples)
add_subdirectory(test)
