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

/** @file Declaration of XML implementation functions. */

#pragma once

#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Impl/DOMNodeHandler.hpp>
#include <string>
#include <xercesc/dom/DOM.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Register a kind of component network. The given handler is invoked each
     * time the specified tag is encountered while registering an XML document.
     *
     * @param tag        XML tag identifying the kind of component network.
     * @param handler    Handler invoked each time this tag is encountered.
     *
     * @note    Kinds of component networks can be registered while loading
     *          an executable, before a try/catch block could possibly be in
     *          place. Thus attempts to register a kind of component network
     *          more than once are silently ignored and no exception is thrown.
     */
    void registerKindOfComponentNetwork(const std::string& tag,
                                        const DOMNodeHandler& handler);

    /**
     * Instantiate a new component directly from a XML tree describing the
     * network of connected components. The component's type is <em>not</em>
     * registered with the Component class and thus cannot be discovered or
     * instantiated via that class.
     *
     * @param document    Document containing the XML tree describing
     *                    the component network to be instantiated.
     * @param root        Root node of the XML tree describing the
     *                    component network to be instantiated.
     *
     * @throw std::runtime_error    The specified XML tree is not of the
     *                              correct format, or one of the specified
     *                              plugins or components couldn't be found.
     *
     * @note    The root node of the provided XML tree must conform to
     *          the NetworkType described in the "Network.xsd" schema.
     */
    Component::Instance instantiateXML(
        const boost::shared_ptr<xercesc::DOMDocument>& document,
        const xercesc::DOMNode* root
        );

} } } // namespace KrellInstitute::CBTF::Impl
