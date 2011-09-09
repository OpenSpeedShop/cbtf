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

/** @file Definition of the OutgoingStreamMediator class. */

#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <string>
#include <typeinfo>

#include "OutgoingStreamMediator.hpp"
#include "StreamMediator.hpp"
#include "XercesExts.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
// Parse the specified <Outgoing[Downstream|Upstream]> XML node, create a new
// outgoing stream mediator, and establish the connection.
//------------------------------------------------------------------------------
Component::Instance OutgoingStreamMediator::create(
    const xercesc::DOMNode* node,
    Component::Instance network,
    const NamedStreams& named_streams,
    const MessageHandler& handler
    )
{
    const std::string name = xercesc::selectValue(node, "./Name");
    const std::string from_output = xercesc::selectValue(node, "./From/Output");
    
    boost::shared_ptr<OutgoingStreamMediator> mediator(
        new OutgoingStreamMediator(named_streams.tag(name), handler)
        );

    Component::Instance mediator_instance =
        boost::reinterpret_pointer_cast<Component>(mediator);

    mediator->converter() = connectWithAutomaticConversion(
        network, from_output, mediator_instance, "value"
        );
    
    return mediator_instance;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
OutgoingStreamMediator::OutgoingStreamMediator(const int& tag,
                                               const MessageHandler& handler) :
    Component(Type(typeid(OutgoingStreamMediator)), Version(0, 0, 0)),
    dm_tag(tag),
    dm_handler(handler),
    dm_converter()
{
    declareInput<MRN::PacketPtr>(
        "value", boost::bind(&OutgoingStreamMediator::handler, this, _1)
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OutgoingStreamMediator::handler(const MRN::PacketPtr& packet)
{
    packet->set_Tag(dm_tag);
    dm_handler(packet);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int OutgoingStreamMediator::tag() const
{
    return dm_tag;
}
