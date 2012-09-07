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

/** @file Declaration of the Component class. */

#pragma once

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Impl/Invoker.hpp>
#include <KrellInstitute/CBTF/Impl/InvokerFor.hpp>
#include <KrellInstitute/CBTF/Impl/InvokerForAny.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <map>
#include <set>
#include <string>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF {

    namespace Impl {
        class ComponentImpl;
        template <typename T> struct RegisterFactoryFunction;
    }

    /**
     * Abstract base class for a component that accepts zero or more inputs and
     * produces zero or more outputs. The framework neither imposes nor implies
     * the scope of operation(s) performed by a component.
     */
    class Component :
        private boost::noncopyable
    {
        friend class Impl::ComponentImpl;
        template <typename T> friend struct Impl::RegisterFactoryFunction;
        
    public:

        /** Type of pointer used when referring to a component instance. */
        typedef boost::shared_ptr<Component> Instance;
        
        /**
         * Type of factory function which instantiates components.
         *
         * @sa http://en.wikipedia.org/wiki/Factory_function
         */
        typedef boost::function<Instance ()> FactoryFunction;
        
        /**
         * Register a plugin providing one or more component types. By default
         * only those component types that are directly linked into the client
         * are available. The client is responsible for finding and registering
         * plugins containing any additional component types that are required.
         *
         * @param path    Path of the plugin to be registered.
         *
         * @throw std::runtime_error    The specified plugin doesn't exist
         *                              or is not of the correct format.
         */
        static void registerPlugin(const boost::filesystem::path& path);
        
        /**
         * Get the available component types. All available component types are
         * returned - regardless of whether they are located within a plugin or
         * are directly linked into the client.
         *
         * @return    Available component types.
         */
        static std::set<Type> getAvailableTypes();

        /**
         * Get the available versions of the given component type. All available
         * versions are returned - regardless of whether they are located within
         * a plugin or are directly linked into the client.
         *
         * @param type    Type of component to be queried.
         * @return        Available component versions.
         *
         * @throw std::runtime_error    There are no available versions
         *                              of the given component type.
         */
        static std::set<Version> getAvailableVersions(const Type& type);

        /**
         * Instantiate a new component of the given type. The component version
         * to be instantiated can optionally be specified, otherwise an instance
         * of the most recent version of the component is returned.
         *
         * @param type       Type of component to be instantiated.
         * @param version    Optional version of component to be instantiated.
         * @return           A new instance of that type.
         *
         * @throw std::runtime_error    There is no factory function
         *                              registered for the specified
         *                              component type or version.
         */
        static Component::Instance instantiate(
            const Type& type,
            const boost::optional<Version>& version = boost::optional<Version>()
            );
        
        /**
         * Connect a component's output to a component's input. Values emitted
         * on the output are directly conveyed to the input.
         *
         * @param output_instance    Component with output being connected.
         * @param output_name        Name of output being connected.
         * @param input_instance     Component with input being connected.
         * @param input_name         Name of input being connected.
         *
         * @throw std::runtime_error    The requested input or output
         *                              doesn't exist, they are not of
         *                              compatible types, or the two
         *                              are already connected to each
         *                              other.
         */
        static void connect(Component::Instance output_instance,
                            const std::string& output_name,
                            Component::Instance input_instance,
                            const std::string& input_name);
        
        /**
         * Disconnect a component's output from a component's input. Values
         * emitted on the output are no longer conveyed to the input.
         *
         * @param output_instance    Component with output being disconnected.
         * @param output_name        Name of output being disconnected.
         * @param input_instance     Component with input being disconnected.
         * @param input_name         Name of input being disconnected.
         *
         * @throw std::runtime_error    The requested input or output
         *                              doesn't exist, or the two are
         *                              not connected to each other.
         */
        static void disconnect(Component::Instance output_instance,
                               const std::string& output_name,
                               Component::Instance input_instance,
                               const std::string& input_name);
        
        /** Destructor. */
        virtual ~Component();
        
        /**
         * Get this component's build string. This string contains a freeform
         * description of the date/time when the component was built, and the
         * compiler with which it was built.
         *
         * @return    Build string of this component.
         *
         * @note    The precise content of this string is subject to change
         *          without notice. The caller should <em>not</em> make any
         *          assumptions regarding its format.
         */
        std::string getBuildString() const;

        /**
         * Get this component's type.
         *
         * @return    Type of this component.
         */
        Type getType() const;

        /**
         * Get this component's version.
         *
         * @return    Version of this component.
         */
        Version getVersion() const;

        /** 
         * Get this component's inputs.
         *
         * @return    Map of names of this component's inputs to their types.
         *
         * @note    An empty map is returned if the component has no inputs.
         */
        std::map<std::string, Type> getInputs() const;

        /** 
         * Get this component's outputs.
         *
         * @return    Map of names of this component's outputs to their types.
         *
         * @note    An empty map is returned if the component has no outputs.
         */
        std::map<std::string, Type> getOutputs() const;
        
    protected:

        /**
         * Register a factory function which instantiates components.
         *
         * @param function    Factory function to be registered.
         *
         * @note    Factory functions can be registered while loading an
         *          executable, before a try/catch block could possibly be
         *          in place. Thus attempts to register a factory function
         *          more than once, or two factory functions instantiating
         *          components of the same type, are silently ignored and
         *          no exception is thrown.
         */
        static void registerFactoryFunction(
            const Component::FactoryFunction& function
            );
        
        /**
         * Construct a new component of the given type and version. Called from
         * the constructor of a derived class to initialize this base class.
         *
         * @param type       Type of this component.
         * @param version    Version of this component.
         */
        Component(const Type& type, const Version& version);

        /**
         * Declare an input of this component. Called from the constructor of
         * a derived class to declare one of the component's inputs.
         *
         * @tparam T         Type of the input being declared.
         * @param name       Name of the input being declared.
         * @param handler    Handler function to be called when receiving a
         *                   new value on this input.
         *
         * @throw std::invalid_argument    An input has already been
         *                                 declared with the given name.
         */
        template <typename T>
        void declareInput(const std::string& name,
                          const boost::function<void (const T&)>& handler)
        {
            declareInputImpl(
                name, Type(typeid(T)),
                boost::shared_ptr<Impl::Invoker>(
                    new Impl::InvokerFor<T>(handler)
                    )
                );
        }

        /**
         * Declare an input of this component. Called from the constructor of
         * a derived class to declare one of the component's inputs.
         *
         * @tparam T         Type of the input being declared.
         * @param name       Name of the input being declared.
         * @param handler    Handler function to be called when receiving a
         *                   new value on this input.
         *
         * @throw std::invalid_argument    An input has already been
         *                                 declared with the given name.
         */
        void declareInput(
            const std::string& name, const Type& type,
            const boost::function<void (const boost::any&)>& handler
            )
        {
            declareInputImpl(
                name, type,
                boost::shared_ptr<Impl::Invoker>(
                    new Impl::InvokerForAny(handler)
                    )
                );
        }

        /**
         * Declare an output of this component. Called from the constructor of
         * a derived class to declare one of the component's outputs.
         *
         * @tparam T      Type of the output being declared.
         * @param name    Name of the output being declared.
         *
         * @throw std::invalid_argument    An output has already been
         *                                 declared with the given name.
         */
        template <typename T>
        void declareOutput(const std::string& name)
        {
            declareOutputImpl(name, Type(typeid(T)));
        }

        /**
         * Declare an output of this component. Called from the constructor of
         * a derived class to declare one of the component's outputs.
         *
         * @param name    Name of the output being declared.
         * @param type    Type of the output being declared.
         *
         * @throw std::invalid_argument    An output has already been
         *                                 declared with the given name.
         */
        void declareOutput(const std::string& name, const Type& type)
        {
            declareOutputImpl(name, type);
        }

        /**
         * Emit an output of this component. Called by a derived class to emit
         * one of the component's outputs.
         *
         * @tparam T       Type of the value being emitted.
         * @param name     Name of the output being emitted.
         * @param value    Value being emitted.
         *
         * @throw std::invalid_argument    The requested output wasn't declared
         *                                 or the given value type doesn't match
         *                                 the output's declared type.
         */
        template <typename T>
        void emitOutput(const std::string& name, const T& value)
        {
            emitOutputImpl(name, Type(typeid(T)), boost::any(value));
        }

        /**
         * Emit an output of this component. Called by a derived class to emit
         * one of the component's outputs.
         *
         * @param name     Name of the output being emitted.
         * @param type     Type of the value being emitted.
         * @param value    Value being emitted.
         *
         * @throw std::invalid_argument    The requested output wasn't declared
         *                                 or the given value type doesn't match
         *                                 the output's declared type.
         */
        void emitOutput(const std::string& name, const Type& type,
                        const boost::any& value)
        {
            emitOutputImpl(name, type, value);
        }
        
    private:

        /** Declare an input of this component. */
        void declareInputImpl(const std::string& name, const Type& type,
                              const boost::shared_ptr<Impl::Invoker>& handler);

        /** Declare an output of this component. */
        void declareOutputImpl(const std::string& name, const Type& type);

        /** Emit an output of this component. */
        void emitOutputImpl(const std::string& name, const Type& type,
                            const boost::any& value);
        
        /**
         * Opaque pointer to this object's internal implementation details.
         * Provides information hiding, improves binary compatibility, and
         * reduces compile times.
         *
         * @sa http://en.wikipedia.org/wiki/Opaque_pointer
         */
        Impl::ComponentImpl* dm_impl;

    }; // class Component
        
} } // namespace KrellInstitute::CBTF

/**
 * Macro definition that generates a statically initialized C++ structure
 * registering the factory function for the specified component type. The
 * factory function is assumed to be named "factoryFunction" and to be a
 * static method of the component type (class).
 *
 * @param type    Type (class name) of the component to be registered.
 */
#define KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(type)                   \
    namespace KrellInstitute { namespace CBTF { namespace Impl {               \
        template <typename T> struct __attribute__ ((visibility ("hidden")))   \
        RegisterFactoryFunction;                                               \
        template <> struct __attribute__ ((visibility ("hidden")))             \
        RegisterFactoryFunction<type>                                          \
        {                                                                      \
            RegisterFactoryFunction()                                          \
            {                                                                  \
                KrellInstitute::CBTF::Component::registerFactoryFunction(      \
                    type::factoryFunction                                      \
                    );                                                         \
            }                                                                  \
            static RegisterFactoryFunction instance;                           \
        };                                                                     \
        RegisterFactoryFunction<type> RegisterFactoryFunction<type>::instance; \
    } } }
