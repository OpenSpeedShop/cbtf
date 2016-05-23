################################################################################
# Copyright (c) 2011-2016 Krell Institute. All Rights Reserved.
# Copyright (c) 2012 Argo Navis Technologies. All Rights Reserved.
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
#
# CBTF builds require 2.8.11 because we are now using cmake features that 
# became available at that version.
#
cmake_minimum_required(VERSION 2.8.11)

project(CBTF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# NOTE related to the following C++11 feature checks:
#    This include: CheckCXX11Features.cmake is projected to be in a future version of CMake. 
#    When this does get included into the new CMake version, we will update our 
#    required CMake version and remove cmake/CheckCXX11Features* files from the cbtf repository.
#

include(cmake/CheckCXX11Features.cmake)
if(NOT HAS_CXX11_AUTO)
  message(STATUS "No support for C++11 found.")
else()
  message(STATUS "C++11 support found, required flags are: ${CXX11_COMPILER_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")
endif()


# If targetted build, only install the include files needed by cbtf-krell
# Currently this only applies to the Intel MIC platform, where we do not
# need all of cbtf, like we do when targetting the Cray platform.
if (RUNTIME_ONLY MATCHES "true")
    # Install include files targetted platform builds. Install only the
    # includes for libcbtf-mrnet
    add_subdirectory(libcbtf-mrnet)
else()
    # Build all components, libraries, etc.  full build.
    find_package(Boost 1.39.0 REQUIRED
        COMPONENTS date_time filesystem system thread unit_test_framework
    )
    find_package(MRNet 4.0.0)
    find_package(Threads)
    find_package(XercesC 3.0)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc64*")
        set(CMAKE_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib64)
        set(LIB_SUFFIX 64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc*")
        set(CMAKE_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    elseif(EXISTS /usr/lib64)
        set(LIB_SUFFIX 64)
        set(CMAKE_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    endif()

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

    install(FILES cmake/FindMRNet.cmake cmake/FindXercesC.cmake 
        DESTINATION share/KrellInstitute/cmake
    )

    install(FILES cmake/FindCBTF.cmake cmake/CBTFConfig.cmake 
        DESTINATION share/CBTF/cmake
    )
endif()
