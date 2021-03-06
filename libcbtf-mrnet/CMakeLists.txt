################################################################################
# Copyright (c) 2011-2016 Krell Institute. All Rights Reserved.
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

if (RUNTIME_ONLY MATCHES "true" AND CBTF_FILTERS_RUN_ON MATCHES "host")
    # Install this include file for target compute node builds (currently the Intel MIC)
    install(FILES KrellInstitute/CBTF/Impl/MessageTags.h DESTINATION include/KrellInstitute/CBTF/Impl)
else()
    add_library(cbtf-mrnet SHARED
        AtomicCounter.hpp
        Frontend.cpp Frontend.hpp
        IncomingStreamMediator.cpp IncomingStreamMediator.hpp
        LocalComponentNetwork.cpp LocalComponentNetwork.hpp
        MessageHandler.hpp
        MessageHandlers.cpp MessageHandlers.hpp
        KrellInstitute/CBTF/Impl/MessageTags.h
        MessageTags.hpp
        KrellInstitute/CBTF/Impl/MRNet.hpp MRNet.cpp MRNet.hpp
        NamedStreams.cpp NamedStreams.hpp
        OutgoingStreamMediator.cpp OutgoingStreamMediator.hpp
        StreamMediator.cpp StreamMediator.hpp
        KrellInstitute/CBTF/XDR.hpp
        )
    
    add_definitions(
        -DBACKEND_FILE="libcbtf-mrnet-backend"
        -DFILTER_FILE="libcbtf-mrnet-filter.so"
        )
    
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libcbtf
        ${PROJECT_SOURCE_DIR}/libcbtf-xml
        ${Boost_INCLUDE_DIRS}
        ${MRNet_INCLUDE_DIRS}
        ${XercesC_INCLUDE_DIRS}
        )
    
    target_link_libraries(cbtf-mrnet
        -Wl,--no-as-needed
        cbtf cbtf-xml
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${MRNet_LIBRARIES}
        ${XercesC_LIBRARIES}
        )
    
    set_target_properties(cbtf-mrnet PROPERTIES VERSION 1.1.0)
    set_target_properties(cbtf-mrnet PROPERTIES COMPILE_DEFINITIONS "${MRNet_DEFINES}")

    install(FILES MRNet.xsd DESTINATION share/KrellInstitute/CBTF)
    install(FILES LLNLrsh DESTINATION share/KrellInstitute/CBTF)
    install(DIRECTORY KrellInstitute DESTINATION include)
    
    install(TARGETS cbtf-mrnet
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        )
endif()
    
