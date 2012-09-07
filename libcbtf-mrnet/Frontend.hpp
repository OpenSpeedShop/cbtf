////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the Frontend class. */

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <mrnet/MRNet.h>

#include "MessageHandlers.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * RAII class representing the frontend of a MRNet network. The lifetime of
     * the network mirrors that of the corresponding Frontend object. Starts and
     * stops a message pump for the frontend, which is responsible for receiving
     * and handling incoming messages from the backends. Also provides the means
     * to configure message handlers and send messages to the backends.
     *
     * @sa http://en.wikipedia.org/wiki/Event_loop
     * @sa http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization
     */
    class Frontend :
        private boost::noncopyable
    {

    public:

        /**
         * Instantiate the frontend for the given MRNet network if it hasn't
         * been instantiated already, or return the existing frontend if one
         * already exists.
         *
         * @param network        MRNet network containing this frontend.
         * @param filter_mode    MRNet filter synchronization mode to be used.
         */
        static boost::shared_ptr<Frontend> instantiate(
            const boost::shared_ptr<MRN::Network>& network,
            const MRN::FilterId& filter_mode = MRN::SFILTER_DONTWAIT
            );
        
        /**
         * Destroy this frontend. Instructs all of the corresponding network's
         * backends to exit, waits for them to do so, and then terminates this
         * frontend's message pump.
         */
        virtual ~Frontend();

        /** Message handlers for this frontend. */
        KrellInstitute::CBTF::Impl::MessageHandlers MessageHandlers;
        
        /**
         * Send a message to all of the backends.
         *
         * @param packet    Packet containing the message to be sent.
         *
         * @throw std::runtime_error    Unable to send the message.
         */
        void sendToBackends(const MRN::PacketPtr& packet);
        
    private:

        /**
         * Construct the frontend for the given MRNet network. Establishes the
         * stream used by backends to pass data to the frontend, then starts a
         * thread executing this frontend's message pump.
         *
         * @param network        MRNet network containing this frontend.
         * @param filter_mode    MRNet filter synchronization mode to be used.
         *
         * @throw std::runtime_error    Unable to initialize MRNet.
         */
        Frontend(const boost::shared_ptr<MRN::Network>& network,
                 const MRN::FilterId& filter_mode);
        
        /** Implementation of the message pump. */
        void doMessagePump();

        /** Flag indicating if debugging is enabled for this frontend. */
        bool dm_is_debug_enabled;
        
        /** MRNet network containing this frontend. */
        boost::shared_ptr<MRN::Network> dm_network;

        /** MRNet stream used to pass data within this network. */
        MRN::Stream* dm_stream;
        
        /** Thread executing this frontend's message pump. */
        boost::thread dm_message_pump_thread;
        
    }; // class Frontend

} } } // namespace KrellInstitute::CBTF::Impl
