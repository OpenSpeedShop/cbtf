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

/** @file Definition of the IncomingStreamMediator class. */

#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <string>
#include <typeinfo>

#include "IncomingStreamMediator.hpp"
#include "Raise.hpp"
#include "XercesExts.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
// Parse the specified <Incoming[Downstream|Upstream]> XML node, create a new
// incoming stream mediator, and establish the connection.
//------------------------------------------------------------------------------
Component::Instance IncomingStreamMediator::create(
    const xercesc::DOMNode* node,
    Component::Instance network,
    const NamedStreams& named_streams
    )
{
    const std::string name = xercesc::selectValue(node, "./Name");
    const std::string to_input = xercesc::selectValue(node, "./To/Input");

    boost::shared_ptr<IncomingStreamMediator> mediator(
        new IncomingStreamMediator(named_streams.tag(name))
        );

    Component::Instance mediator_instance =
        boost::reinterpret_pointer_cast<Component>(mediator);

    Component::connect(mediator_instance, "value", network, to_input);

    return mediator_instance;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void IncomingStreamMediator::handler(const MRN::PacketPtr& packet)
{
    if (packet->get_Tag() != dm_tag)
    {
        raise<std::invalid_argument>(
            "The incoming message to be mediated has the wrong MRNet tag."
            );
    }
    emitOutput<MRN::PacketPtr>("value", packet);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
IncomingStreamMediator::IncomingStreamMediator(const int& tag) :
    Component(Type(typeid(IncomingStreamMediator)), Version(0, 0, 0)),
    dm_tag(tag)
{
    declareOutput<MRN::PacketPtr>("value");
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int IncomingStreamMediator::tag() const
{
    return dm_tag;
}
