////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the path resolution functions. */

#pragma once

#include <boost/filesystem.hpp>
#include <vector>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /** 
     * Enumeration of the file types for which path resolution can be applied.
     */
    enum FileType
    {
        kDataFileType,
        kExecutableFileType,
        kLibraryFileType,
        kPluginFileType
    };

    /**
     * Prepend the specified path to the search paths for the given file type.
     *
     * @param type    File type for which to append a search path.
     * @param path    Path to be prepended.
     */
    void prependToSearchPath(const FileType& type,
                             const boost::filesystem::path& path);

    /**
     * Append the specified path to the search paths for the given file type.
     *
     * @param type    File type for which to append a search path.
     * @param path    Path to be appended.
     */
    void appendToSearchPath(const FileType& type,
                            const boost::filesystem::path& path);

    /**
     * Resolve the specified relative path to an absolute path using the given
     * search paths. The search paths are examined in order. An empty path is
     * returned if the path could not be resolved.
     *
     * @param search_paths    Search paths used for this resolution.
     * @param path            Relative path to be resolved.
     * @return                Absolute path resolved from this relative path,
     *                        or an empty path if the relative path could not
     *                        be resolved.
     *
     * @note    If the specified relative path is already an absolute path, it
     *          is returned unmodified.
     */
    boost::filesystem::path resolvePath(
        const std::vector<boost::filesystem::path>& search_paths,
        const boost::filesystem::path& path
        );

    /**
     * Resolve the specified relative path to an absolute path using the search
     * paths for the given file type. All of the current search paths for this
     * file type are searched in order. An empty path is returned if the path
     * could not be resolved.
     *
     * @param type    File type used for this resolution.
     * @param path    Relative path to be resolved.
     * @return        Absolute path resolved from this relative path, or an
     *                empty path if the relative path could not be resolved.
     *
     * @note    If the specified relative path is already an absolute path, it
     *          is returned unmodified.
     */
    boost::filesystem::path resolvePath(const FileType& type,
                                        const boost::filesystem::path& path);

} } } // namespace KrellInstitute::CBTF::Impl
