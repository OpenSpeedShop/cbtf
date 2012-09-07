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

/** @file Declaration and definition of the OutputMediator class. */

#pragma once

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF { namespace Impl {
    
    /**
     * Mediator for a network's output. Provides the ability to forward the
     * value of that output from the output of another component. This class
     * is itself a component, and thus its input can be attached to outputs
     * of other components via the usual mechanism for connecting components.
     *
     * @note    Unlike most component types, this type has no factory
     *          function and thus is not registered with the Component
     *          class and must be directly instantiated.
     *
     * @note    This class is similar to the CBTF library's ValueSink class.
     *
     * @sa http://en.wikipedia.org/wiki/Mediator_pattern
     */
    class OutputMediator :
        public Component
    {

    public:

        /**
         * Construct a new mediator for an output.
         *
         * @param type       Type of the output being mediated.
         * @param handler    Handler for the output being mediated.
         */
        OutputMediator(
            const Type& type,
            const boost::function<void (const boost::any&)>& handler
            ) :
            Component(Type(typeid(OutputMediator)), Version(0, 0, 0)),
            dm_handler(handler)
        {
            declareInput("value", type, handler);
        }
        
    private:

        /** Handler for the output being mediated. */
        const boost::function<void (const boost::any&)> dm_handler;
        
    }; // class OutputMediator

} } } // namespace KrellInstitute::CBTF::Impl
