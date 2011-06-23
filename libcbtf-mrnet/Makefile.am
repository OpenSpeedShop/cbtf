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

lib_LTLIBRARIES = libcbtf-mrnet.la

libcbtf_mrnet_la_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-DBINDIR="\"$(bindir)\"" \
	-DLIBDIR="\"$(libdir)\"" \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ @CBTF_XML_CPPFLAGS@ \
	@LIBXERCES_C_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

libcbtf_mrnet_la_LDFLAGS = \
	-version-info 0:0:0 \
	@BOOST_LDFLAGS@ \
	@CBTF_LDFLAGS@ @CBTF_XML_LDFLAGS@ \
	@LIBXERCES_C_LDFLAGS@ \
	@MRNET_LDFLAGS@

libcbtf_mrnet_la_LIBADD = \
	@BOOST_SYSTEM_LIB@ @BOOST_THREAD_LIB@ \
	@CBTF_LIBS@ @CBTF_XML_LIBS@ \
	@LIBXERCES_C@ \
	@MRNET_LIBS@

libcbtf_mrnet_la_SOURCES = \
	AtomicCounter.hpp \
	Frontend.hpp Frontend.cpp \
	IncomingStreamMediator.hpp IncomingStreamMediator.cpp \
	LocalComponentNetwork.hpp LocalComponentNetwork.cpp \
	MessageHandler.hpp \
	MessageTags.hpp \
	MRNet.hpp MRNet.cpp \
	NamedStreams.hpp NamedStreams.cpp \
	OutgoingStreamMediator.hpp OutgoingStreamMediator.cpp
