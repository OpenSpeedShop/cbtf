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

/** @file Definition of the Backend namespace. */

#include <algorithm>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <iostream>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <map>
#include <mrnet/MRNet.h>
#include <stdexcept>
#include <sys/select.h>

#include "Backend.hpp"
#include "MessageTags.hpp"
#include "Raise.hpp"

using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Flag indicating if debugging is enabled for this backend. */
    bool is_backend_debug_enabled = false;

    /** Message handlers for this backend. */
    std::map<int, MessageHandler> message_handlers;
    
    /** Mutual exclusion lock for this backend's message handlers. */
    boost::shared_mutex message_handlers_mutex;
    
    /** MRNet network containing this backend. */
    MRN::Network* mrnet_network = NULL;

    /** MRNet stream used to pass data within this network. */
    MRN::Stream* mrnet_stream = NULL;

    /** Thread executing this backend's message pump. */
    boost::thread message_pump_thread;
    
    /**
     * Implementation of the backend's message pump. Executing the pump within
     * a separate thread insures the incoming messages are handled in a timely
     * manner. The pump simply receives incoming messages and then dispatches
     * them to the appropriate handler until it is instructed to exit.
     */
    void doMessagePump()
    {
        // Get the file descriptor for MRNet data event notification
        int mrnet_fd = mrnet_network->get_EventNotificationFd(
            MRN::Event::DATA_EVENT
            );
        
        // Run the message pump until instructed to exit
        try
        {
            while (true)
            {
                // Initialize the set of incoming file descriptors
                int nfds = 0;
                fd_set readfds;
                FD_ZERO(&readfds);
                
                nfds = std::max(nfds, mrnet_fd + 1);
                FD_SET(mrnet_fd, &readfds);
                
                // Initialize a one second timeout
                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                
                // Wait for file descriptor activity or timeout expiration
                int retval = select(nfds, &readfds, NULL, NULL, &timeout);

                // Is MRNet indicating there is incoming data available?
                if ((retval > 0) && FD_ISSET(mrnet_fd, &readfds))
                {
                    while (true)
                    {
                        // Receive the next available message
                        int tag = -1;
                        MRN::Stream* stream = NULL;
                        MRN::PacketPtr packet;
                        int retval = mrnet_network->recv(
                            &tag, packet, &stream, false
                            );
                        if (retval == 0)
                        {
                            break;
                        }
                        else if ((retval == -1) || (packet == NULL))
                        {
                            raise<std::runtime_error>(
                                "MRNet failed to receive the next message."
                                );
                        }

                        // Dispatch the message to the proper handler

                        MessageHandler handler;

                        {
                            boost::shared_lock<boost::shared_mutex> 
                                guard_message_handlers(message_handlers_mutex);
                            std::map<int, MessageHandler>::const_iterator i =
                                message_handlers.find(tag);
                            if (i != message_handlers.end())
                            {
                                handler = i->second;
                            }
                        }

                        if (is_backend_debug_enabled)
                        {
                            std::cout << "[BE " << getpid() << "] "
                                      << "Received and "
                                      << (handler ? "handling" : "ignoring")
                                      << " " << tag << "." << std::endl;
                        }

                        if (handler)
                        {
                            try
                            {
                                handler(packet);
                            }
                            catch (const std::exception& error)
                            {
                                std::cout << "[BE " << getpid() 
                                          << "] EXCEPTION: "
                                          << error.what() << std::endl;
                            }
                        }
                        
                        // Reset MRNet data event notification
                        mrnet_network->clear_EventNotificationFd(
                            MRN::Event::DATA_EVENT
                            );
                    }
                }
                
                // Manually provide a thread interruption point
                boost::this_thread::interruption_point();
            }
        }
        catch (const boost::thread_interrupted&)
        {
            //
            // Intentionally squash this exception. When the thread executing
            // the message pump is interrupted, all it needs to do is return.
            //
        }
    } // doMessagePump()
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
// Set the message handler for the specified message tag.
//------------------------------------------------------------------------------
void Backend::setMessageHandler(const int& tag, const MessageHandler& handler)
{
    boost::unique_lock<boost::shared_mutex> guard_message_handlers(
        message_handlers_mutex
        );
    message_handlers.insert(std::make_pair(tag, handler));    
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Backend::startMessagePump(int argc, char* argv[])
{
    // Determine the type of debugging that should be enabled
    bool is_tracing_debug_enabled = false;
    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == std::string("--debug"))
        {
            is_backend_debug_enabled = true;
        }
        else if (std::string(argv[i]) == std::string("--tracing"))
        {
            is_tracing_debug_enabled = true;
        }
    }
    
    // Initialize the MRNet library (participating as a backend)
    if (is_tracing_debug_enabled)
    {
        MRN::set_OutputLevel(MRN::MAX_OUTPUT_LEVEL);
    }
    mrnet_network = MRN::Network::CreateNetworkBE(argc, argv);
    if (mrnet_network->has_Error())
    {
        raise<std::runtime_error>("Unable to initialize MRNet.");
    }
    
    // Initialize the topological information for this MRNet node

    MRN::TopologyLocalInfo topology_info(
        mrnet_network->get_NetworkTopology(),
        mrnet_network->get_NetworkTopology()->find_Node(
            mrnet_network->get_LocalRank()
            )
        );
    
    TheTopologyInfo.Rank = topology_info.get_Rank();
    TheTopologyInfo.NumChildren = topology_info.get_NumChildren();
    TheTopologyInfo.NumSiblings = topology_info.get_NumSiblings();
    TheTopologyInfo.NumDescendants = topology_info.get_NumDescendants();
    TheTopologyInfo.NumLeafDescendants = topology_info.get_NumLeafDescendants();
    TheTopologyInfo.RootDistance = topology_info.get_RootDistance();
    TheTopologyInfo.MaxLeafDistance = topology_info.get_MaxLeafDistance();
    
    // Establish the stream used to pass data within this network
    int tag = -1;
    MRN::PacketPtr packet;
    if ((mrnet_network->recv(&tag, packet, &mrnet_stream, true) != 1) ||
        (tag != MessageTags::EstablishUpstream) ||
        (mrnet_stream == NULL))
    {
        raise<std::runtime_error>("Unable to connect to the frontend.");
    }
    
    // Start a thread executing this backend's message pump
    message_pump_thread = boost::thread(doMessagePump);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Backend::stopMessagePump()
{
    // Interrupt the thread executing this backend's message pump
    message_pump_thread.interrupt();

    // Wait for the thread to actually exit
    message_pump_thread.join();

    // Destroy the stream used to pass data within this network
    delete mrnet_stream;
    
    // Finalize the MRNet library
    mrnet_network->waitfor_ShutDown();
    delete mrnet_network;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Backend::sendToFrontend(const MRN::PacketPtr& packet)
{
    if (is_backend_debug_enabled)
    {
        std::cout << "[BE " << getpid() << "] "
                  << "Sending " << packet->get_Tag() << "." << std::endl;
    }

    // Insure the packet containing the message has the correct stream ID
    if (mrnet_stream == NULL)
    {
        raise<std::runtime_error>(
            "The MRNet stream hasn't been created yet."
            );
    }
    packet->set_StreamId(mrnet_stream->get_Id());

    // Send the message
    int retval = mrnet_stream->send(
        const_cast<MRN::PacketPtr&>(packet)
        );
    if (retval != 0)
    {
        raise<std::runtime_error>("Failed to send the specified message.");
    }
    retval = mrnet_stream->flush();
    if (retval != 0)
    {
        raise<std::runtime_error>("Failed to send the specified message.");
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Backend::isDebugEnabled()
{
    return is_backend_debug_enabled;
}
