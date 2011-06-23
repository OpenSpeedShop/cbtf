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

include(FindPackageHandleStandardArgs)

find_library(MRNet_MRNET_LIBRARY NAMES libmrnet.so)
find_library(MRNet_XPLAT_LIBRARY NAMES libxplat.so)
find_path(MRNet_INCLUDE_DIR mrnet/MRNet.h)

find_package_handle_standard_args(
    MRNet DEFAULT_MSG MRNet_MRNET_LIBRARY MRNet_XPLAT_LIBRARY MRNet_INCLUDE_DIR
    )

set(MRNet_LIBRARIES ${MRNet_MRNET_LIBRARY} ${MRNet_XPLAT_LIBRARY})
set(MRNet_INCLUDE_DIRS ${MRNet_INCLUDE_DIR})

set(MRNet_DEFINES "-Dos_linux")

mark_as_advanced(MRNet_MRNET_LIBRARY MRNet_XPLAT_LIBRARY MRNet_INCLUDE_DIR)
