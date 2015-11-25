################################################################################
# Copyright (c) 2011-2013 Krell Institute. All Rights Reserved.
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

add_library(cbtf SHARED
    KrellInstitute/CBTF/BoostExts.hpp
    KrellInstitute/CBTF/Component.hpp Component.cpp
    ComponentImpl.hpp ComponentImpl.cpp
    Global.hpp
    KrellInstitute/CBTF/Impl/InvokerForAny.hpp
    KrellInstitute/CBTF/Impl/InvokerFor.hpp
    KrellInstitute/CBTF/Impl/Invoker.hpp
    Raise.hpp
    ResolvePath.hpp ResolvePath.cpp
    KrellInstitute/CBTF/SignalAdapter.hpp
    KrellInstitute/CBTF/Type.hpp Type.cpp
    TypeImpl.hpp TypeImpl.cpp
    KrellInstitute/CBTF/Version.hpp Version.cpp
    KrellInstitute/CBTF/ValueSink.hpp
    KrellInstitute/CBTF/ValueSource.hpp
    VersionImpl.hpp VersionImpl.cpp
    )

add_definitions(
  -DCBTF_DATA_FILE_DIR="${CMAKE_INSTALL_PREFIX}/share/KrellInstitute/CBTF"
  -DCBTF_EXECUTABLE_FILE_DIR="${CMAKE_INSTALL_PREFIX}/bin"
  -DCBTF_LIBRARY_FILE_DIR="${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
  -DCBTF_PLUGIN_FILE_DIR="${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/KrellInstitute/CBTF"
  )
  
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    )

target_link_libraries(cbtf
    -Wl,--no-as-needed
    ${CMAKE_DL_LIBS}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    )

set_target_properties(cbtf PROPERTIES VERSION 1.1.0)

install(DIRECTORY KrellInstitute DESTINATION include)

install(TARGETS cbtf
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    )
