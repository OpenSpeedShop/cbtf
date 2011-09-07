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

add_library(cbtf-xml SHARED
    DOMNodeHandler.hpp
    InputMediator.hpp
    Network.cpp Network.hpp
    OutputMediator.hpp
    XercesExts.hpp XercesExts.cpp
    KrellInstitute/CBTF/XML.hpp XML.hpp XML.cpp
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/libcbtf
    ${Boost_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    )
  
target_link_libraries(cbtf-xml
    cbtf
    ${Boost_FILESYSTEM_LIBRARY}
    ${XercesC_LIBRARIES}
    )

set_target_properties(cbtf-xml PROPERTIES VERSION 0.0.0)

install(FILES Network.xsd DESTINATION share/KrellInstitute/CBTF)

install(DIRECTORY KrellInstitute DESTINATION include)

install(TARGETS cbtf-xml
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
