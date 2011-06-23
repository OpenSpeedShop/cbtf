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

/** @file Declaration of the NamedStreams class. */

#pragma once

#include <boost/bimap.hpp>
#include <boost/utility.hpp>
#include <iostream>
#include <mrnet/Packet.h>
#include <string>
#include <xercesc/dom/DOM.hpp>

#include "AtomicCounter.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Container used to track the named streams used for communication between
     * the local component networks on the backends, filters, and frontend.
     */
    class NamedStreams :
        private boost::noncopyable
    {
        friend std::ostream& operator<<(std::ostream& stream,
                                        const NamedStreams& named_streams);

    public:

        /**
         * Construct the named streams for the distributed (via MRNet) network
         * of connected components described by the specified XML tree.
         *
         * @param root    Root node of the XML tree describing the
         *                component network.
         *
         * @note    The root node of the provided XML tree must conform
         *          to the MRNetType described in the "MRNet.xsd" schema.
         *
         * @note    The MRNet frontend uses this constructor.
         */
        NamedStreams(const xercesc::DOMNode* root);

        /**
         * Construct the named streams described by the specified MRNet packet.
         *
         * @param packet    Packet describing the named streams.
         *
         * @note    The MRNet filters and backends use this constructor.
         */
        NamedStreams(const MRN::PacketPtr& packet);

        /**
         * Type conversion to an MRNet packet describing these named streams.
         *
         * @return    Packet describing the named streams.
         */
        operator MRN::PacketPtr() const;

        /**
         * Get the unique identifier for this distributed component network.
         *
         * @return    Unique identifier for this distributed component network.
         */
        int uid() const;

        /**
         * Get the MRNet message tag corresponding to the given named stream.
         *
         * @param name    Named stream.
         * @return        Corresponding MRNet message tag.
         *
         * @throw std::runtime_error    The requested named
         *                              stream doesn't exist.
         */
        int tag(const std::string& name) const;
        
    private:

        /** Parse the specified [Incoming|Outgoing]StreamType node. */
        void parseStream(const xercesc::DOMNode* node);

        /** Parse the specified StreamDeclarationType node. */
        void parseStreamDeclaration(const xercesc::DOMNode* node);

        /** Generator of unique identifiers. */
        static AtomicCounter<int> dm_uid_generator;
        
        /** Generator of unique tag offsets. */
        static AtomicCounter<int> dm_offset_generator;

        /** Unique identifier for the distributed component network. */
        int dm_uid;

        /** Map of named streams to their corresponding MRNet message tags. */
        boost::bimap<std::string, int> dm_tags;
        
    }; // class NamedStreams

    /**
     * Redirection to an output stream.
     *
     * @param stream           Destination output stream.
     * @param named_streams    Named streams to be redirected.
     * @return                 Destination output stream.
     */
    std::ostream& operator<<(std::ostream& stream,
                             const NamedStreams& named_streams);

} } } // namespace KrellInstitute::CBTF::Impl
