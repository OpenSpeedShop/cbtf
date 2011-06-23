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

if(DEFINED XERCESC_FOUND)
    set(XML_SOURCES "test-xml.cpp")
    set(XML_TARGET "cbtf-xml")
else(DEFINED XERCESC_FOUND)
    set(XML_SOURCES "")
    set(XML_TARGET "")
endif(DEFINED XERCESC_FOUND)

add_library(plugin MODULE plugin.cpp)
add_library(plugin-xml MODULE plugin-xml.cpp)
add_executable(test test.cpp ${XML_SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/libcbtf)

if(DEFINED XERCESC_FOUND)
    include_directories(${PROJECT_SOURCE_DIR}/libcbtf-xml)
endif(DEFINED XERCESC_FOUND)

target_link_libraries(test
    cbtf ${XML_TARGET}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    )

set_target_properties(plugin PROPERTIES PREFIX "")
set_target_properties(plugin-xml PROPERTIES PREFIX "")

if(DEFINED XERCESC_FOUND)
    file(COPY test-xml.xml DESTINATION .)
endif(DEFINED XERCESC_FOUND)
