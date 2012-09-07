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

/** @file Declaration of the MessageHandlers class. */

#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <mrnet/Packet.h>

#include "MessageHandler.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Container used to track the handler(s) associated with specific MRNet
     * message tags. Used by the MRNet frontend, communication processes, and
     * non-lightweight backends to route messages appropriately.
     */
    class MessageHandlers :
        private boost::noncopyable
    {

    public:
        
        /** Construct an empty message handler table. */
        MessageHandlers();
        
        /** Destructor. */
        virtual ~MessageHandlers();

        /**
         * Add a message handler.
         *
         * @param uid        Unique identifier for the distribured component
         *                   network associated with this message handler.
         * @param tag        Message tag for which a handler is to be added.
         * @param handler    Handler to be added for that message tag.
         */
        void add(const int& uid, const int& tag, const MessageHandler& handler);
        
        /**
         * Remove message handlers.
         *
         * @param uid    Unique identifier for the distributed component
         *               network for which handlers are to be removed.
         */
        void remove(const int& uid);
        
        /**
         * Invoke message handlers.
         *
         * @param tag       Message tag for which handlers are to be invoked.
         * @param packet    Packet containing the message to be passed to the
         *                  handlers.
         * @return          Boolean "true" if one or more handlers were invoked
         *                  for this packet, or "false" otherwise.
         */
        bool operator()(const int& tag, const MRN::PacketPtr& packet) const;

    private:

        /**
         * Plain old data (POD) structure describing one row in the table.
         *
         * @sa http://en.wikipedia.org/wiki/Plain_old_data_structure
         */
        struct Row
        {
            /**
             * Unique identifier for the distributed component
             * network associated with this message handler.
             */
            int UID;
            
            /** Message tag being handled. */
            int MessageTag;
            
            /** Handler for this message tag. */
            MessageHandler Handler;
        };
        
        /**
         * Type of associative container used to map unique identifiers
         * for distributed component networks, and message tags, to the
         * corresponding handler(s).
         */
        typedef boost::multi_index_container<
            Row,
            boost::multi_index::indexed_by<
                boost::multi_index::ordered_non_unique<
                    boost::multi_index::member<Row, int, &Row::UID>
                    >,
                boost::multi_index::ordered_non_unique<
                    boost::multi_index::member<Row, int, &Row::MessageTag>
                    >
                >
            > HandlerMap;

        /** Mutual exclusion lock for this message handler table. */
        mutable boost::shared_mutex dm_mutex;

        /** Contents of this message handler table. */
        HandlerMap dm_handlers;
        
    }; // class MessageHandlers

} } } // namespace KrellInstitute::CBTF::Impl
