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

/** @file Declaration of MRNet implementation functions. */

#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include <vector>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Get the path of the MRNet backend executable. Typically used by MRNet
     * launcher components when starting backend processes.
     *
     * @return    Path of the MRNet backend executable.
     *
     * @throw std::runtime_error    The path of the MRNet backend could not
     *                              be resolved.
     */
    boost::filesystem::path getMRNetBackendPath();

    /**
     * Get the arguments to the MRNet backend executable. Typically used by
     * MRNet launcher components when starting backend processes.
     *
     * @return    Arguments to the MRNet backend executable.
     */
    std::vector<std::string> getMRNetBackendArguments();
            
} } } // namespace KrellInstitute::CBTF::Impl
