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

/** @file Main entry points for the CBTF MRNet filter. */

#include <boost/assert.hpp>
#include <boost/bind.hpp>
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
#include "MessageHandlers.hpp"
#include "MessageTags.hpp"
#include "NamedStreams.hpp"
#include "ParseDepth.hpp"
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

    /** Incoming upstream message handlers for this filter. */
    MessageHandlers incoming_upstream_message_handlers;

    /** Incoming downstream message handlers for this filter. */
    MessageHandlers incoming_downstream_message_handlers;

    /** Mutual exclusion lock for the packet queues. */
    boost::mutex packet_queues_mutex;

    /** Queue of packets to be delivered on the upstream. */
    std::vector<MRN::PacketPtr> upstream_packet_queue;

    /** Queue of packets to be delivered on the downstream. */
    std::vector<MRN::PacketPtr> downstream_packet_queue;

    /**
     * Bind the specified incoming upstream mediator by adding its handler()
     * method to the filter's incoming upstream message handlers.
     *
     * @param uid         Unique identifier for the distribured component
     *                    network associated with this mediator.
     * @param mediator    Incoming upstream mediator to be bound.
     */
    void bindIncomingUpstream(
        const int& uid,
        const boost::shared_ptr<IncomingStreamMediator>& mediator
        )
    {
        incoming_upstream_message_handlers.add(
            uid, mediator->tag(),
            boost::bind(&IncomingStreamMediator::handler, mediator, _1)
            );
    }
    
    /**
     * Bind the specified incoming downstream mediator by adding its handler()
     * method to the filter's incoming downstream message handlers.
     *
     * @param uid         Unique identifier for the distribured component
     *                    network associated with this mediator.
     * @param mediator    Incoming downstream mediator to be bound.
     */
    void bindIncomingDownstream(
        const int& uid,
        const boost::shared_ptr<IncomingStreamMediator>& mediator
        )
    {
        incoming_downstream_message_handlers.add(
            uid, mediator->tag(),
            boost::bind(&IncomingStreamMediator::handler, mediator, _1)
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

        if (is_filter_debug_enabled)
        {
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
        }

        TheTopologyInfo.IsFrontend = 
            topology_info.get_Network()->is_LocalNodeFrontEnd();
        TheTopologyInfo.IsBackend =
            topology_info.get_Network()->is_LocalNodeBackEnd();
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
     * Is this filter located on a leaf communication process (CP) within the
     * network?
     *
     * @param topology_info    Location of this filter instance.
     * @return                 Boolean "true" if this filter is located on a
     *                         leaf CP within the network, or "false" otherwise.
     */
    bool isOnLeafCP(const MRN::TopologyLocalInfo& topology_info)
    {
        const MRN::NetworkTopology* topology = topology_info.get_Topology();
        BOOST_ASSERT(topology != NULL);
        
        MRN::NetworkTopology::Node* node =
            topology->find_Node(topology_info.get_Rank());
        BOOST_ASSERT(node != NULL);
        
        const std::set<MRN::NetworkTopology::Node*>& children =
            node->get_Children();
        
        std::set<MRN::NetworkTopology::Node*> backends;
        topology->get_BackEndNodes(backends);

        if (backends.size() == 0)
        {
            if (children.size() == 0)
            {
                return true;
            }
        }
        else
        {
            for (std::set<MRN::NetworkTopology::Node*>::const_iterator
                     i = children.begin(); i != children.end(); ++i)
            {
                if (backends.find(*i) != backends.end())
                {
                    return true;
                }
            }
        }
        
        return false;
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

        /* 
          Replace this line with the 4 lines below to avert a gcc/g++ 5.1 compiler error
          bool selected = i->second->network();
        */
        bool selected = false;
        if (i->second->network()) {
           selected = true;
        }
        
        xercesc::selectNodes(
            document.get()->getDocumentElement(), "./Depth",
            boost::bind(&parseDepth, _1, isOnLeafCP(topology_info),
                        boost::ref(selected))
            );
        
        if (!selected)
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
            boost::bind(&bindIncomingUpstream, uid, _1),
            boost::bind(&bindIncomingDownstream, uid, _1),
            boost::bind(&sendToFrontend, _1),
            boost::bind(&sendToBackends, _1)
            );
    }

    /**
     * Handler for the DestroyNetwork message. Initiate the destruction of
     * this filter's local component network for the specified distributed
     * component network after removing its message handlers.
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

        incoming_upstream_message_handlers.remove(uid);
        incoming_downstream_message_handlers.remove(uid);
        networks.erase(uid);
    }
    
} // namespace <anonymous>



/** Format of data operated upon by the upstream filter function. */
extern "C" const char* const libcbtf_mrnet_upstream_filter_format_string = "";

/** Format of data operated upon by the downstream filter function. */
extern "C" const char* const libcbtf_mrnet_downstream_filter_format_string = "";

extern "C" const char* const libcbtf_mrnet_sync_waitforall_filter_format_string = "";


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

#if 0
    std::cout << "ENTERED libcbtf_mrnet_upstream_filter"
	<< " upstream in " << packets_in_upstream.size()
	<< " upstream out " << packets_out_upstream.size()
	<< " downstream out " << packets_out_downstream.size()
	<< std::endl;
#endif
    std::vector<MRN::PacketPtr> packets_to_forward;

    for (std::vector<MRN::PacketPtr>::const_iterator
             i = packets_in_upstream.begin();
         i != packets_in_upstream.end();
         ++i)
    {
        bool handled = false;

        try
        {
            handled = incoming_upstream_message_handlers((*i)->get_Tag(), *i);
        }
        catch (const std::exception& error)
        {
            std::cout << debug_prefix << "EXCEPTION: "
                      << error.what() << std::endl;
        }
        
        if (!handled)
        {
            packets_to_forward.push_back(*i);
        }
                
        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix
                      << "Received (upward) and "
                      << (handled ? "handled" : "forwarded")
                      << " " << (*i)->get_Tag() << "." << std::endl;
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
        bool handled = false;

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
                handled = incoming_downstream_message_handlers(
                    (*i)->get_Tag(), *i
                    );
            }
        }
        catch (const std::exception& error)
        {
            std::cout << debug_prefix << "EXCEPTION: "
                      << error.what() << std::endl;
        }
        
        if (!handled)
        {
            packets_to_forward.push_back(*i);
        }
                
        if (is_filter_debug_enabled)
        {
            std::cout << debug_prefix
                      << "Received (downward) and "
                      << (handled ? "handled" : "forwarded")
                      << " " << (*i)->get_Tag() << "." << std::endl;
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



typedef struct {
    std::map < MRN::Rank, std::vector< MRN::PacketPtr >* > packets_by_rank;
    std::set < MRN::Rank > ready_peers;
} wfa_state;

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
extern "C" void libcbtf_mrnet_sync_waitforall_filter(
    std::vector<MRN::PacketPtr>& packets_in_upstream,
    std::vector<MRN::PacketPtr>& packets_out_upstream,
    std::vector<MRN::PacketPtr>& packets_out_downstream,
    void** filter_state,
    MRN::PacketPtr& config_params,
    const MRN::TopologyLocalInfo& topology_info
    )
{
    configurationParameters(config_params, topology_info);

    if (is_filter_debug_enabled) {
	for( unsigned i = 0; i < packets_in_upstream.size( ); i++ ) {
	    MRN::PacketPtr cur_packet = packets_in_upstream[i];
	    std::cerr << "CBTF SFILTER IN packet [" << i << "]"
	    << " streamID:" << cur_packet->get_StreamId()
	    << " tag:" << cur_packet->get_Tag()
	    << std::endl;
	}
    }


    std::map < MRN::Rank, std::vector< MRN::PacketPtr >* >::iterator map_iter, del_iter;
    wfa_state* state;
    
    MRN::Network* net = const_cast< MRN::Network* >( topology_info.get_Network() );

    int stream_id = packets_in_upstream[0]->get_StreamId();
    MRN::Stream* stream = net->get_Stream( stream_id );
    if( stream == NULL ) {
        if (is_filter_debug_enabled) {
            std::cerr << "ERROR: stream lookup " << stream_id <<  " failed" << std::endl;
	}
        return;
    }

    //1. Setup/Recover Filter State
    if( *filter_state == NULL ) {
        // allocate packet buffer map as appropriate
        if (is_filter_debug_enabled) {
            std::cerr << "No previous storage, allocating ..." << std::endl;
	}
        state = new wfa_state;
        *filter_state = state;
    }
    else{
        // get packet buffer map from filter state
        state = ( wfa_state * ) *filter_state;

        // check for failed nodes && closed Peers
        map_iter = state->packets_by_rank.begin();
        while ( map_iter != state->packets_by_rank.end() ) {

            MRN::Rank rank = (*map_iter).first;
            if( net->node_Failed(rank) ) {
                if (is_filter_debug_enabled) {
                    std::cerr << "Discarding packets from failed node[" << rank << "] ... " << std::endl;
		}
                del_iter = map_iter;
                map_iter++;

                // clear packet vector
                (*del_iter).second->clear();

                // erase map slot
                state->packets_by_rank.erase( del_iter );
                state->ready_peers.erase( rank );
            }
            else{
        	if (is_filter_debug_enabled) {
                    std::cerr << "Node[" << rank << "] failed? no " << std::endl;
		}
                map_iter++;
            }
        }
    }

    //2. Place input packets
    for( unsigned int i=0; i < packets_in_upstream.size(); i++ ) {

        MRN::Rank cur_inlet_rank = packets_in_upstream[i]->get_InletNodeRank();

        // special case for back-end synchronization; packets have unknown inlet
        if( cur_inlet_rank == MRN::UnknownRank ) {
            if( packets_in_upstream.size() == 1 ) {
                packets_out_upstream.push_back( packets_in_upstream[i] );
                if (is_filter_debug_enabled) {
		    std::cerr << "MRN::UnknownRank, packets have unknown inlet ... return" << std::endl;
		}
                return;
            }
        }

        if( net->node_Failed(cur_inlet_rank) ) {
            // drop packets from failed node
            if (is_filter_debug_enabled) {
	        std::cerr << "drop packets from failed node ... continue" << std::endl;
	    }
            continue;
        }

        // insert packet into map
        map_iter = state->packets_by_rank.find( cur_inlet_rank );

        // allocate new slot if necessary
        if( map_iter == state->packets_by_rank.end() ) {
            if (is_filter_debug_enabled) {
                std::cerr << "Allocating new map slot for node[" << cur_inlet_rank << "] ..." << std::endl;
	    }
            state->packets_by_rank[ cur_inlet_rank ] = new std::vector < MRN::PacketPtr >;
        }

        if (is_filter_debug_enabled) {
            std::cerr <<  "Placing packet[" << i << "] from node[" << cur_inlet_rank << "]" << std::endl;
	}
        state->packets_by_rank[ cur_inlet_rank ]->push_back( packets_in_upstream[i] );
        state->ready_peers.insert( cur_inlet_rank );
    }

    std::set< MRN::Rank > peers;
    stream->get_ChildRanks( peers );

    if (is_filter_debug_enabled) {
	std::cerr << "slots:" << state->packets_by_rank.size()
	      << " ready:" << state->ready_peers.size()
	      << " peers:" << peers.size()
	<< std::endl;
    }

    // check for a complete wave
    if( state->ready_peers.size() < peers.size() ) {
        // not all peers ready, so sync condition not met
	if (is_filter_debug_enabled) {
            std::cerr << "sync condition not met! return." << std::endl;
	}
        return;
    }

    // if we get here, SYNC CONDITION MET!
    if (is_filter_debug_enabled) {
        std::cerr << "All child nodes ready!" << std::endl;
    }

    //3. All nodes ready! Place output packets
    for( map_iter = state->packets_by_rank.begin();
         map_iter != state->packets_by_rank.end();
         map_iter++ ) {

        MRN::Rank r = map_iter->first;
        std::vector< MRN::PacketPtr >* pkt_vec = map_iter->second;
        if( pkt_vec->empty() ) {
            // list should only be empty if peer closed stream
	    if (is_filter_debug_enabled) {
                std::cerr << "Node[" << r << "]'s slot is empty" << std::endl;
	    }
            continue;
        }

	if (is_filter_debug_enabled) {
            std::cerr << "Popping packet from Node[" << r << "]" << std::endl;
	}

        // push head of list onto output vector
        packets_out_upstream.push_back( pkt_vec->front() );
        pkt_vec->erase( pkt_vec->begin() );
        
        // if list now empty, remove slot from ready list
        if( pkt_vec->empty() ) {
	    if (is_filter_debug_enabled) {
                std::cerr << "Removing Node[" << r << "] from ready list" << std::endl;
            }
            state->ready_peers.erase( r );
        }
    }

    if (is_filter_debug_enabled) {
	for( unsigned i = 0; i < packets_out_upstream.size( ); i++ ) {
	    MRN::PacketPtr cur_packet = packets_out_upstream[i];
	    std::cerr << "CBTF SFILTER OUT packet [" << i << "]"
	    << " streamID:" << cur_packet->get_StreamId()
	    << " tag:" << cur_packet->get_Tag()
	    << std::endl;
	}
    }
}

