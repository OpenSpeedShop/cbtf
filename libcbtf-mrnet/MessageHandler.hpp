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

/** @file Declaration of the MessageHandler type. */

#pragma once

#include <boost/function.hpp>
#include <mrnet/Packet.h>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Type of function for handling a message. A pointer to the MRNet packet
     * containing the message is passed to the handler as a parameter.
     *
     * @sa http://en.wikipedia.org/wiki/Event_handler
     */
    typedef boost::function<void (const MRN::PacketPtr&)> MessageHandler;

} } } // namespace KrellInstitute::CBTF::Impl
