////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010,2011 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the Backend namespace. */

#pragma once

#include <mrnet/MRNet.h>

#include "MessageHandler.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Namespace containing functions to set message handlers for, starting,
     * and stopping the MRNet backend message pump. This pump is responsible
     * for receiving and handling incoming messages from the frontend. Also
     * provides functions for sending messages to the frontend.
     *
     * @sa http://en.wikipedia.org/wiki/Event_loop
     */
    namespace Backend
    {

        /**
         * Set a message handler for this backend's message pump. Requests the
         * given handler be invoked when messages with the specified tag arrive
         * at this backend's message pump.
         *
         * @param tag        Message tag for which the handler is to be set.
         * @param handler    New handler for that message tag.
         *
         * @note    The message handler for a particular message tag can be
         *          effectively removed by specifying a default constructed
         *          message handler.
         */
        void setMessageHandler(const int& tag, const MessageHandler& handler);
        
        /**
         * Start this backend's message pump. Initializes the MRNet library as
         * a backend, gathering necessary parameters directly from the command-
         * line arguments. Establishes the stream used by backends to pass data
         * to the frontend, and then starts the thread executing this backend's
         * message pump.
         *
         * @param argc    Number of command-line arguments.
         * @param argv    Array of command-line arguments.
         *
         * @throw std::runtime_error    Unable to initialize MRNet, or to
         *                              connect to the frontend.
         */
        void startMessagePump(int argc, char* argv[]);

        /**
         * Stop this backend's message pump. Instructs the thread executing
         * this backend's message pump to exit, and then waits for it to do
         * so. Also finalizes the MRNet library.
         */
        void stopMessagePump();

        /**
         * Send a message to the frontend.
         *
         * @param packet    Packet containing the message to be sent.
         *
         * @throw std::runtime_error    Unable to send the message.
         */
        void sendToFrontend(const MRN::PacketPtr& packet);

        /**
         * Return a flag indicating if debugging for the backend is enabled.
         *
         * @return    Boolean "true" if debugging is enabled, "false" otherwise.
         */
        bool isDebugEnabled();

    } // namespace Backend

} } } // namespace KrellInstitute::CBTF::Impl
