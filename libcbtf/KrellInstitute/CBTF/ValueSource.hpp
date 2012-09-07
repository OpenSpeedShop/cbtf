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

/** @file Declaration and definition of the ValueSource class. */

#pragma once

#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF {
    
    /**
     * Source for a value of template-specified type. Provides the ability
     * to directly set (via assignment) the input of a component. This class
     * is itself a component, and thus its output can be attached to inputs
     * of other components via the usual mechanism for connecting components.
     *
     * @tparam T    Type of the value being sourced.
     *
     * @note    Unlike most component types, this type's factory function
     *          is not registered with the Component class, and thus must
     *          be directly instantiated.
     */
    template <typename T>
    class ValueSource :
        public Component
    {
        
    public:

        /**
         * Factory function for this component type.
         *
         * @return    A new instance of this type.
         */
        static boost::shared_ptr<ValueSource> instantiate()
        {
            return boost::shared_ptr<ValueSource>(new ValueSource());
        }
        
        /**
         * Set the value of this source.
         *
         * @param value    New value of this source.
         * @return         Resulting (this) component.
         */
        ValueSource& operator=(const T& value)
        {
            dm_value = value;
            emitOutput<T>("value", dm_value);
            return *this;
        }
        
        /**
         * Get the current value of this source.
         *
         * @return    Current value of this source.
         */
        operator T() const
        {
            return dm_value;
        }

    private:

        /** Default constructor. */
        ValueSource() :
            Component(Type(typeid(ValueSource)), Version(0, 0, 0)),
            dm_value()
        {
            declareOutput<T>("value");
        }
        
        /** Current value of this source. */
        T dm_value;
       
    }; // class ValueSource<T>
        
} } // namespace KrellInstitute::CBTF
