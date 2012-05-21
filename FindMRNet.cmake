################################################################################
# Copyright (c) 2011,2012 Krell Institute. All Rights Reserved.
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

include(FindPackageHandleStandardArgs)

find_library(MRNet_MRNET_LIBRARY NAMES libmrnet.so HINTS ENV MRNET_ROOT)
find_library(MRNet_XPLAT_LIBRARY NAMES libxplat.so HINTS ENV MRNET_ROOT)
find_path(MRNet_INCLUDE_DIR mrnet/MRNet.h HINTS ENV MRNET_ROOT)

find_package_handle_standard_args(
    MRNet DEFAULT_MSG MRNet_MRNET_LIBRARY MRNet_XPLAT_LIBRARY MRNet_INCLUDE_DIR
    )

set(MRNet_LIBRARIES ${MRNet_MRNET_LIBRARY} ${MRNet_XPLAT_LIBRARY})

set(MRNet_DEFINES "-Dos_linux")

if(MRNET_FOUND AND DEFINED MRNet_INCLUDE_DIR)

    file(READ ${MRNet_INCLUDE_DIR}/mrnet/Types.h MRNet_VERSION_FILE)
  
    string(REGEX REPLACE
        ".*#[ ]*define MRNET_VERSION_MAJOR[ ]+([0-9]+)\n.*" "\\1"
        MRNet_VERSION_MAJOR ${MRNet_VERSION_FILE}
        )
      
    string(REGEX REPLACE
        ".*#[ ]*define MRNET_VERSION_MINOR[ ]+([0-9]+)\n.*" "\\1"
        MRNet_VERSION_MINOR ${MRNet_VERSION_FILE}
        )
  
    string(REGEX REPLACE
        ".*#[ ]*define MRNET_VERSION_REV[ ]+([0-9]+)\n.*" "\\1"
        MRNet_VERSION_PATCH ${MRNet_VERSION_FILE}
        )
  
    set(MRNet_VERSION_STRING 
      ${MRNet_VERSION_MAJOR}.${MRNet_VERSION_MINOR}.${MRNet_VERSION_PATCH}
      )
  
    message(STATUS "MRNet version: " ${MRNet_VERSION_STRING})

    if(DEFINED MRNet_FIND_VERSION)
        if(${MRNet_VERSION_STRING} VERSION_LESS ${MRNet_FIND_VERSION})

            set(MRNET_FOUND FALSE)

            if(DEFINED MRNet_FIND_REQUIRED)
                message(FATAL_ERROR
                    "Could NOT find MRNet  (version < "
                    ${MRNet_FIND_VERSION} ")"
                    )
            else()
                message(STATUS
                    "Could NOT find MRNet  (version < " 
                    ${MRNet_FIND_VERSION} ")"
                    )
            endif()
 
        endif()
    endif()
  
endif()

if(MRNet_VERSION_STRING VERSION_LESS "4.0.0")

    set(MRNet_INCLUDE_DIRS ${MRNet_INCLUDE_DIR})
    mark_as_advanced(MRNet_MRNET_LIBRARY MRNet_XPLAT_LIBRARY MRNet_INCLUDE_DIR)

else()

    #
    # Find the MRNet 4 (and up) configuration header files. These are found
    # in the lib[64] subdirectory rather than the include subdirectory where
    # one would expect to find them...
    #

    find_path(
        MRNet_CONFIG_INCLUDE_DIR1 mrnet_config.h
        PATH_SUFFIXES lib/mrnet-${MRNet_VERSION_STRING}/include lib64/mrnet-${MRNet_VERSION_STRING}/include HINTS ENV MRNET_ROOT
        )

    if(NOT MRNet_CONFIG_INCLUDE_DIR1)
        message(FATAL_ERROR
          "Could NOT find the MRNet " ${MRNet_VERSION_STRING}
          " mrnet configuration header files"
          )
    endif()

    find_path(
        MRNet_CONFIG_INCLUDE_DIR2 xplat_config.h
        PATH_SUFFIXES lib/xplat-${MRNet_VERSION_STRING}/include lib64/xplat-${MRNet_VERSION_STRING}/include HINTS ENV MRNET_ROOT
        )

    if(NOT MRNet_CONFIG_INCLUDE_DIR2)
        message(FATAL_ERROR
          "Could NOT find the MRNet " ${MRNet_VERSION_STRING}
          " xplat configuration header files"
          )
    endif()

    set(MRNet_INCLUDE_DIRS ${MRNet_INCLUDE_DIR} ${MRNet_CONFIG_INCLUDE_DIR1} ${MRNet_CONFIG_INCLUDE_DIR2})

    mark_as_advanced(
        MRNet_MRNET_LIBRARY MRNet_XPLAT_LIBRARY
        MRNet_INCLUDE_DIR MRNet_CONFIG_INCLUDE_DIR1 MRNet_CONFIG_INCLUDE_DIR2
        )

endif()



