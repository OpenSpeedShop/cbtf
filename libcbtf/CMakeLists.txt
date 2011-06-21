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

add_library(cbtf SHARED
    KrellInstitute/CBTF/BoostExts.hpp
    KrellInstitute/CBTF/Component.hpp Component.cpp
    ComponentImpl.hpp ComponentImpl.cpp
    KrellInstitute/CBTF/Impl/InvokerForAny.hpp
    KrellInstitute/CBTF/Impl/InvokerFor.hpp
    KrellInstitute/CBTF/Impl/Invoker.hpp
    KrellInstitute/CBTF/Impl/Raise.hpp
    KrellInstitute/CBTF/Type.hpp Type.cpp
    TypeImpl.hpp TypeImpl.cpp
    KrellInstitute/CBTF/Version.hpp Version.cpp
    KrellInstitute/CBTF/ValueSink.hpp
    KrellInstitute/CBTF/ValueSource.hpp
    VersionImpl.hpp VersionImpl.cpp
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
    )
  
target_link_libraries(cbtf
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${CMAKE_DL_LIBS}
    )

set_target_properties(cbtf PROPERTIES VERSION 0.0.0)

install(DIRECTORY KrellInstitute DESTINATION include)

install(TARGETS cbtf
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
