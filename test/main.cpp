////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Main entry point for the CBTF unit test. */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE CBTF

#include <boost/test/unit_test.hpp>

#include "ResolvePath.hpp"



/** Global fixture adding the testing search paths. */
struct AddTestingSearchPaths
{
    AddTestingSearchPaths()
    {
        KrellInstitute::CBTF::Impl::prependToSearchPath(
            KrellInstitute::CBTF::Impl::kPluginFileType,
            CBTF_TEST_BINARY_DIR
            );
        KrellInstitute::CBTF::Impl::prependToSearchPath(
            KrellInstitute::CBTF::Impl::kDataFileType,
            CBTF_MRNET_SOURCE_DIR
            );
        KrellInstitute::CBTF::Impl::prependToSearchPath(
            KrellInstitute::CBTF::Impl::kExecutableFileType,
            CBTF_MRNET_BACKEND_BINARY_DIR
            );
        KrellInstitute::CBTF::Impl::prependToSearchPath(
            KrellInstitute::CBTF::Impl::kLibraryFileType,
            CBTF_MRNET_FILTER_BINARY_DIR
            );
        KrellInstitute::CBTF::Impl::prependToSearchPath(
            KrellInstitute::CBTF::Impl::kPluginFileType,
            CBTF_MRNET_LAUNCHERS_BINARY_DIR
            );
        KrellInstitute::CBTF::Impl::prependToSearchPath(
            KrellInstitute::CBTF::Impl::kDataFileType,
            CBTF_XML_SOURCE_DIR
            );
    }
};

BOOST_GLOBAL_FIXTURE(AddTestingSearchPaths);
