////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the depth parsing function. */

#pragma once

#include <xercesc/dom/DOM.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Parse the specified DepthType node and return a flag indicating whether
     * the parsed depth specification selects the current MRNet filter.
     *
     * @param root                Root node of the XML tree containing the
     *                            filter depth specification in a DepthType
     *                            node.
     * @param is_on_leaf_cp       Boolean "true" if this MRNet filter is
     *                            located on a leaf communication process
     *                            (CP) or "false" otherwise.
     * @param[in,out] selected    Boolean "true" if this depth specification
     *                            selects the current MRNet filter, or "false"
     *                            otherwise. The caller must set the initial
     *                            value of this parameter to indicate if the
     *                            filter has already been selected.
     */
    void parseDepth(const xercesc::DOMNode* root, bool is_on_leaf_cp,
                    bool& selected);

} } } // namespace KrellInstitute::CBTF::Impl
