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

plugindir = $(libdir)/KrellInstitute/CBTF

plugin_LTLIBRARIES = BasicMRNetLaunchers.la

BasicMRNetLaunchers_la_CXXFLAGS = \
	-I$(top_srcdir)/include \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

BasicMRNetLaunchers_la_LDFLAGS = \
	-module -avoid-version \
	-L$(top_srcdir)/src \
	@CBTF_LDFLAGS@ \
	@MRNET_LDFLAGS@

BasicMRNetLaunchers_la_LIBADD = \
	-lcbtf-mrnet \
	@CBTF_LIBS@ \
	@MRNET_LIBS@

BasicMRNetLaunchers_la_SOURCES = \
	SimpleMRNetLauncher.cpp
