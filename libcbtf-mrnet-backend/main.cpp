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

/** @file Main entry point for the CBTF MRNet backend. */

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <KrellInstitute/CBTF/Impl/XercesExts.hpp>
#include <mrnet/MRNet.h>
#include <unistd.h>
#include <xercesc/dom/DOM.hpp>

#include "Backend.hpp"
#include "IncomingStreamMediator.hpp"
#include "LocalComponentNetwork.hpp"
#include "MessageHandler.hpp"
#include "MessageTags.hpp"
#include "NamedStreams.hpp"

using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Mutual exclusion lock for implementing the exit signal. */
    boost::mutex exit_signal_mutex;
    
    /** Condition variable for implementing the exit signal. */
    boost::condition_variable exit_signal_condition;

    /**
     * Type of associative container used to map between the unique identifiers
     * for distributed component networks and their local component networks.
     */
    typedef std::map<
        int, boost::shared_ptr<LocalComponentNetwork>
        > NetworkMap;

    /** Distributed component networks on this backend. */
    NetworkMap networks;

    /**
     * Bind the specified incoming downstream mediator by attaching its
     * handler() method directly to the correct backend message handler.
     *
     * @param mediator    Incoming downstream mediator to be bound.
     */
    void bindIncomingDownstream(
        boost::shared_ptr<IncomingStreamMediator>& mediator
        )
    {
        Backend::setMessageHandler(
            mediator->tag(),
            boost::bind(&IncomingStreamMediator::handler, mediator, _1)
            );
    }
    
    /**
     * Handler for the RequestShutdown message. Signals the backend to exit.
     *
     * @param packet    Packet containing the received message.
     */    
    void requestShutdown(const MRN::PacketPtr& packet)
    {
        boost::mutex::scoped_lock guard_exit_signal(exit_signal_mutex);
        exit_signal_condition.notify_all();
    }
    
    /**
     * Handler for the SpecifyNamedStreams message. Decodes the named streams
     * and begins the construction of this backend's local component network
     * for the specified distributed component network.
     *
     * @param packet    Packet containing the received message.
     */
    void specifyNamedStreams(const MRN::PacketPtr& packet)
    {
        boost::shared_ptr<NamedStreams> named_streams(new NamedStreams(packet));
        
        if (networks.find(named_streams->uid()) != networks.end())
        {
            std::cout << "[BE " << getpid() << "] WARNING: "
                      << "Received SpecifyNamedStreams for distributed "
                      << "component network UID " << named_streams->uid() 
                      << " more than once." << std::endl;
            return;
        }

        if (Backend::isDebugEnabled())
        {
            std::cout << "[BE " << getpid() << "] "
                      << "Received SpecifyNamedStreams for distributed "
                      << "component network UID " << named_streams->uid()
                      << "." << std::endl;        
            std::cout << std::endl << *named_streams << std::endl << std::endl;
        }
        
        boost::shared_ptr<LocalComponentNetwork> network(
            new LocalComponentNetwork()
            );
        
        network->initializeStepOne(named_streams);
        
        networks.insert(std::make_pair(named_streams->uid(), network));
    }
    
    /**
     * Handler for the SpecifyBackend message. Decodes the XML specification
     * for the backends and completes the construction of this backend's local
     * component network for the specified distributed component network.
     *
     * @param packet    Packet containing the received message.
     */
    void specifyBackend(const MRN::PacketPtr& packet)
    {
        int uid = -1;
        char* buffer = NULL;
        
        try
        {
            packet->unpack("%d %s", &uid, &buffer);
        }
        catch (...)
        {
            if (buffer != NULL)
            {
                free(buffer);
            }
            throw;            
        }
        std::string xml(buffer);
        free(buffer);

        boost::shared_ptr<xercesc::DOMDocument> document = 
            xercesc::loadFromString(xml);
                
        NetworkMap::iterator i = networks.find(uid);
        
        if (i == networks.end())
        {
            std::cout << "[BE " << getpid() << "] WARNING: "
                      << "Received SpecifyBackend for distributed "
                      << "component network UID " << uid << " more than once."
                      << std::endl;
            return;
        }

        if (Backend::isDebugEnabled())
        {
            std::cout << "[BE " << getpid() << "] "
                      << "Received SpecifyBackend for distributed "
                      << "component network UID " << uid << "." << std::endl;
            std::cout << std::endl << xml << std::endl << std::endl;
        }
        
        i->second->initializeStepTwo(
            document, document.get()->getDocumentElement()
            );
        
        i->second->initializeStepThree(
            LocalComponentNetwork::IncomingBinder(), // No Incoming Upstreams
            boost::bind(&bindIncomingDownstream, _1),
            boost::bind(&Backend::sendToFrontend, _1),
            MessageHandler() // No Outgoing Downstreams
            );        
    }

    /**
     * Handler for the DestroyNetwork message. Initiate the destruction of
     * this backend's local component network for the specified distributed
     * component network.
     *
     * @param packet    Packet containing the received message.
     */
    void destroyNetwork(const MRN::PacketPtr& packet)
    {
        int uid = -1;
        
        packet->unpack("%d", &uid);

        if (networks.find(uid) == networks.end())
        {
            std::cout << "[BE " << getpid() << "] WARNING: "
                      << "Received DestroyNetwork for non-existent distributed "
                      << "component network UID " << uid << "." << std::endl;
            return;
        }

        if (Backend::isDebugEnabled())
        {
            std::cout << "[BE " << getpid() << "] "
                      << "Received DestroyNetwork for distributed "
                      << "component network UID " << uid << "." << std::endl;
        }

        networks.erase(uid);
    }
    
} // namespace <anonymous>



/**
 * Main entry point. Starts the MRNet backend message pump after setting
 * message handlers, waits until the backend is instructed to exit, then
 * stops the MRNet backend message pump and exits.
 *
 * @param argc    Number of command-line arguments.
 * @param argv    Array of command-line arguments.
 */
int main(int argc, char* argv[])
{
    // Redirect the stdout and stderr streams to a log file
    boost::filesystem::path log_file_path =
        boost::filesystem::path("/tmp") / boost::filesystem::path(
            boost::str(boost::format("%1%.%2%") % argv[0] % getpid())
            ).filename();
    FILE* log_file = fopen(log_file_path.string().c_str(), "w");
    if (log_file != NULL)
    {
        dup2(fileno(log_file), STDOUT_FILENO);
        dup2(fileno(log_file), STDERR_FILENO);        
    }
    
    // Display a startup message
    std::cout << "Started CBTF MRNet Backend: "
              << boost::posix_time::second_clock::local_time() << std::endl;
    std::cout << std::endl;
    extern char** environ;
    for (int i = 0; environ[i] != NULL; ++i)
    {
        std::cout << environ[i] << std::endl;
    }
    std::cout << std::endl;
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }
    std::cout << std::endl;
    std::cout.flush();
    
    // Set message handlers for this backend
    Backend::setMessageHandler(
        MessageTags::RequestShutdown, requestShutdown
        );
    Backend::setMessageHandler(
        MessageTags::SpecifyNamedStreams, specifyNamedStreams
        );
    Backend::setMessageHandler(
        MessageTags::SpecifyBackend, specifyBackend
        );
    Backend::setMessageHandler(
        MessageTags::DestroyNetwork, destroyNetwork
        );

    // Catch and log any exceptions encountered during execution
    try
    {
        // Start this backend's message pump
        Backend::startMessagePump(argc, argv);

        // Wait until this backend is instructed to exit
        boost::mutex::scoped_lock guard_exit_signal(exit_signal_mutex);
        exit_signal_condition.wait(guard_exit_signal);
    }
    catch (const std::exception& error)
    {
        std::cout << "[BE " << getpid() << "] EXCEPTION: "
                  << error.what() << std::endl;
    }

    // Let the frontend know that this backend is shutting down
    Backend::sendToFrontend(
        MRN::PacketPtr(new MRN::Packet(0, MessageTags::AcknowledgeShutdown, ""))
        );
    
    // Stop this backend's message pump
    Backend::stopMessagePump();
    
    // Remove message handlers for this backend
    Backend::setMessageHandler(
        MessageTags::RequestShutdown, MessageHandler()
        );
    Backend::setMessageHandler(
        MessageTags::SpecifyNamedStreams, MessageHandler()
        );
    Backend::setMessageHandler(
        MessageTags::SpecifyBackend, MessageHandler()
        );
    Backend::setMessageHandler(
        MessageTags::DestroyNetwork, MessageHandler()
        );

    // Display a shutdown message
    std::cout << std::endl;
    std::cout << "Stopped CBTF MRNet Backend: "
              << boost::posix_time::second_clock::local_time() << std::endl;
    std::cout.flush();

    // Close the log file containing the redirected stdout and stderr streams
    if (log_file != NULL)
    {
        fclose(log_file);
    }

    // Indicate success to the shell
    return 0;
}
