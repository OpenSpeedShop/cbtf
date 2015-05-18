################################################################################
# Copyright (c) 2012-2015 Krell Institute. All Rights Reserved.
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
# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
#SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/cray/xt-asyncpe/default/bin/cc -dynamic)
SET(CMAKE_CXX_COMPILER /opt/cray/xt-asyncpe/default/bin/CC -dynamic)

# where is the target environment - point to the non-standard installations of targeted software
#SET(CMAKE_FIND_ROOT_PATH  $CBTF_KRELL_ROOT/$CBTF_TARGET_ARCH $CBTF_BOOST_ROOT/$CBTF_TARGET_ARCH)
SET(CMAKE_FIND_ROOT_PATH  /users/jeg/todi/krell_root/cray-xk /users/jeg/todi/boost_root/cray-xk)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


