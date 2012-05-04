////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
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

#include "MessageHandlers.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Namespace containing functions to start and stop a message pump for
     * the backend, which is responsible for receiving and handling incoming
     * messages from the frontend. Also provides the means to configure message
     * handlers and send messages to the frontend.
     *
     * @sa http://en.wikipedia.org/wiki/Event_loop
     */
    namespace Backend
    {

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

        /** Message handlers for this backend. */
        extern KrellInstitute::CBTF::Impl::MessageHandlers MessageHandlers;

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
