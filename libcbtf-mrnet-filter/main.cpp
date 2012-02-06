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

/** @file Main entry points for the CBTF MRNet filter. */

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/weak_ptr.hpp>
#include <cstdlib>
#include <iostream>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <map>
#include <mrnet/MRNet.h>
#include <set>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <xercesc/dom/DOM.hpp>

#include "LocalComponentNetwork.hpp"
#include "MessageTags.hpp"
#include "NamedStreams.hpp"
#include "XercesExts.hpp"

using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** ID for the MRNet stream used to pass data within this network. */
    unsigned int mrnet_stream_id = 0;

    /** Flag indicating if debugging is enabled for this filter. */
    bool is_filter_debug_enabled = false;

    /** Prefix to apply to all debugging statements. */
    std::string debug_prefix;

    /**
     * Type of associative container used to map between the unique identifiers
     * for distributed component networks and their local component networks.
     */
    typedef std::map<
        int, boost::shared_ptr<LocalComponentNetwork>
        > NetworkMap;

    /** Distributed component networks on this filter. */
    NetworkMap networks;

    /**
     * Type of associative container used to map between the MRNet message tags
     * for named streams and their incoming stream mediators.
     */
    typedef std::map<
        int, boost::shared_ptr<IncomingStreamMediator>
        > MediatorMap;

    /** Named incoming upstreams on this filter. */
    MediatorMap incoming_upstream_mediators;

    /** Named incoming downstreams on this filter. */
    MediatorMap incoming_downstream_mediators;

    /** Mutual exclusion lock for the packet queues. */
    boost::mutex packet_queues_mutex;

    /** Queue of packets to be delivered on the upstream. */
    std::vector<MRN::PacketPtr> upstream_packet_queue;

    /** Queue of packets to be delivered on the downstream. */
    std::vector<MRN::PacketPtr> downstream_packet_queue;

    /**
     * Bind the specified incoming upstream mediator by adding it to the map
     * of incoming upstream mediators for later use when receiving packets.
     *
     * @param mediator    Incoming upstream mediator to be bound.
     */
    void bindIncomingUpstream(
        boost::shared_ptr<IncomingStreamMediator>& mediator
        )
    {
        incoming_upstream_mediators.insert(
            std::make_pair(mediator->tag(), mediator)
            );
    }
    
    /**
     * Bind the specified incoming downstream mediator by adding it to the map
     * of incoming downstream mediators for later use when receiving packets.
     *
     * @param mediator    Incoming downstream mediator to be bound.
     */
    void bindIncomingDownstream(
        boost::shared_ptr<IncomingStreamMediator>& mediator
        )
    {
        incoming_downstream_mediators.insert(
            std::make_pair(mediator->tag(), mediator)
            );
    }

    /**
     * Handler for the filter configuration parameters message. Decodes the
     * parameters and configures debugging settings as appropriate.
     *
     * @param packet           Packet containing the received message.
     * @param topology_info    Location of this filter instance.
     */
    void configurationParameters(const MRN::PacketPtr& packet,
                                 const MRN::TopologyLocalInfo& topology_info)
    {
        int filter_debug_enabled = -1, tracing_debug_enabled = -1;

        if (packet != MRN::Packet::NullPacket)
        {
            packet->unpack("%ud %d %d",
                &mrnet_stream_id, &filter_debug_enabled, &tracing_debug_enabled
                );
        }
        
        is_filter_debug_enabled = (filter_debug_enabled == 1) ? true : false;
        
        if (tracing_debug_enabled == 1)
        {
            MRN::set_OutputLevel(MRN::MAX_OUTPUT_LEVEL); 
        }

        std::ostringstream stream;
        stream << "[FI/";
        if (topology_info.get_Network()->is_LocalNodeFrontEnd())
        {
            stream << "FE";
        }
        else if (topology_info.get_Network()->is_LocalNodeBackEnd())
        {
            stream << "BE";
        }
        else
        {
            stream << "CP";
        }
        stream << " " << getpid() << "] ";

        debug_prefix = stream.str();

        TheTopologyInfo.Rank = topology_info.get_Rank();
        TheTopologyInfo.NumChildren = topology_info.get_NumChildren();
        TheTopologyInfo.NumSiblings = topology_info.get_NumSiblings();
        TheTopologyInfo.NumDescendants = topology_info.get_NumDescendants();
        TheTopologyInfo.NumLeafDescendants =
            topology_info.get_NumLeafDescendants();
        TheTopologyInfo.RootDistance = topology_info.get_RootDistance();
        TheTopologyInfo.MaxLeafDistance = topology_info.get_MaxLeafDistance();
    }
    
    /**
     * Send a message to the frontend.
     *
     * @param packet    Packet containing the message to be sent.
     */
    void sendToFrontend(const MRN::PacketPtr& packet)
    {
        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix << "Sending (upward) "  
                      << packet->get_Tag() << "." << std::endl;
        }

        packet->set_StreamId(mrnet_stream_id);

        boost::mutex::scoped_lock guard_packet_queues(packet_queues_mutex);
        upstream_packet_queue.push_back(packet);
    }
    
    /**
     * Send a message to all of the descendant backends.
     *
     * @param packet    Packet containing the message to be sent.
     */
    void sendToBackends(const MRN::PacketPtr& packet)
    {
        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix << "Sending (downward) " 
                      << packet->get_Tag() << "." << std::endl;
        }

        packet->set_StreamId(mrnet_stream_id);

        boost::mutex::scoped_lock guard_packet_queues(packet_queues_mutex);
        downstream_packet_queue.push_back(packet);
    }

    /**
     * Handler for the SpecifyNamedStreams message. Decodes the named streams
     * and begins the construction of this filter's local component network
     * for the specified distributed component network.
     *
     * @param packet    Packet containing the received message.
     */
    void specifyNamedStreams(const MRN::PacketPtr& packet)
    {
        boost::shared_ptr<NamedStreams> named_streams(new NamedStreams(packet));
        
        if (networks.find(named_streams->uid()) != networks.end())
        {
            std::cout << debug_prefix << "WARNING: "
                      << "Received SpecifyNamedStreams for distributed "
                      << "component network UID " << named_streams->uid()
                      << " more than once." << std::endl;
            return;
        }

        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix
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

    /** Insert the value of the specified node into the given integer set. */
    void insertInteger(const xercesc::DOMNode* node, std::set<int>& integers)
    {
        integers.insert(
            boost::lexical_cast<int>(xercesc::selectValue(node, "."))
            );
    }

    /** Compare the given offset against the ones in the specified node. */
    void checkOffset(const xercesc::DOMNode* node, int offset,
                     boost::tribool& use_filter)
    {
        std::set<int> offsets;

        xercesc::selectNodes(
            node, "./Offset",
            boost::bind(&insertInteger, _1, boost::ref(offsets))
            );
        
        std::string value = xercesc::selectValue(node, "./@offset");
        
        if (!value.empty())
        {
            offsets.insert(boost::lexical_cast<int>(value));
        }
        
        use_filter = (offsets.find(offset) != offsets.end());
    }

    /**
     * Handler for the SpecifyFilter message. Decodes the XML specification
     * for the filters and completes the construction of this filter's local
     * component network for the specified distributed component network.
     *
     * @param packet           Packet containing the received message.
     * @param topology_info    Location of this filter instance.
     */
    void specifyFilter(const MRN::PacketPtr& packet,
                       const MRN::TopologyLocalInfo& topology_info)
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
            std::cout << debug_prefix << "WARNING: "
                      << "Received SpecifyFilter for distributed "
                      << "component network UID " << uid
                      << " before receiving SpecifyNamedStreams."
                      << std::endl;
            return;
        }
        
        boost::tribool use_filter = boost::indeterminate;

        xercesc::selectNodes(
            document.get()->getDocumentElement(),
            "./Depth/LeafRelative",
            boost::bind(&checkOffset, _1, topology_info.get_MaxLeafDistance(),
                        boost::ref(use_filter))
            );
        
        xercesc::selectNodes(
            document.get()->getDocumentElement(),
            "./Depth/RootRelative",
            boost::bind(&checkOffset, _1, topology_info.get_RootDistance(),
                        boost::ref(use_filter))
            );

        if (boost::indeterminate(use_filter))
        {
            use_filter = !i->second->network();
        }
        
        if (!use_filter)
        {
            if (is_filter_debug_enabled)
            {
                std::cout << debug_prefix
                          << "Received, and ignored, SpecifyFilter for "
                          << "distributed component network UID " << uid 
                          << "." << std::endl;
            }
            return;
        }
        
        if (i->second->network())
        {
            std::cout << debug_prefix << "WARNING: "
                      << "Received SpecifyFilter for distributed "
                      << "component network UID " << uid << " more than once."
                      << std::endl;
            return;
        }
        
        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix
                      << "Received SpecifyFilter for distributed "
                      << "component network UID " << uid << "." << std::endl;
            std::cout << std::endl << xml << std::endl << std::endl;
        }

        i->second->initializeStepTwo(
            document, document.get()->getDocumentElement()
            );
        
        i->second->initializeStepThree(
            boost::bind(&bindIncomingUpstream, _1),
            boost::bind(&bindIncomingDownstream, _1),
            boost::bind(&sendToFrontend, _1),
            boost::bind(&sendToBackends, _1)
            );
    }

    /**
     * Handler for the DestroyNetwork message. Initiate the destruction of
     * this filter's local component network for the specified distributed
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
            std::cout << debug_prefix << "WARNING: "
                      << "Received DestroyNetwork for non-existent distributed "
                      << "component network UID " << uid << "." << std::endl;
            return;
        }

        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix
                      << "Received DestroyNetwork for distributed "
                      << "component network UID " << uid << "." << std::endl;
        }

        networks.erase(uid);
    }
    
} // namespace <anonymous>



/** Format of data operated upon by the upstream filter function. */
extern "C" const char* const libcbtf_mrnet_upstream_filter_format_string = "";

/** Format of data operated upon by the downstream filter function. */
extern "C" const char* const libcbtf_mrnet_downstream_filter_format_string = "";



/**
 * Upstream filter function. Mediate all packets connected to one of the local
 * component networks on this filter, forwarding all the rest along with all of
 * the packets currently waiting in the outgoing queues.
 *
 * @param packets_in_upstream       Packets arriving along the upstream.
 * @param packets_out_upstream      Packets outgoing along the upstream.
 * @param packets_out_downstream    Packets outgoing along the downstream.
 * @param filter_state              State specific to this filter instance.
 * @param config_params             Packet containing the current configuration
 *                                  settings for this filter instance.
 * @param topology_info             Location of this filter instance.
 */
extern "C" void libcbtf_mrnet_upstream_filter(
    std::vector<MRN::PacketPtr>& packets_in_upstream,
    std::vector<MRN::PacketPtr>& packets_out_upstream,
    std::vector<MRN::PacketPtr>& packets_out_downstream,
    void** filter_state,
    MRN::PacketPtr& config_params,
    const MRN::TopologyLocalInfo& topology_info
    )
{
    configurationParameters(config_params, topology_info);

    std::vector<MRN::PacketPtr> packets_to_forward;

    for (std::vector<MRN::PacketPtr>::const_iterator
             i = packets_in_upstream.begin();
         i != packets_in_upstream.end();
         ++i)
    {
        bool forward_packet = true;

        try
        {
            MediatorMap::const_iterator j = 
                incoming_upstream_mediators.find((*i)->get_Tag());
            
            if (j != incoming_upstream_mediators.end())
            {
                forward_packet = false;
                
                if (is_filter_debug_enabled)
                {
                    std::cout << debug_prefix
                              << "Received (upward) and handling "
                              << (*i)->get_Tag() << "." << std::endl;
                }

                j->second->handler(*i);
            }
        }
        catch (const std::exception& error)
        {
            forward_packet = true;
            std::cout << debug_prefix << "EXCEPTION: "
                      << error.what() << std::endl;
        }
        
        if (forward_packet)
        {
            if (is_filter_debug_enabled)
            {
                std::cout << debug_prefix << "Received (upward) and forwarding "
                          << (*i)->get_Tag() << "." << std::endl;
            }
            
            packets_to_forward.push_back(*i);
        }
    }

    boost::mutex::scoped_lock guard_packet_queues(packet_queues_mutex);

    upstream_packet_queue.swap(packets_out_upstream);
    upstream_packet_queue.clear();

    downstream_packet_queue.swap(packets_out_downstream);
    downstream_packet_queue.clear();

    packets_out_upstream.insert(
        packets_out_upstream.end(),
        packets_to_forward.begin(), packets_to_forward.end()
        );
}



/**
 * Downstream filter function. Mediate all packets connected to one of the local
 * component networks on this filter, forwarding all the rest along with all of
 * the packets currently waiting in the outgoing queues. Also passes any control
 * packets to the appropriate handler.
 *
 * @param packets_in_downstream     Packets arriving along the downstream.
 * @param packets_out_downstream    Packets outgoing along the downstream.
 * @param packets_out_upstream      Packets outgoing along the upstream.
 * @param filter_state              State specific to this filter instance.
 * @param config_params             Packet containing the current configuration
 *                                  settings for this filter instance.
 * @param topology_info             Location of this filter instance.
 */
extern "C" void libcbtf_mrnet_downstream_filter(
    std::vector<MRN::PacketPtr>& packets_in_downstream,
    std::vector<MRN::PacketPtr>& packets_out_downstream,
    std::vector<MRN::PacketPtr>& packets_out_upstream,
    void** filter_state,
    MRN::PacketPtr& config_params,
    const MRN::TopologyLocalInfo& topology_info
    )
{
    configurationParameters(config_params, topology_info);

    std::vector<MRN::PacketPtr> packets_to_forward;

    for (std::vector<MRN::PacketPtr>::const_iterator
             i = packets_in_downstream.begin();
         i != packets_in_downstream.end();
         ++i)
    {
        bool forward_packet = true;

        try
        {
            if ((*i)->get_Tag() == MessageTags::SpecifyNamedStreams)
            {
                specifyNamedStreams(*i);
            }
            else if ((*i)->get_Tag() == MessageTags::SpecifyFilter)
            {
                specifyFilter(*i, topology_info);
            }
            else if ((*i)->get_Tag() == MessageTags::DestroyNetwork)
            {
                destroyNetwork(*i);
            }
            else
            {
                MediatorMap::const_iterator j = 
                    incoming_downstream_mediators.find((*i)->get_Tag());
                
                if (j != incoming_downstream_mediators.end())
                {
                    forward_packet = false;

                    if (is_filter_debug_enabled)
                    {
                        std::cout << debug_prefix
                                  << "Received (downward) and handling "
                                  << (*i)->get_Tag() << "." << std::endl;
                    }
                    
                    j->second->handler(*i);
                }
            }
        }
        catch (const std::exception& error)
        {
            forward_packet = true;
            std::cout << debug_prefix << "EXCEPTION: "
                      << error.what() << std::endl;
        }
        
        if (forward_packet)
        {
            if (is_filter_debug_enabled)
            {
                std::cout << debug_prefix
                          << "Received (downward) and forwarding "
                          << (*i)->get_Tag() << "." << std::endl;
            }

            packets_to_forward.push_back(*i);
        }
    }

    boost::mutex::scoped_lock guard_packet_queues(packet_queues_mutex);

    upstream_packet_queue.swap(packets_out_upstream);
    upstream_packet_queue.clear();

    downstream_packet_queue.swap(packets_out_downstream);
    downstream_packet_queue.clear();

    packets_out_downstream.insert(
        packets_out_downstream.end(),
        packets_to_forward.begin(), packets_to_forward.end()
        );
}
