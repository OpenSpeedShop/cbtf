////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

/** @file Definition of the Component class. */

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>

#include "ComponentImpl.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::registerPlugin(const boost::filesystem::path& path)
{
    ComponentImpl::registerPlugin(path);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::set<Type> Component::getAvailableTypes()
{
    return ComponentImpl::getAvailableTypes();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::set<Version> Component::getAvailableVersions(const Type& type)
{
    return ComponentImpl::getAvailableVersions(type);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Component::Instance Component::instantiate(
    const Type& type,
    const boost::optional<Version>& version
    )
{
    return ComponentImpl::instantiate(type, version);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::connect(Component::Instance output_instance,
                        const std::string& output_name,
                        Component::Instance input_instance,
                        const std::string& input_name)
{
    ComponentImpl::connect(output_instance, output_name,
                           input_instance, input_name);
}
        


//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::disconnect(Component::Instance output_instance,
                           const std::string& output_name,
                           Component::Instance input_instance,
                           const std::string& input_name)
{
    ComponentImpl::disconnect(output_instance, output_name,
                              input_instance, input_name);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Component::~Component()
{
    delete dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::string Component::getBuildString() const
{
    return dm_impl->getBuildString();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type Component::getType() const
{
    return dm_impl->getType();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version Component::getVersion() const
{
    return dm_impl->getVersion();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::map<std::string, Type> Component::getInputs() const
{
    return dm_impl->getInputs();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::map<std::string, Type> Component::getOutputs() const
{
    return dm_impl->getOutputs();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::registerFactoryFunction(
    const Component::FactoryFunction& function
    )
{
    ComponentImpl::registerFactoryFunction(function);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Component::Component(const Type& type, const Version& version) :
    dm_impl(new ComponentImpl(type, version))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::declareInputImpl(
    const std::string& name,
    const Type& type,
    const boost::shared_ptr<Impl::Invoker>& handler
    )
{
    dm_impl->declareInputImpl(name, type, handler);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::declareOutputImpl(const std::string& name, const Type& type)
{
    dm_impl->declareOutputImpl(name, type);
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
void Component::emitOutputImpl(const std::string& name, const Type& type,
                               const boost::any& value)
{
    dm_impl->emitOutputImpl(name, type, value);
}
