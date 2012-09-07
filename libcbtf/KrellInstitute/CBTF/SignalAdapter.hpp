////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Krell Institute. All Rights Reserved.
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

/** @file Declaration and definition of the SignalAdapter class. */

#pragma once

#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF {
    
    /**
     * Signal adapter for a component output. Provides the ability to have
     * the output of a component passed into one or more callback functions
     * through the use of the Boost Signals2 library. This class is itself
     * a component, and thus its input can be attached to outputs of other
     * components via the usual mechanism for connecting components.
     *
     * @tparam T    Type of the output being adapted.
     *
     * @note    Unlike most component types, this type's factory function
     *          is not registered with the Component class, and thus must
     *          be directly instantiated.
     *
     * @sa http://en.wikipedia.org/wiki/Adapter_pattern
     * @sa http://www.boost.org/doc/libs/release/doc/html/signals2.html
     */
    template <typename T>
    class SignalAdapter :
        public Component
    {
        
    public:

        /**
         * Factory function for this component type.
         *
         * @return    A new instance of this type.
         */
        static boost::shared_ptr<SignalAdapter> instantiate()
        {
            return boost::shared_ptr<SignalAdapter>(new SignalAdapter());
        }

        /**
         * Signal that is emitted every time a new value arrives on this
         * component's "value" input.
         */
        boost::signals2::signal<void (const T&)> Value;
                
    private:

        /** Default constructor. */
        SignalAdapter() :
            Component(Type(typeid(SignalAdapter)), Version(0, 0, 0))
        {
            declareInput<T>(
                "value", boost::bind(&SignalAdapter::valueHandler, this, _1)
                );
        }
                
        /** Handler for the "value" input. */
        void valueHandler(const T& value)
        {
            Value(value);
        }
        
    }; // class SignalAdapter<T>

} } // namespace KrellInstitute::CBTF
