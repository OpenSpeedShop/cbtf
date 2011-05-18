################################################################################
# Copyright (c) 2010,2011 Krell Institute. All Rights Reserved.
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

noinst_PROGRAMS = test

test_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-DBUILDDIR="\"$(abs_builddir)\"" \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ @CBTF_XML_CPPFLAGS@ \
	@LIBXERCES_C_CPPFLAGS@ \
	@MRNET_CPPFLAGS@ \

test_LDFLAGS = \
	-L$(top_srcdir)/src \
	@BOOST_LDFLAGS@ \
	@CBTF_LDFLAGS@ @CBTF_XML_LDFLAGS@ \
	@LIBXERCES_C_LDFLAGS@ \
	@MRNET_LDFLAGS@

test_LDADD = \
	-lcbtf-mrnet \
	@BOOST_UNIT_TEST_FRAMEWORK_LIB@ \
	@CBTF_LIBS@ @CBTF_XML_LIBS@ \
	@LIBXERCES_C@ \
	@MRNET_LIBS@

test_SOURCES = \
	messages.x messages.h messages.c \
	test.cpp

SUFFIXES = .x
CLEANFILES = messages.h messages.c

messages.h messages.c : messages.x
	rm -f  $(patsubst %.x, %.h, $<) $(patsubst %.x, %.c, $<)
	$(RPCGEN) -h -o $(patsubst %.x, %.h, $<) $<
	$(RPCGEN) -c -o $(patsubst %.x, %.c, $<) $<

noinst_LTLIBRARIES = plugin.la

plugin_la_CXXFLAGS = \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

plugin_la_LDFLAGS = \
	-module -avoid-version -rpath /ForceShared \
	@CBTF_LDFLAGS@ \
	@MRNET_LDFLAGS@

plugin_la_LIBADD = \
	@CBTF_LIBS@ \
	@MRNET_LIBS@

plugin_la_SOURCES = plugin.cpp
