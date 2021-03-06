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

/** @file Definition of the MRNet class. */

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <stdexcept>

#include "InputMediator.hpp"
#include "MessageTags.hpp"
#include "MRNet.hpp"
#include "NamedStreams.hpp"
#include "OutputMediator.hpp"
#include "Raise.hpp"
#include "ResolvePath.hpp"
#include "XercesExts.hpp"
#include "XML.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {
    
    /**
     * Statically initialized C++ structure registering the "MRNet" kind
     * of component network.
     */
    struct RegisterMRNetKind
    {
        RegisterMRNetKind()
        {
            registerKindOfComponentNetwork(
                "MRNet", boost::bind(&MRNet::registerXML, _1, _2)
                );
        }
    } register_mrnet_kind;

    /** Assign the specified variable to the given constant value. */
    template <typename T>
    void assign(const xercesc::DOMNode* node, T& variable, const T& value)
    {
        variable = value;
    }
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::filesystem::path Impl::getMRNetBackendPath()
{
    const char* cbtf_be_path = getenv("CBTF_MRNET_BACKEND_PATH");
    boost::filesystem::path backend_path;
    if (cbtf_be_path != NULL) {
	backend_path = 
        resolvePath(kExecutableFileType, cbtf_be_path);
    } else {
	backend_path = 
        resolvePath(kExecutableFileType, BACKEND_FILE);
    }
    
    if (backend_path.empty())
    {
        raise<std::runtime_error>(
            "The path of the MRNet backend (%1%) could not be resolved.",
            BACKEND_FILE
            );            
    }
    
    return backend_path;
}



//------------------------------------------------------------------------------
// Determine the type of debugging that should be enabled, construct the
// arguments to the MRNet backend executable, and return those arguments.
//------------------------------------------------------------------------------
std::vector<std::string> Impl::getMRNetBackendArguments()
{
    bool is_backend_debug_enabled =
        ((getenv("CBTF_DEBUG_MRNET") != NULL) ||
         (getenv("CBTF_DEBUG_MRNET_BACKEND") != NULL));
    bool is_tracing_debug_enabled = 
        (getenv("CBTF_DEBUG_MRNET_TRACING") != NULL);
    
    std::vector<std::string> arguments;
    if (is_backend_debug_enabled)
    {
        arguments.push_back("--debug");
    }
    if (is_tracing_debug_enabled)
    {
        arguments.push_back("--tracing");
    }
    
    return arguments;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TopologyInfo Impl::TheTopologyInfo;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MRNet::registerXML(
    const boost::shared_ptr<xercesc::DOMDocument>& document,
    const xercesc::DOMNode* root
    )
{
    Component::registerFactoryFunction(
        boost::bind(&MRNet::factoryFunction, document, root)
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Component::Instance MRNet::factoryFunction(
    const boost::shared_ptr<xercesc::DOMDocument>& document,
    const xercesc::DOMNode* root
    )
{
    return Component::Instance(
        reinterpret_cast<Component*>(
            new MRNet(
                Type(xercesc::selectValue(root, "./Type")),
                Version(xercesc::selectValue(root, "./Version")),
                document, root
                )
            )
        );
}



//------------------------------------------------------------------------------
// Send a DestroyNetwork message to the backends and filters, and then remove
// all of our message handlers.
//------------------------------------------------------------------------------
MRNet::~MRNet()
{
    if (dm_frontend)
    {
        dm_frontend->sendToBackends(MRN::PacketPtr(new MRN::Packet(
            0, MessageTags::DestroyNetwork, "%d",
            dm_local_component_network.named_streams()->uid()
            )));

        dm_frontend->MessageHandlers.remove(
            dm_local_component_network.named_streams()->uid()
            );
    }
}



//------------------------------------------------------------------------------
// Parse the specified <MRNet> XML node, define the named streams, construct
// the frontend's local component network, and declare that network's inputs
// and outputs.
//------------------------------------------------------------------------------
MRNet::MRNet(const Type& type, const Version& version,
             const boost::shared_ptr<xercesc::DOMDocument>& document,
             const xercesc::DOMNode* root) :
    Component(type, version),
    dm_document(document),
    dm_root(root),
    dm_local_component_network(),
    dm_frontend(),
    dm_mediators()
{
    dm_local_component_network.initializeStepOne(
        boost::shared_ptr<NamedStreams>(new NamedStreams(root))
        );
    
    xercesc::selectNodes(root, "./Frontend",
                         boost::bind(&MRNet::parseFrontend, this, _1));
    xercesc::selectNodes(root, "./Input",
                         boost::bind(&MRNet::parseInput, this, _1));
    xercesc::selectNodes(root, "./Output",
                         boost::bind(&MRNet::parseOutput, this, _1));

    declareInput<boost::shared_ptr<MRN::Network> >(
        "Network", boost::bind(&MRNet::handleNetwork, this, _1)
        );    
}



//------------------------------------------------------------------------------
// Bind the specified incoming upstream mediator by adding its handler()
// method to the frontend's message handlers.
//------------------------------------------------------------------------------
void MRNet::bindIncomingUpstream(
    boost::shared_ptr<IncomingStreamMediator>& mediator
    )
{
    dm_frontend->MessageHandlers.add(
        dm_local_component_network.named_streams()->uid(),
        mediator->tag(),
        boost::bind(&IncomingStreamMediator::handler, mediator, _1)
        );
}



//------------------------------------------------------------------------------
// Create a new MRNet frontend using the specified MRNet network, create the
// necessary stream mediators, and then send the named streams, filters, and
// backend specifications.
//------------------------------------------------------------------------------
void MRNet::handleNetwork(const boost::shared_ptr<MRN::Network>& network)
{
    if (dm_frontend)
    {
        raise<std::runtime_error>("Only one MRNet network may be specified.");
    }

    // Default to WAITFORALL.
    MRN::FilterId mode = MRN::SFILTER_DONTWAIT;

    std::string filter_mode = xercesc::selectValue(dm_root, "./FilterMode");
    if (filter_mode == "DontWait")
    {
        mode = MRN::SFILTER_DONTWAIT;
    }
    else if (filter_mode == "WaitForAll")
    {
        mode = MRN::SFILTER_WAITFORALL;
    }
    else if (filter_mode == "TimeOut")
    {
        mode = MRN::SFILTER_TIMEOUT;
    }
    
    dm_frontend = Frontend::instantiate(network, mode);

    dm_local_component_network.initializeStepThree(
        boost::bind(&MRNet::bindIncomingUpstream, this, _1),
        LocalComponentNetwork::IncomingBinder(), // No Incoming Downstreams
        MessageHandler(), // No Outgoing Upstreams
        boost::bind(&Frontend::sendToBackends, boost::ref(dm_frontend), _1)
        );
    
    dm_frontend->sendToBackends(*dm_local_component_network.named_streams());

    //
    // The first "./Filter" node selection below sends all filter specifications
    // whose depth is not "AllOther". The second sends those whose depth is that
    // value. This insures the "AllOther" specification(s) are always last, and
    // that allows libcbtf-mrnet-filter, when an "AllOther" is received, to only
    // test "Have I been previously selected?" in order to determine whether or
    // not the "AllOther" should apply to it.
    //

    xercesc::selectNodes(
        dm_root, "./Filter", boost::bind(&MRNet::parseFilter, this, _1, false)
        );
    xercesc::selectNodes(
        dm_root, "./Filter", boost::bind(&MRNet::parseFilter, this, _1, true)
        );
    xercesc::selectNodes(
        dm_root, "./Backend", boost::bind(&MRNet::parseBackend, this, _1)
        );

    //
    // The ordering of the calls above ensure a tool that launches
    // backends will see a completely populated distributed component
    // network before any backend starts generating messages from
    // the backends described in the xml.  For tools that attach to
    // backends, the following NetworkReady tag can be used in the
    // backend connection code as a sync condition to wait until
    // all xml has been parsed for the Filters.  Without it, the
    // backends (which are not defined by any xml) can start streaming
    // messages before all levels of the filter network are populated
    // with filter components.
    //

    dm_frontend->sendToBackends(MRN::PacketPtr(new MRN::Packet(
            0, MessageTags::NetworkReady, "%d",
            dm_local_component_network.named_streams()->uid()
            )));
}


//------------------------------------------------------------------------------
// Send a SpecifyBackend message to the backends containing the contents of
// this <Backend> XML node.
//------------------------------------------------------------------------------
void MRNet::parseBackend(const xercesc::DOMNode* node)
{
    dm_frontend->sendToBackends(MRN::PacketPtr(new MRN::Packet(
        0, MessageTags::SpecifyBackend, "%d %s",
        dm_local_component_network.named_streams()->uid(),
        xercesc::saveToString(node).c_str()
        )));
}



//------------------------------------------------------------------------------
// Send a SpecifyFilter message to the backends containing the contents of
// this <Filter> XML node. The filters also receive this message on its way
// down to the backends.
//------------------------------------------------------------------------------
void MRNet::parseFilter(const xercesc::DOMNode* node, bool sendAllOther)
{
    bool isAllOther = false;

    xercesc::selectNodes(
        node, "./Depth/AllOther",
        boost::bind(&assign<bool>, _1, isAllOther, true)
        );

    if (sendAllOther == isAllOther)
    {
        dm_frontend->sendToBackends(MRN::PacketPtr(new MRN::Packet(
            0, MessageTags::SpecifyFilter, "%d %s",
            dm_local_component_network.named_streams()->uid(),
            xercesc::saveToString(node).c_str()
            )));
    }
}



//------------------------------------------------------------------------------
// Invoke step two of the local component network's initialization.
//------------------------------------------------------------------------------
void MRNet::parseFrontend(const xercesc::DOMNode* node)
{
    dm_local_component_network.initializeStepTwo(dm_document, node);
}



//------------------------------------------------------------------------------
// Parse the specified <Input> XML node, create an appropriate input mediator,
// establish the connection, and declare the input.
//------------------------------------------------------------------------------
void MRNet::parseInput(const xercesc::DOMNode* node)
{
    const std::string name = xercesc::selectValue(node, "./Name");
    const std::string to_input = xercesc::selectValue(node, "./To/Input");

    std::map<std::string, Type> to_inputs = 
        dm_local_component_network.network()->getInputs();
    
    std::map<std::string, Type>::const_iterator i = to_inputs.find(to_input);
    
    if (i == to_inputs.end())
    {
        raise<std::runtime_error>(
            "The requested input (%1%) doesn't exist.", to_input
            );
    }

    boost::shared_ptr<InputMediator> input_mediator(
        new InputMediator(i->second)
        );

    Component::Instance input_mediator_instance =
        boost::reinterpret_pointer_cast<Component>(input_mediator);
    
    Component::connect(
        input_mediator_instance, "value",
        dm_local_component_network.network(), to_input
        );
    
    dm_mediators.push_back(input_mediator_instance);
    
    declareInput(
        name, i->second, 
        boost::bind(&InputMediator::handler, input_mediator.get(), _1)
        );
}



//------------------------------------------------------------------------------
// Parse the specified <Output> XML node, create an appropriate output mediator,
// establish the connection, and declare the output.
//------------------------------------------------------------------------------
void MRNet::parseOutput(const xercesc::DOMNode* node)
{
    const std::string name = xercesc::selectValue(node, "./Name");    
    const std::string from_output = xercesc::selectValue(node, "./From/Output");
    
    std::map<std::string, Type> from_outputs = 
        dm_local_component_network.network()->getOutputs();
    
    std::map<std::string, Type>::const_iterator i =
        from_outputs.find(from_output);
    
    if (i == from_outputs.end())
    {
        raise<std::runtime_error>(
            "The requested output (%1%) doesn't exist.", from_output
            );
    }

    boost::shared_ptr<OutputMediator> output_mediator(
        new OutputMediator(
            i->second,
            boost::bind(
                (void (Component::*)(
                    const std::string&, const Type&, const boost::any&
                    ))(&MRNet::emitOutput),
                this, name, i->second, _1
                )
            )
        );
    
    Component::Instance output_mediator_instance =
        boost::reinterpret_pointer_cast<Component>(output_mediator);

    Component::connect(
        dm_local_component_network.network(), from_output,
        output_mediator_instance, "value"
        );

    dm_mediators.push_back(output_mediator_instance);
    
    declareOutput(name, i->second);
}
