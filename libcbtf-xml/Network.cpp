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

/** @file Definition of the Network class. */

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/home/classic.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <cstdlib>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Impl/InputMediator.hpp>
#include <KrellInstitute/CBTF/Impl/OutputMediator.hpp>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <KrellInstitute/CBTF/Impl/XercesExts.hpp>
#include <KrellInstitute/CBTF/XML.hpp>
#include <set>
#include <stdexcept>

#include "Network.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Type of associative container used to track the loaded plugins. */
    typedef std::set<boost::filesystem::path> PluginSet;

    /** Set of loaded plugins. */
    PluginSet plugins;

    /** Mutual exclusion lock for the set of loaded plugins. */
    boost::recursive_mutex plugins_mutex;

    /** Push the value of the specified node onto the given vector of paths. */
    void pushPath(const xercesc::DOMNode* node,
                  std::vector<boost::filesystem::path>& paths)
    {
        paths.push_back(xercesc::selectValue(node, "."));
    }
    
    /**
     * Register the plugin at the specified path if it wasn't previously
     * registered, and return a boolean flag indicating if the plugin is
     * now registered.
     */
    bool registerPlugin(const boost::filesystem::path& path)
    {
        using namespace boost::filesystem;
        
        boost::recursive_mutex::scoped_lock guard_plugins(plugins_mutex);
        
        if (plugins.find(path) != plugins.end())
        {
            return true;
        }
        
        try
        {
            if (is_regular_file(path) && (extension(path) == ".xml"))
            {
                registerXML(path);
            }
            else
            {
                Component::registerPlugin(path);
            }

            plugins.insert(path);

            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Network::registerXML(
    const boost::shared_ptr<xercesc::DOMDocument>& document,
    const xercesc::DOMNode* root
    )
{
    Component::registerFactoryFunction(
        boost::bind(&Network::factoryFunction, document, root)
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Component::Instance Network::factoryFunction(
    const boost::shared_ptr<xercesc::DOMDocument>& document,
    const xercesc::DOMNode* root
    )
{
    std::vector<boost::filesystem::path> search_paths, plugin_paths;
    
    search_paths.push_back(boost::filesystem::path(PLUGIN_DIR));
    
    const char* cbtf_plugin_paths = getenv("CBTF_PLUGIN_PATH");
    if (cbtf_plugin_paths != NULL)
    {
        using namespace boost::spirit::classic;
        
        parse(
            cbtf_plugin_paths,
            list_p((+~ch_p(':'))[push_back_a(search_paths)], ch_p(':')),
            space_p
            );
    }
    
    xercesc::selectNodes(root, "./SearchPath",
                         boost::bind(&pushPath, _1, boost::ref(search_paths)));
    xercesc::selectNodes(root, "./Plugin",
                         boost::bind(&pushPath, _1, boost::ref(plugin_paths)));
    
    for (std::vector<boost::filesystem::path>::const_iterator
             i = plugin_paths.begin(); i != plugin_paths.end(); ++i)
    {
        bool was_found = false;
        
        if (i->is_complete())
        {
            was_found = ::registerPlugin(*i);
        }
        else
        {
            for (std::vector<boost::filesystem::path>::const_reverse_iterator
                     j = search_paths.rbegin(); 
                 !was_found && (j != search_paths.rend());
                 ++j)
            {
                was_found = ::registerPlugin(*j / *i);
            }
        }
        
        if (!was_found)
        {
            raise<std::runtime_error>(
                "One of the specified plugins (%1%) couldn't be found.", *i
                );
        }
    }
    
    return Component::Instance(
        reinterpret_cast<Component*>(
            new Network(
                Type(xercesc::selectValue(root, "./Type")),
                Version(xercesc::selectValue(root, "./Version")),
                document, root
                )
            )
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Network::~Network()
{
}



//------------------------------------------------------------------------------
// Parse the specified <Network> XML node, construct the corresponding component
// network, and declare that network's inputs and outputs.
//------------------------------------------------------------------------------
Network::Network(const Type& type, const Version& version,
                 const boost::shared_ptr<xercesc::DOMDocument>& /* Unused */,
                 const xercesc::DOMNode* root) :
    Component(type, version),
    dm_components(),
    dm_mediators()
{
    xercesc::selectNodes(root, "./Component",
                         boost::bind(&Network::parseComponent, this, _1));
    xercesc::selectNodes(root, "./Input",
                         boost::bind(&Network::parseInput, this, _1));
    xercesc::selectNodes(root, "./Connection",
                         boost::bind(&Network::parseConnection, this, _1));
    xercesc::selectNodes(root, "./Output",
                         boost::bind(&Network::parseOutput, this, _1));
}
        


//------------------------------------------------------------------------------
// Parse the specified <Component> XML node, attempt to instantiate a component
// of the requested type, and then add the instance to the network's components.
//------------------------------------------------------------------------------
void Network::parseComponent(const xercesc::DOMNode* node)
{
    const std::string name = xercesc::selectValue(node, "./Name");
    
    if (dm_components.find(name) != dm_components.end())
    {
        raise<std::runtime_error>(
            "The component name \"%1%\" isn't unique within the network.", name
            );
    }

    const Type type(xercesc::selectValue(node, "./Type"));
    
    boost::optional<Version> minimum_version, maximum_version;

    try
    {
        minimum_version = Version(
            xercesc::selectValue(node, "./Version/@minimum")
            );
        maximum_version = Version(
            xercesc::selectValue(node, "./Version/@maximum")
            );
    }
    catch (...)
    {
    }

    if (!minimum_version && !maximum_version)
    {
        dm_components.insert(
            std::make_pair(name, Component::instantiate(type))
            );
    }
    else
    {
        boost::optional<Version> version;

        const std::set<Version> available_versions = 
            Component::getAvailableVersions(type);
        
        for (std::set<Version>::const_iterator i = available_versions.begin();
             i != available_versions.end();
             ++i)
        {
            if ((!minimum_version || (*i >= minimum_version)) &&
                (!maximum_version || (*i <= maximum_version)) &&
                (!version || (*i > version)))
            {
                version = *i;
            }
        }

        if (!version)
        {
            raise<std::runtime_error>(
                "No suitable version in the range [%1%, %2%] "
                "found for the component named \"%3%\".",
                minimum_version, maximum_version, name
                );
        }

        dm_components.insert(
            std::make_pair(name, Component::instantiate(type, version.get()))
            );
    }
}



//------------------------------------------------------------------------------
// Parse the specified <Connection> XML node, locate the requested components
// within this network, and then establish the connection.
//------------------------------------------------------------------------------
void Network::parseConnection(const xercesc::DOMNode* node)
{
    const std::string from_name = xercesc::selectValue(node, "./From/Name");
    const std::string from_output = xercesc::selectValue(node, "./From/Output");

    const ComponentMap::iterator from = dm_components.find(from_name);

    if (from == dm_components.end())
    {
        raise<std::runtime_error>(
            "The component name \"%1%\" isn't found within the network.",
            from_name
            );
    }

    const std::string to_name = xercesc::selectValue(node, "./To/Name");
    const std::string to_input = xercesc::selectValue(node, "./To/Input");

    const ComponentMap::iterator to = dm_components.find(to_name);

    if (to == dm_components.end())
    {
        raise<std::runtime_error>(
            "The component name \"%1%\" isn't found within the network.",
            to_name
            );
    }

    Component::connect(from->second, from_output, to->second, to_input);
}



//------------------------------------------------------------------------------
// Parse the specified <Input> XML node, locate the requested component
// within this network, create an appropriate input mediator, establish
// the connection, and declare the input.
//------------------------------------------------------------------------------
void Network::parseInput(const xercesc::DOMNode* node)
{
    const std::string input_name = xercesc::selectValue(node, "./Name");

    const std::string to_name = xercesc::selectValue(node, "./To/Name");
    const std::string to_input = xercesc::selectValue(node, "./To/Input");

    const ComponentMap::iterator to = dm_components.find(to_name);

    if (to == dm_components.end())
    {
        raise<std::runtime_error>(
            "The component name \"%1%\" isn't found within the network.",
            to_name
            );
    }

    std::map<std::string, Type> to_inputs = to->second->getInputs();
    
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
        input_mediator_instance, "value", to->second, to_input
        );
    
    dm_mediators.push_back(input_mediator_instance);
    
    declareInput(
        input_name, i->second, 
        boost::bind(&InputMediator::handler, input_mediator.get(), _1)
        );
}



//------------------------------------------------------------------------------
// Parse the specified <Output> XML node, locate the requested component
// within this network, create an appropriate output mediator, establish
// the connection, and declare the output.
//------------------------------------------------------------------------------
void Network::parseOutput(const xercesc::DOMNode* node)
{
    const std::string output_name = xercesc::selectValue(node, "./Name");
    
    const std::string from_name = xercesc::selectValue(node, "./From/Name");
    const std::string from_output = xercesc::selectValue(node, "./From/Output");
    
    const ComponentMap::iterator from = dm_components.find(from_name);

    if (from == dm_components.end())
    {
        raise<std::runtime_error>(
            "The component name \"%1%\" isn't found within the network.",
            from_name
            );
    }

    std::map<std::string, Type> from_outputs = from->second->getOutputs();
    
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
                    ))(&Component::emitOutput),
                this, output_name, i->second, _1
                )
            )
        );
    
    Component::Instance output_mediator_instance =
        boost::reinterpret_pointer_cast<Component>(output_mediator);

    Component::connect(
        from->second, from_output, output_mediator_instance, "value"
        );

    dm_mediators.push_back(output_mediator_instance);
    
    declareOutput(output_name, i->second);
}
