////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the DOMNodeHandler type. */

#pragma once

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <xercesc/dom/DOM.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Type of function for handling a DOM node. Pointers to the DOM document
     * containing the node, and to the node itself, are passed to the handler
     * as parameters.
     *
     * @sa http://en.wikipedia.org/wiki/Event_handler
     * @sa http://xercesc.apache.org/xerces-c/apiDocs-3/classDOMDocument.html
     * @sa http://xercesc.apache.org/xerces-c/apiDocs-3/classDOMNode.html
     */
    typedef boost::function<
        void (const boost::shared_ptr<xercesc::DOMDocument>&,
              const xercesc::DOMNode*)
        > DOMNodeHandler;

} } } // namespace KrellInstitute::CBTF::Impl
