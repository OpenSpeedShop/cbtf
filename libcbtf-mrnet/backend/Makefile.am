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

bin_PROGRAMS = libcbtf-mrnet-backend

libcbtf_mrnet_backend_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ @CBTF_XML_CPPFLAGS@ \
	@LIBXERCES_C_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

libcbtf_mrnet_backend_LDFLAGS = \
	-L$(top_srcdir)/src \
	@BOOST_LDFLAGS@ \
	@CBTF_LDFLAGS@ @CBTF_XML_LDFLAGS@ \
	@LIBXERCES_C_LDFLAGS@ \
	@MRNET_LDFLAGS@ 

libcbtf_mrnet_backend_LDADD = \
	-lcbtf-mrnet \
	@BOOST_DATE_TIME_LIB@ @BOOST_SYSTEM_LIB@ @BOOST_THREAD_LIB@ \
	@CBTF_LIBS@ @CBTF_XML_LIBS@ \
	@LIBXERCES_C@ \
	@MRNET_LIBS@

libcbtf_mrnet_backend_SOURCES = \
	Backend.hpp Backend.cpp \
	main.cpp
