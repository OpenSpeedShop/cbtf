////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Krell Institute. All Rights Reserved.
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

/** @file Definition of the MessageHandlers class. */

#include <boost/thread/locks.hpp>
#include <vector>

#include "MessageHandlers.hpp"

using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MessageHandlers::MessageHandlers() :    
    dm_mutex(),
    dm_handlers()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MessageHandlers::~MessageHandlers()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MessageHandlers::add(const int& uid, const int& tag,
                          const MessageHandler& handler)
{
    boost::unique_lock<boost::shared_mutex> guard_this(dm_mutex);

    Row row;
    row.UID = uid;
    row.MessageTag = tag;
    row.Handler = handler;

    dm_handlers.insert(row);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MessageHandlers::remove(const int& uid)
{
    boost::unique_lock<boost::shared_mutex> guard_this(dm_mutex);

    dm_handlers.get<0>().erase(uid);
}



//------------------------------------------------------------------------------
// Note that the handlers must be called here while NOT holding the lock. If
// this isn't done, handlers which themselves add (or remove) other handlers
// will cause a deadlock.
//------------------------------------------------------------------------------
bool MessageHandlers::operator()(
    const int& tag,
    const MRN::PacketPtr& packet
    ) const
{
    std::vector<MessageHandler> handlers;

    {
        boost::shared_lock<boost::shared_mutex> guard_this(dm_mutex);
        
        for (HandlerMap::nth_index<1>::type::const_iterator
                 i = dm_handlers.get<1>().lower_bound(tag);
             i != dm_handlers.get<1>().upper_bound(tag);
             ++i)
        {
            handlers.push_back(i->Handler);
        }
    }

    for (std::vector<MessageHandler>::const_iterator
             i = handlers.begin(); i != handlers.end(); ++i)
    {
        (*i)(packet);
    }

    return !handlers.empty();
}
