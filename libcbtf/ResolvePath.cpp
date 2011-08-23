////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Definition of the path resolution functions. */

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/spirit/home/classic.hpp>
#include <cstdlib>
#include <map>
#include <stdexcept>
#include <vector>

#include "Global.hpp"
#include "Raise.hpp"
#include "ResolvePath.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /**
     * Global associative container mapping file types to their search paths.
     */
    KRELL_INSTITUTE_CBTF_IMPL_GLOBAL(
        SearchPaths,
        std::map<
           FileType BOOST_PP_COMMA() std::vector<boost::filesystem::path>
           >
        )

    /**
     * Statically initialized C++ structure adding the default search paths.
     */
    struct AddDefaultSearchPaths
    {
        AddDefaultSearchPaths()
        {
            const char* cbtf_plugin_paths = getenv("CBTF_PLUGIN_PATH");
            if (cbtf_plugin_paths != NULL)
            {
                using namespace boost::spirit::classic;

                parse(
                    cbtf_plugin_paths,
                    list_p((+~ch_p(':'))[
                        boost::bind(appendToSearchPath, kPluginFileType, _1)
                        ], ch_p(':')),
                    space_p
                    );
            }

            appendToSearchPath(kDataFileType, CBTF_DATA_FILE_DIR);
            appendToSearchPath(kExecutableFileType, CBTF_EXECUTABLE_FILE_DIR);
            appendToSearchPath(kLibraryFileType, CBTF_LIBRARY_FILE_DIR);
            appendToSearchPath(kPluginFileType, CBTF_PLUGIN_FILE_DIR);
        }
    } add_default_search_paths;

} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void KrellInstitute::CBTF::Impl::prependToSearchPath(
    const FileType& type,
    const boost::filesystem::path& path
    )
{
    SearchPaths::GuardType guard_search_paths(SearchPaths::mutex());

    SearchPaths::Type::iterator i = SearchPaths::value().find(type);
    if (i == SearchPaths::value().end())
    {
        i = SearchPaths::value().insert(
            std::make_pair(type, SearchPaths::Type::mapped_type())
            ).first;
    }

    i->second.insert(i->second.begin(), path);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void KrellInstitute::CBTF::Impl::appendToSearchPath(
    const FileType& type,
    const boost::filesystem::path& path
    )
{
    SearchPaths::GuardType guard_search_paths(SearchPaths::mutex());

    SearchPaths::Type::iterator i = SearchPaths::value().find(type);
    if (i == SearchPaths::value().end())
    {
        i = SearchPaths::value().insert(
            std::make_pair(type, SearchPaths::Type::mapped_type())
            ).first;
    }

    i->second.push_back(path);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::filesystem::path KrellInstitute::CBTF::Impl::resolvePath(
    const FileType& type,
    const boost::filesystem::path& path
    )
{
    if (path.is_complete())
    {
        return path;
    }

    SearchPaths::Type::const_iterator i = SearchPaths::value().find(type);
    if (i != SearchPaths::value().end())
    {
        for (SearchPaths::Type::mapped_type::const_iterator
                 j = i->second.begin(); j != i->second.end(); ++j)
        {
            boost::filesystem::path candidate = *j / path;
            if (boost::filesystem::exists(candidate))
            {
                return candidate;
            }
        }
    }

    raise<std::runtime_error>(
        "The specified relative path (%1%) to a file of "
        "the given type (%2%) could not be resolved.", path, type
        );
}
