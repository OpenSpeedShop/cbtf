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

/** @file Definition of the XML functions. */

#include <boost/bind.hpp>
#include <KrellInstitute/CBTF/XML.hpp>
#include <utility>
#include <vector>

#include "Global.hpp"
#include "Network.hpp"
#include "ResolvePath.hpp"
#include "XercesExts.hpp"
#include "XML.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /**
     * Global associative container mapping the XML tags identifying the
     * kind of component network to their corresponding handler.
     */
    KRELL_INSTITUTE_CBTF_IMPL_GLOBAL(
        Handlers, std::map<std::string BOOST_PP_COMMA() DOMNodeHandler>
        )
        
    /** Invoke the appropriate handler for the specified node. */
    void invokeHandler(const boost::shared_ptr<xercesc::DOMDocument>& document,
                       const xercesc::DOMNode* node)
    {
        Handlers::GuardType guard_handlers(Handlers::mutex());

        char* transcoded_node_name = xercesc::XMLString::transcode(
            node->getNodeName()
            );

        Handlers::Type::const_iterator i = 
            Handlers::value().find(transcoded_node_name);
        
        if (i != Handlers::value().end())
        {
            (i->second)(document, node);
        }

        xercesc::XMLString::release(&transcoded_node_name);
    }

    /**
     * Statically initialized C++ structure registering the "Network" kind
     * of component network.
     */
    struct RegisterNetworkKind
    {
        RegisterNetworkKind()
        {
            registerKindOfComponentNetwork(
                "Network", boost::bind(&Network::registerXML, _1, _2)
                );
        }
    } register_network_kind;
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void KrellInstitute::CBTF::registerXML(const boost::filesystem::path& path)
{
    Handlers::GuardType guard_handlers(Handlers::mutex());
    
    std::vector<boost::filesystem::path> schema_paths;

    for (Handlers::Type::const_iterator
             i = Handlers::value().begin(); i != Handlers::value().end(); ++i)
    {
        boost::filesystem::path schema_path =
            resolvePath(kDataFileType, i->first + ".xsd");
        
        if (!schema_path.empty())
        {
            schema_paths.insert(
                (i->first == "Network") ? 
                    schema_paths.begin() : schema_paths.end(),
                schema_path
                );
        }
    }
    
    boost::shared_ptr<xercesc::DOMDocument> document =
        xercesc::loadFromFile(path, schema_paths);
    
    xercesc::selectNodes(document.get(), "./*",
                         boost::bind(invokeHandler, document, _1));
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void KrellInstitute::CBTF::Impl::registerKindOfComponentNetwork(
    const std::string& tag,
    const DOMNodeHandler& handler
    )
{
    Handlers::GuardType guard_handlers(Handlers::mutex());

    if (Handlers::value().find(tag) == Handlers::value().end())
    {
        Handlers::value().insert(std::make_pair(tag, handler));
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Component::Instance KrellInstitute::CBTF::Impl::instantiateXML(
    const boost::shared_ptr<xercesc::DOMDocument>& document,
    const xercesc::DOMNode* root
    )
{
    return Network::factoryFunction(document, root);
}
