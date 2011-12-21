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

/** @file Definition of the Frontend class. */

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/locks.hpp>
#include <iostream>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <sys/select.h>
#include <vector>

#include "Frontend.hpp"
#include "MessageTags.hpp"
#include "Raise.hpp"
#include "ResolvePath.hpp"

using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Frontend::Frontend(const boost::shared_ptr<MRN::Network>& network,
                   const MRN::FilterId& filter_mode) :
    dm_is_debug_enabled(false),
    dm_message_handlers(),
    dm_message_handlers_mutex(),
    dm_network(network),
    dm_stream(NULL),
    dm_message_pump_thread()
{
    // Initialize the topological information for this MRNet node
    
    MRN::TopologyLocalInfo topology_info(
        dm_network->get_NetworkTopology(),
        dm_network->get_NetworkTopology()->find_Node(
            dm_network->get_LocalRank()
            )
        );
    
    TheTopologyInfo.Rank = topology_info.get_Rank();
    TheTopologyInfo.NumChildren = topology_info.get_NumChildren();
    TheTopologyInfo.NumSiblings = topology_info.get_NumSiblings();
    TheTopologyInfo.NumDescendants = topology_info.get_NumDescendants();
    TheTopologyInfo.NumLeafDescendants = topology_info.get_NumLeafDescendants();
    TheTopologyInfo.RootDistance = topology_info.get_RootDistance();
    TheTopologyInfo.MaxLeafDistance = topology_info.get_MaxLeafDistance();
    
    // Determine the type of debugging that should be enabled
    dm_is_debug_enabled = 
        ((getenv("CBTF_DEBUG_MRNET") != NULL) ||
         (getenv("CBTF_DEBUG_MRNET_FRONTEND") != NULL));
    bool is_filter_debug_enabled =
        ((getenv("CBTF_DEBUG_MRNET") != NULL) ||
         (getenv("CBTF_DEBUG_MRNET_FILTER") != NULL));
    bool is_tracing_debug_enabled = 
        (getenv("CBTF_DEBUG_MRNET_TRACING") != NULL);

    // Enable tracing in the MRNet library (if appropriate)
    if (is_tracing_debug_enabled)
    {
        MRN::set_OutputLevel(MRN::MAX_OUTPUT_LEVEL);
    }

    // Resolve the filter path
    boost::filesystem::path filter_path =
        resolvePath(kLibraryFileType, FILTER_FILE);
    if (filter_path.empty())
    {
        raise<std::runtime_error>(
            "The path of the MRNet filter library (%1%) could not be resolved.",
            FILTER_FILE
            );
    }
    
    // Load the upstream filter
    int upstream_filter = dm_network->load_FilterFunc(
        filter_path.string().c_str(), "libcbtf_mrnet_upstream_filter"
        );
    if (upstream_filter == -1)
    {
        raise<std::runtime_error>(
            "Unable to load the MRNet filter library (%1%) or to locate the "
            "filter function libcbtf_mrnet_upstream_filter().", filter_path
            );
    }

    // Load the downstream filter
    int downstream_filter = dm_network->load_FilterFunc(
        filter_path.string().c_str(), "libcbtf_mrnet_downstream_filter"
        );
    if (downstream_filter == -1)
    {
        raise<std::runtime_error>(
            "Unable to load the MRNet filter library (%1%) or to locate the "
            "filter function libcbtf_mrnet_downstream_filter().", filter_path
            );
    }

    // Establish the stream used to pass data within this network
    dm_stream = dm_network->new_Stream(
        dm_network->get_BroadcastCommunicator(),
        upstream_filter, filter_mode, downstream_filter
        );
    if ((dm_stream == NULL) ||
        (dm_stream->send(MessageTags::EstablishUpstream, 0) != 0) ||
        (dm_stream->flush() != 0))
    {
        raise<std::runtime_error>("Unable to connect to the backends.");
    }

    // Configure the upstream and downstream filters
    if (dm_stream->set_FilterParameters(
            MRN::FILTER_UPSTREAM_TRANS, "%ud %d %d", 
            dm_stream->get_Id(),
            is_filter_debug_enabled ? 1 : 0,
            is_tracing_debug_enabled ? 1 : 0) != 0)
    {
        raise<std::runtime_error>("Unable to configure the upstream filter.");
    }
    if (dm_stream->set_FilterParameters(
            MRN::FILTER_DOWNSTREAM_TRANS, "%ud %d %d", 
            dm_stream->get_Id(),
            is_filter_debug_enabled ? 1 : 0,
            is_tracing_debug_enabled ? 1 : 0) != 0)
    {
        raise<std::runtime_error>("Unable to configure the downstream filter.");
    }

    // Start a thread executing the frontend's message pump
    dm_message_pump_thread = boost::thread(
        boost::bind(&Frontend::doMessagePump, this)
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Frontend::~Frontend()
{
    // Interrupt the thread executing this backend's message pump
    dm_message_pump_thread.interrupt();

    // Wait for the thread to actually exit
    dm_message_pump_thread.join();
    
    // Instruct the backends to shutdown
    sendToBackends(
        MRN::PacketPtr(new MRN::Packet(0, MessageTags::RequestShutdown, ""))
        );
    
    // Iterate until all backends are ready to shutdown
    int remaining_endpoints =
        dm_network->get_BroadcastCommunicator()->get_EndPoints().size();
    while (remaining_endpoints > 0)
    {
        // Receive the next available message
        int tag = -1;
        MRN::PacketPtr packet;
        int retval = dm_stream->recv(&tag, packet, false);
        if (retval == 0)
        {
            continue;
        }
        else if ((retval == -1) || (packet == NULL))
        {
            raise<std::runtime_error>(
                "MRNet failed to receive the next message."
                );
        }

        if (dm_is_debug_enabled)
        {
            bool handling = (tag == MessageTags::AcknowledgeShutdown);
            std::cout << "[FE " << getpid() << "] "
                      << "Received and "
                      << (handling ? "handling" : "ignoring")
                      << " " << tag << "." << std::endl;
        }
                            
        // Was this backend reporting its readiness to shutdown?
        if (tag == MessageTags::AcknowledgeShutdown)
        {
            --remaining_endpoints;
        }
    }

    // Destroy the stream used to pass data within this network
    delete dm_stream;
}



//------------------------------------------------------------------------------
// Set the message handler for the specified message tag.
//------------------------------------------------------------------------------
void Frontend::setMessageHandler(const int& tag, const MessageHandler& handler)
{
    boost::unique_lock<boost::shared_mutex> guard_message_handlers(
        dm_message_handlers_mutex
        );
    dm_message_handlers.insert(std::make_pair(tag, handler));    
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Frontend::sendToBackends(const MRN::PacketPtr& packet)
{
    if (dm_is_debug_enabled)
    {
        std::cout << "[FE " << getpid() << "] "
                  << "Sending " << packet->get_Tag() << "." << std::endl;
    }

    // Insure the packet containing the message has the correct stream ID
    if (dm_stream == NULL)
    {
        raise<std::runtime_error>("The MRNet stream hasn't been created yet.");
    }
    packet->set_StreamId(dm_stream->get_Id());

    // Send the message
    int retval = dm_stream->send(const_cast<MRN::PacketPtr&>(packet));
    if (retval != 0)
    {
        raise<std::runtime_error>("Failed to send the specified message.");
    }
    retval = dm_stream->flush();
    if (retval != 0)
    {
        raise<std::runtime_error>("Failed to send the specified message.");
    }
}



//------------------------------------------------------------------------------
// Executing the pump within a separate thread insures the incoming messages
// are handled in a timely manner. The pump simply receives incoming messages
// and then dispatches them to the appropriate handler until it is instructed
// to exit.
//------------------------------------------------------------------------------
void Frontend::doMessagePump()
{
    // Get the file descriptor for MRNet data event notification
    int mrnet_fd = dm_network->get_EventNotificationFd(MRN::Event::DATA_EVENT);
    
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
                    MRN::PacketPtr packet;
                    int retval = dm_stream->recv(&tag, packet, false);
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
                            guard_message_handlers(dm_message_handlers_mutex);
                        std::map<int, MessageHandler>::const_iterator i =
                            dm_message_handlers.find(tag);
                        if (i != dm_message_handlers.end())
                        {
                            handler = i->second;
                        }
                    }

                    if (dm_is_debug_enabled)
                    {
                        std::cout << "[FE " << getpid() << "] "
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
                            std::cout << "[FE " << getpid() << "] EXCEPTION: "
                                      << error.what() << std::endl;
                        }
                    }
                    
                    // Reset MRNet data event notification
                    dm_network->clear_EventNotificationFd(
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
