////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

/** @file Declaration and definition of the InputMediator class. */

#pragma once

#include <boost/any.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF { namespace Impl {
    
    /**
     * Mediator for a network's input. Provides the ability to forward the
     * value of that input to the input of another component. This class is
     * itself a component, and thus its output can be attached to inputs of
     * other components via the usual mechanism for connecting components.
     *
     * @note    Unlike most component types, this type has no factory
     *          function and thus is not registered with the Component
     *          class and must be directly instantiated.
     *
     * @note    This class is similar to the CBTF library's ValueSource class.
     *
     * @sa http://en.wikipedia.org/wiki/Mediator_pattern
     */
    class InputMediator :
        public Component
    {

    public:

        /**
         * Construct a new mediator for an input.
         *
         * @param type    Type of the input being mediated.
         */
        InputMediator(const Type& type) :
            Component(Type(typeid(InputMediator)), Version(0, 0, 0)),
            dm_type(type)
        {
            declareOutput("value", type);
        }
        
        /**
         * Handler for the input being mediated.
         *
         * @param value    New value for the input being mediated.
         */
        void handler(const boost::any& value)
        {
            emitOutput("value", dm_type, value);
        }

    private:
        
        /** Type of the input being mediated. */
        const Type dm_type;
        
    }; // class InputMediator

} } } // namespace KrellInstitute::CBTF::Impl
