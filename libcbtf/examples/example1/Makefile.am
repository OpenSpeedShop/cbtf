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

noinst_PROGRAMS = test

test_CXXFLAGS = -I$(top_srcdir)/include -DBUILDDIR="\"$(abs_builddir)\"" @BOOST_CPPFLAGS@
test_LDFLAGS = -L$(top_srcdir)/src @BOOST_LDFLAGS@
test_LDADD = -lcbtf @BOOST_FILESYSTEM_LIB@ @BOOST_UNIT_TEST_FRAMEWORK_LIB@
test_SOURCES = test.cpp

noinst_LTLIBRARIES = ExampleComponentA.la ExampleComponentB.la

ExampleComponentA_la_CXXFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@
ExampleComponentA_la_LDFLAGS = \
	-L$(top_srcdir)/src -module -avoid-version -shared -rpath /ForceShared 
ExampleComponentA_la_LIBADD = -lcbtf
ExampleComponentA_la_SOURCES = ExampleComponentA.cpp

ExampleComponentB_la_CXXFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@
ExampleComponentB_la_LDFLAGS = \
	-L$(top_srcdir)/src -module -avoid-version -shared -rpath /ForceShared 
ExampleComponentB_la_LIBADD = -lcbtf
ExampleComponentB_la_SOURCES = ExampleComponentB.cpp

