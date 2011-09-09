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

if(XERCESC_FOUND)
    set(XML_SOURCES "test-xml.cpp")
    set(XML_LIBRARIES "cbtf-xml")
endif()

if(XERCESC_FOUND AND MRNET_FOUND)
    set(MRNET_SOURCES "test-mrnet.cpp")
    set(MRNET_LIBRARIES "cbtf-mrnet")
endif()

add_library(plugin MODULE plugin.cpp)
add_library(plugin-xml MODULE plugin-xml.cpp)

if(XERCESC_FOUND AND MRNET_FOUND)
    add_library(plugin-mrnet MODULE plugin-mrnet.cpp)
    add_definitions(${MRNet_DEFINES})
endif()

add_executable(test
    ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.h
    ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.c
    test.cpp
    ${XML_SOURCES}
    ${MRNET_SOURCES}
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.h
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.c
    COMMAND rpcgen -h -o 
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.h
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.x
    COMMAND rpcgen -c -o 
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.c
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.x
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/TestMessage.x
    )

add_definitions(
    -DCBTF_TEST_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    -DCBTF_MRNET_BACKEND_BINARY_DIR="${CMAKE_BINARY_DIR}/libcbtf-mrnet-backend"
    -DCBTF_MRNET_FILTER_BINARY_DIR="${CMAKE_BINARY_DIR}/libcbtf-mrnet-filter"
    -DCBTF_MRNET_LAUNCHERS_BINARY_DIR="${CMAKE_BINARY_DIR}/BasicMRNetLaunchers"
    )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/libcbtf
    ${Boost_INCLUDE_DIRS}
    )

if(XERCESC_FOUND)
    include_directories(${PROJECT_SOURCE_DIR}/libcbtf-xml)
endif()

if(XERCESC_FOUND AND MRNET_FOUND)
    include_directories(
        ${PROJECT_SOURCE_DIR}/libcbtf-mrnet
        ${MRNet_INCLUDE_DIRS}
        )
endif()

target_link_libraries(test
    cbtf
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${MRNET_LIBRARIES}
    ${XML_LIBRARIES}
    )

set_target_properties(plugin PROPERTIES PREFIX "")
set_target_properties(plugin-xml PROPERTIES PREFIX "")

if(XERCESC_FOUND AND MRNET_FOUND)
    set_target_properties(plugin-mrnet PROPERTIES PREFIX "")
endif()

file(COPY TestMessage.x DESTINATION .)

if(XERCESC_FOUND)
    file(COPY test-xml.xml DESTINATION .)
endif()

if(XERCESC_FOUND AND MRNET_FOUND)
    file(COPY test-mrnet.topology DESTINATION .)
    configure_file(test-mrnet.xml .)
endif()
