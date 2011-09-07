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

/** @file Declaration of the ComponentImpl class. */

#pragma once

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/utility.hpp>
#include <boost/weak_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Impl/Invoker.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <map>
#include <string>
#include <utility>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Implementation details of the Component class. Anything that
     * would normally be a private member of Component is instead a
     * member of ComponentImpl.
     */
    class ComponentImpl :
        private boost::noncopyable
    {

    public:

        /** Register a plugin providing one or more component types. */
        static void registerPlugin(const boost::filesystem::path& path);
        
        /** Register a factory function which instantiates components. */
        static void registerFactoryFunction(
            const Component::FactoryFunction& function
            );

        /** Get the available component types. */
        static std::set<Type> getAvailableTypes();

        /** Get the available versions of the given component type. */
        static std::set<Version> getAvailableVersions(const Type& type);

        /** Instantiate a new component of the given type. */
        static Component::Instance instantiate(
            const Type& type,
            const boost::optional<Version>& version
            );

        /** Connect a component's output to a component's input. */
        static void connect(Component::Instance output_instance,
                            const std::string& output_name,
                            Component::Instance input_instance,
                            const std::string& input_name);
        
        /** Disconnect a component's output from a component's input. */
        static void disconnect(Component::Instance output_instance,
                               const std::string& output_name,
                               Component::Instance input_instance,
                               const std::string& input_name);
        
        /** Constructor. */
        ComponentImpl(const Type& type, const Version& version);
        
        /** Destructor. */
        ~ComponentImpl();

        /** Get this component's build string. */
        std::string getBuildString() const;

        /** Get this component's type. */
        Type getType() const;

        /** Get this component's version. */
        Version getVersion() const;

        /** Get this component's inputs. */
        std::map<std::string, Type> getInputs() const;

        /** Get this component's outputs. */
        std::map<std::string, Type> getOutputs() const;

        /** Declare an input of this component. */
        void declareInputImpl(const std::string& name, const Type& type,
                              const boost::shared_ptr<Impl::Invoker>& handler);

        /** Declare an output of this component. */
        void declareOutputImpl(const std::string& name, const Type& type);
        
        /** Emit an output of this component. */
        void emitOutputImpl(const std::string& name, const Type& type,
                            const boost::any& value);
        
    private:

        /**
         * Type of associative container used to map a component's outputs to
         * their connected component inputs.
         */
        typedef std::multimap<
            std::string, std::pair<boost::weak_ptr<Component>, std::string>
            > ConnectionMap;

        /**
         * Type of associative container used to map a component's inputs to
         * their corresponding handler function.
         */
        typedef std::map<
            std::string, boost::shared_ptr<Impl::Invoker>
            > HandlerMap;

        /** Mutual exclusion lock for this component. */
        mutable boost::shared_mutex dm_mutex;
        
        /** Type of this component. */
        const Type dm_type;

        /** Version of this component. */
        const Version dm_version;
        
        /** Inputs of this component. */
        std::map<std::string, Type> dm_inputs;
        
        /** Outputs of this component. */
        std::map<std::string, Type> dm_outputs;
        
        /** Input handler functions of this component. */
        HandlerMap dm_handlers;
        
        /** Connections emanating from this component. */
        ConnectionMap dm_connections;
        
    }; // class ComponentImpl
        
} } } // namespace KrellInstitute::CBTF::Impl
