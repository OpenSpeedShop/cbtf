################################################################################
# Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

lib_LTLIBRARIES = libcbtf-xml.la

libcbtf_xml_la_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-DDATADIR="\"$(datadir)\"" \
	-DLIBDIR="\"$(libdir)\"" \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@LIBXERCES_C_CPPFLAGS@

libcbtf_xml_la_LDFLAGS = \
	-version-info 0:0:0 \
	@BOOST_LDFLAGS@ \
	@CBTF_LDFLAGS@ \
	@LIBXERCES_C_LDFLAGS@

libcbtf_xml_la_LIBADD = \
	@BOOST_FILESYSTEM_LIB@ \
	@CBTF_LIBS@ \
	@LTLIBXERCES_C@

libcbtf_xml_la_SOURCES = \
	Network.hpp Network.cpp \
	XercesExts.cpp \
	XML.cpp
