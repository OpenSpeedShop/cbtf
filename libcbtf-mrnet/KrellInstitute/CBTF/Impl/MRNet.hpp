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

    /**
     * Plain old data (POD) structure containing topological information for
     * a single node in an MRNet network.
     *
     * @sa http://en.wikipedia.org/wiki/Plain_old_data_structure
     */
    struct TopologyInfo
    {

        /** Flag indicating if this node is located on the MRNet frontend. */
        bool IsFrontend;
        
        /** Flag indicating if this node is located on a MRNet backend. */
        bool IsBackend;

        /** Rank number of this node within the network. */
        unsigned int Rank;

        /** Number of direct children of this node. */
        unsigned int NumChildren;
        
        /** Number of siblings to this node. */
        unsigned int NumSiblings;

        /** Number of descendants of this node. */
        unsigned int NumDescendants;

        /** Number of leaves below this node. */
        unsigned int NumLeafDescendants;

        /** Distance from this node to the network's root node. */
        unsigned int RootDistance;

        /** Maximum distance from this node to one of its leaf descendants. */
        unsigned int MaxLeafDistance;
        
    }; // struct TopologyInfo

    /**
     * Topological information for this MRNet node.
     *
     * @note    Using a single global to contain the per-node topological
     *          information implies each node can participate in only one
     *          MRNet network. Currently MRNet itself has this limitation
     *          due to its own extensive use of globals. In the future,
     *          if this limitation is removed, a different mechanism will
     *          be needed to access the topological information.
     */
    extern TopologyInfo TheTopologyInfo;
            
} } } // namespace KrellInstitute::CBTF::Impl
