////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the XML component registration function. */

#pragma once

#include <boost/filesystem.hpp>

namespace KrellInstitute { namespace CBTF {

    /**
     * Register a XML document describing one or more networks of connected
     * components. Each component network is, itself, a component with zero
     * or more inputs and outputs. A new component type is defined for each
     * child of the document's root element, and these new component types
     * can be discovered and instantiated via the Component class.
     *
     * @param path    Path of the XML document to be registered.
     *
     * @throw std::runtime_error    The specified XML document doesn't
     *                              exist, is not of the correct format,
     *                              or one of the specified plugins or
     *                              components couldn't be found.
     *
     * @note    By itself this library only provides in-process component
     *          networks described by <Network> nodes that conform to the
     *          NetworkType found in the "Network.xsd" schema. Additional
     *          kinds of component networks (e.g. <MRNet>) are defined by
     *          other libraries, but XML documents using these extensions
     *          are still registered via this function.
     */
    void registerXML(const boost::filesystem::path& path);

} } // namespace KrellInstitute::CBTF
