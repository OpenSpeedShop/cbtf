################################################################################
# Copyright (c) 2011-2015 Krell Institute. All Rights Reserved.
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

add_library(BasicMRNetLaunchers SHARED
    BasicMRNetLauncherUsingBackendAttach.cpp
    BasicMRNetLauncherUsingBackendCreate.cpp
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/libcbtf
    ${PROJECT_SOURCE_DIR}/libcbtf-xml
    ${PROJECT_SOURCE_DIR}/libcbtf-mrnet
    ${Boost_INCLUDE_DIRS}
    ${MRNet_INCLUDE_DIRS}
    )

target_link_libraries(BasicMRNetLaunchers
    cbtf-mrnet
    ${Boost_FILESYSTEM_LIBRARY}
    ${MRNet_LIBRARIES}
    )

set_target_properties(BasicMRNetLaunchers PROPERTIES PREFIX "")
set_target_properties(BasicMRNetLaunchers PROPERTIES COMPILE_DEFINITIONS "${MRNet_DEFINES}")

install(TARGETS BasicMRNetLaunchers
    RUNTIME DESTINATION bin/KrellInstitute/CBTF
    LIBRARY DESTINATION lib${LIB_SUFFIX}/KrellInstitute/CBTF
    ARCHIVE DESTINATION lib${LIB_SUFFIX}/KrellInstitute/CBTF
    )
