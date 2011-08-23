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

/** @file Declaration of the path resolution functions. */

#pragma once

#include <boost/filesystem.hpp>

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
     * Resolve the specified relative path to a file of the given type into
     * an absolute path. All of the current search paths for this file type
     * are searched.
     *
     * @param type    Type of the file.
     * @param path    Relative path to the file.
     * @return        Absolute path to the file.
     *
     * @throw std::runtime_error    The specified relative path to a file
     *                              of the given type could not be resolved.
     *
     * @note    If an absolute path is specified, it is returned unmodified.
     */
    boost::filesystem::path resolvePath(const FileType& type,
                                        const boost::filesystem::path& path);

} } } // namespace KrellInstitute::CBTF::Impl
