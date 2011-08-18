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

/** @file Definition of the LocalComponentNetwork class. */

#include <boost/bind.hpp>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <stdexcept>

#include "LocalComponentNetwork.hpp"
#include "Raise.hpp"
#include "XercesExts.hpp"
#include "XML.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
LocalComponentNetwork::LocalComponentNetwork() :
    dm_named_streams(),
    dm_document(),
    dm_root(),
    dm_network(),
    dm_mediators()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void LocalComponentNetwork::initializeStepOne(
    const boost::shared_ptr<NamedStreams>& named_streams
    )
{
    if (dm_named_streams)
    {
        raise<std::logic_error>(
            "The method initializeStepOne() can't be invoked more than once."
            );
    }
    
    dm_named_streams = named_streams;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void LocalComponentNetwork::initializeStepTwo(
    const boost::shared_ptr<xercesc::DOMDocument>& document,
    const xercesc::DOMNode* root
    )
{
    if (dm_document || (dm_root != NULL) || dm_network)
    {
        raise<std::logic_error>(
            "The method initializeStepTwo() can't be invoked more than once."
            );
    }

    dm_document = document;
    dm_root = root;

    xercesc::selectNodes(
        dm_root, "./Network",
        boost::bind(&LocalComponentNetwork::parseNetwork, this, _1)
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void LocalComponentNetwork::initializeStepThree(
    const IncomingBinder& incoming_upstream_binder,
    const IncomingBinder& incoming_downstream_binder,
    const MessageHandler& outgoing_upstream_handler,
    const MessageHandler& outgoing_downstream_handler
    )
{
    if (!dm_mediators.empty())
    {
        raise<std::logic_error>(
            "The method initializeStepThree() can't be invoked more than once."
            );
    }

    if (!dm_named_streams || !dm_document || (dm_root == NULL) || !dm_network)
    {
        raise<std::logic_error>(
            "The method initializeStepThree() can't be invoked before both "
            "the initializeStepOne() and the initializeStepTwo() methods "
            "have been invoked."
            );
    }
    
    xercesc::selectNodes(
        dm_root, "./IncomingUpstream",
        boost::bind(&LocalComponentNetwork::parseIncomingStream, this,
                    _1, incoming_upstream_binder)
        );
    xercesc::selectNodes(
        dm_root, "./IncomingDownstream",
        boost::bind(&LocalComponentNetwork::parseIncomingStream, this,
                    _1, incoming_downstream_binder)
        );
    xercesc::selectNodes(
        dm_root, "./OutgoingUpstream",
        boost::bind(&LocalComponentNetwork::parseOutgoingStream, this,
                    _1, outgoing_upstream_handler)
        );
    xercesc::selectNodes(
        dm_root, "./OutgoingDownstream",
        boost::bind(&LocalComponentNetwork::parseOutgoingStream, this,
                    _1, outgoing_downstream_handler)
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const boost::shared_ptr<NamedStreams>& 
LocalComponentNetwork::named_streams() const
{
    return dm_named_streams;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Component::Instance LocalComponentNetwork::network()
{
    return dm_network;
}



//------------------------------------------------------------------------------
// Parse the specified <Incoming[Downstream|Upstream]> XML node, create an
// appropriate incoming stream mediator, and bind it.
//------------------------------------------------------------------------------
void LocalComponentNetwork::parseIncomingStream(const xercesc::DOMNode* node,
                                                const IncomingBinder& binder)
{
    Component::Instance mediator_instance = IncomingStreamMediator::create(
        node, dm_network, *dm_named_streams
        );

    dm_mediators.push_back(mediator_instance);

    boost::shared_ptr<IncomingStreamMediator> mediator =
        boost::reinterpret_pointer_cast<IncomingStreamMediator>(
            mediator_instance
            );

    binder(mediator);
}



//------------------------------------------------------------------------------
// Instantiate the component network described by this <Network> XML node.
//------------------------------------------------------------------------------
void LocalComponentNetwork::parseNetwork(const xercesc::DOMNode* node)
{
    dm_network = instantiateXML(dm_document, node);
}



//------------------------------------------------------------------------------
// Parse the specified <Outgoing[Downstream|Upstream]> XML node, create an
// appropriate outgoing stream mediator, and bind it.
//------------------------------------------------------------------------------
void LocalComponentNetwork::parseOutgoingStream(const xercesc::DOMNode* node,
                                                const MessageHandler& handler)
{
    Component::Instance mediator_instance = OutgoingStreamMediator::create(
        node, dm_network, *dm_named_streams, handler
        );
    
    dm_mediators.push_back(mediator_instance);
}
