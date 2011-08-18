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

/** @file Definition of the ComponentImpl class. */

#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/once.hpp>
#include <dlfcn.h>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <stdexcept>

#include "ComponentImpl.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
// Load the module with the specified path. Loading the module causes all of
// the components contained within the module to be registered with this class.
//------------------------------------------------------------------------------
void ComponentImpl::registerPlugin(const boost::filesystem::path& path)
{
    boost::filesystem::path real_path = path.extension().empty() ?
        boost::filesystem::change_extension(path, ".so") : path;

    if (dlopen(real_path.string().c_str(), RTLD_NOW) == NULL)
    {
        raise<std::runtime_error>(
            "The specified plugin (%1%) doesn't exist or is not "
            "of the correct format. dlopen() reported \"%2%\".",
            path, dlerror()
            );
    }
}



//------------------------------------------------------------------------------
// Add the specified component factory function to the set of available
// components. Doing so requires instantiating the component in order to
// obtain its type and version.
//------------------------------------------------------------------------------
void ComponentImpl::registerFactoryFunction(
    const Component::FactoryFunction& function
    )
{
    Component::Instance instance = function();

    boost::recursive_mutex::scoped_lock guard_factories(dm_factories_mutex);
    
    FactoryMap::iterator i = dm_factories.find(instance->getType());
    if (i == dm_factories.end())
    {
        i = dm_factories.insert(
            std::make_pair(instance->getType(), FactoryMap::mapped_type())
            ).first;
    }
    
    i->second.insert(std::make_pair(instance->getVersion(), function));
}



//------------------------------------------------------------------------------
// Construct and return the set of available component types.
//------------------------------------------------------------------------------
std::set<Type> ComponentImpl::getAvailableTypes()
{
    boost::recursive_mutex::scoped_lock guard_factories(dm_factories_mutex);
    
    std::set<Type> available_types;
    for (FactoryMap::const_iterator
             i = dm_factories.begin(); i != dm_factories.end(); ++i)
    {
        available_types.insert(i->first);
    }
    
    return available_types;
}



//------------------------------------------------------------------------------
// Construct and return the set of available versions of the given component
// type.
//------------------------------------------------------------------------------
std::set<Version> ComponentImpl::getAvailableVersions(const Type& type)
{
    boost::recursive_mutex::scoped_lock guard_factories(dm_factories_mutex);

    FactoryMap::const_iterator i = dm_factories.find(type);
    if (i == dm_factories.end())
    {
        raise<std::runtime_error>(
            "There are no available versions of "
            "of the given component type (%1%).",
            type
            );
    }

    std::set<Version> available_versions;
    for (FactoryMap::mapped_type::const_iterator
             j = i->second.begin(); j != i->second.end(); ++j)
    {
        available_versions.insert(j->first);
    }
    
    return available_versions;
}



//------------------------------------------------------------------------------
// Use the set of available components to find the component factory function
// corresponding to the specified component type and version, then instantiate
// the component and return it.
//------------------------------------------------------------------------------
Component::Instance ComponentImpl::instantiate(
    const Type& type,
    const boost::optional<Version>& version
    )
{
    boost::recursive_mutex::scoped_lock guard_factories(dm_factories_mutex);

    FactoryMap::const_iterator i = dm_factories.find(type);
    if (i == dm_factories.end())
    {
        raise<std::runtime_error>(
            "There is no factory function registered "
            "for the specified component type (%1%).",
            type
            );
    }

    if (!version)
    {
        return i->second.rbegin()->second();
    }

    FactoryMap::mapped_type::const_iterator j = i->second.find(version.get());
    if (j == i->second.end())
    {
        raise<std::runtime_error>(
            "There is no factory function registered "
            "for the specified component version (%1%).",
            version
            );
    }
    
    return j->second();
}



//------------------------------------------------------------------------------
// Check the validity of the specified connection and then add it to the
// output component's connections.
//------------------------------------------------------------------------------
void ComponentImpl::connect(Component::Instance output_instance,
                            const std::string& output_name,
                            Component::Instance input_instance,
                            const std::string& input_name)
{
    ComponentImpl& output_impl = *(output_instance->dm_impl);
    ComponentImpl& input_impl = *(input_instance->dm_impl);

    boost::unique_lock<boost::shared_mutex> guard_output(output_impl.dm_mutex);

    boost::shared_ptr<boost::shared_lock<boost::shared_mutex> > guard_input;
    if (input_instance->dm_impl != output_instance->dm_impl)
    {
        guard_input.reset(
            new boost::shared_lock<boost::shared_mutex>(input_impl.dm_mutex)
            );
    }
    
    if (output_impl.dm_outputs.find(output_name) == 
        output_impl.dm_outputs.end())
    {
        raise<std::runtime_error>(
            "The requested output (%1%) doesn't exist.", output_name
            );
    }
    
    if (input_impl.dm_inputs.find(input_name) ==
        input_impl.dm_inputs.end())
    {
        raise<std::runtime_error>(
            "The requested input (%1%) doesn't exist.", input_name
            );
    }
    
    if (output_impl.dm_outputs.find(output_name)->second !=
        input_impl.dm_inputs.find(input_name)->second)
    {
        raise<std::runtime_error>(
            "The requested output (%1%) and input "
            "(%2%) are not of compatible types.",
            output_name, input_name
            );
    }

    const ConnectionMap::const_iterator i_begin =
        output_impl.dm_connections.lower_bound(output_name);
    const ConnectionMap::const_iterator i_end =
        output_impl.dm_connections.upper_bound(output_name);

    for (ConnectionMap::const_iterator i = i_begin; i != i_end; ++i)
    {
        Component::Instance i_instance = i->second.first.lock();

        if (i_instance && (i_instance == input_instance) &&
            (i->second.second == input_name))
        {
            raise<std::runtime_error>(
                "The requested output (%1%) and input (%2%) "
                "are already connected to each other.",
                output_name, input_name
                );
        }
    }

    output_impl.dm_connections.insert(
        std::make_pair(output_name, std::make_pair(input_instance, input_name))
        );
}



//------------------------------------------------------------------------------
// Check the validity of the specified connection and then remove it from the
// output component's connections.
//------------------------------------------------------------------------------
void ComponentImpl::disconnect(Component::Instance output_instance,
                               const std::string& output_name,
                               Component::Instance input_instance,
                               const std::string& input_name)
{
    ComponentImpl& output_impl = *(output_instance->dm_impl);
    ComponentImpl& input_impl = *(input_instance->dm_impl);

    boost::unique_lock<boost::shared_mutex> guard_output(output_impl.dm_mutex);

    boost::shared_ptr<boost::shared_lock<boost::shared_mutex> > guard_input;
    if (input_instance->dm_impl != output_instance->dm_impl)
    {
        guard_input.reset(
            new boost::shared_lock<boost::shared_mutex>(input_impl.dm_mutex)
            );
    }
    
    if (output_impl.dm_outputs.find(output_name) ==
        output_impl.dm_outputs.end())
    {
        raise<std::runtime_error>(
            "The requested output (%1%) doesn't exist.", output_name
            );
    }
    
    if (input_impl.dm_inputs.find(input_name) ==
        input_impl.dm_inputs.end())
    {
        raise<std::runtime_error>(
            "The requested input (%1%) doesn't exist.", input_name
            );
    }

    const ConnectionMap::iterator i_begin =
        output_impl.dm_connections.lower_bound(output_name);
    const ConnectionMap::iterator i_end =
        output_impl.dm_connections.upper_bound(output_name);

    for (ConnectionMap::iterator i = i_begin; i != i_end; ++i)
    {
        Component::Instance i_instance = i->second.first.lock();

        if (i_instance && (i_instance == input_instance) &&
            (i->second.second == input_name))
        {
            output_impl.dm_connections.erase(i);
            return;
        }
    }
    
    raise<std::runtime_error>(
        "The requested output (%1%) and input "
        "(%2%) are not connected to each other.",
        output_name, input_name
        );
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ComponentImpl::ComponentImpl(const Type& type, const Version& version) :
    dm_mutex(),
    dm_type(type),
    dm_version(version),
    dm_inputs(),
    dm_outputs(),
    dm_handlers(),
    dm_connections()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ComponentImpl::~ComponentImpl()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string ComponentImpl::getBuildString() const
{
#if defined(__GNUC__)
    return boost::str(
        boost::format("Built by GCC %1%.%2%.%3% at %4% on %5%.")
        % __GNUC__ % __GNUC_MINOR__ % __GNUC_PATCHLEVEL__ % __TIME__ % __DATE__
        );    
#else
    return boost::str(
        boost::format("Built by an unidentified compiler at %1% on %2%.")
        % __TIME__ % __DATE__
        );
#endif
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Type ComponentImpl::getType() const
{
    return dm_type;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Version ComponentImpl::getVersion() const
{
    return dm_version;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::map<std::string, Type> ComponentImpl::getInputs() const
{
    boost::shared_lock<boost::shared_mutex> guard_this(dm_mutex);
    return dm_inputs;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::map<std::string, Type> ComponentImpl::getOutputs() const
{
    boost::shared_lock<boost::shared_mutex> guard_this(dm_mutex);
    return dm_outputs;
}



//------------------------------------------------------------------------------
// Add the specified input to this component's inputs.
//------------------------------------------------------------------------------
void ComponentImpl::declareInputImpl(
    const std::string& name,
    const Type& type,
    const boost::shared_ptr<Impl::Invoker>& handler
    )
{
    boost::unique_lock<boost::shared_mutex> guard_this(dm_mutex);

    if (dm_inputs.find(name) != dm_inputs.end())
    {
        raise<std::invalid_argument>(
            "An input has already been declared with the given name (%1%).",
            name
            );
    }

    dm_inputs.insert(std::make_pair(name, type));
    dm_handlers.insert(std::make_pair(name, handler));
}



//------------------------------------------------------------------------------
// Add the specified output to this component's outputs.
//------------------------------------------------------------------------------
void ComponentImpl::declareOutputImpl(const std::string& name, const Type& type)
{
    boost::unique_lock<boost::shared_mutex> guard_this(dm_mutex);

    if (dm_outputs.find(name) != dm_outputs.end())
    {
        raise<std::invalid_argument>(
            "An output has already been declared with the given name (%1%).",
            name
            );
    }

    dm_outputs.insert(std::make_pair(name, type));
}



//------------------------------------------------------------------------------
// Pass the specified output value to each of the component inputs to which
// the given output is connected.
//------------------------------------------------------------------------------
void ComponentImpl::emitOutputImpl(const std::string& name, const Type& type,
                                   const boost::any& value)
{
    boost::shared_lock<boost::shared_mutex> guard_this(dm_mutex);

    if (dm_outputs.find(name) == dm_outputs.end())
    {
        raise<std::invalid_argument>(
            "The requested output (%1%) wasn't declared.", name
            );
    }
    if (dm_outputs.find(name)->second != type)
    {
        raise<std::invalid_argument>(
            "The given value type (%1%) doesn't "
            "match the output's declared type (%2%).",
            type, dm_outputs.find(name)->second
            );
    }
    
    const ConnectionMap::const_iterator i_begin =
        dm_connections.lower_bound(name);
    const ConnectionMap::const_iterator i_end =
        dm_connections.upper_bound(name);
    
    for (ConnectionMap::const_iterator i = i_begin; i != i_end; ++i)
    {
        Component::Instance i_instance = i->second.first.lock();
        if (!i_instance)
        {
            continue;
        }

        const ComponentImpl& input_impl = *(i_instance->dm_impl);
        const std::string& input_name = i->second.second;

        BOOST_ASSERT(input_impl.dm_inputs.find(input_name) !=
                     input_impl.dm_inputs.end());
        BOOST_ASSERT(input_impl.dm_inputs.find(input_name)->second == type);
        
        const HandlerMap::const_iterator j = 
            input_impl.dm_handlers.find(input_name);
        if (j != input_impl.dm_handlers.end())
        {
            (*(j->second))(value);
        }
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ComponentImpl::FactoryMap ComponentImpl::dm_factories;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::recursive_mutex ComponentImpl::dm_factories_mutex;
