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

/** @file Declaration and definition of the InvokerForAny functor. */

#pragma once

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <KrellInstitute/CBTF/Impl/Invoker.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Concrete implementation of the Invoker abstract base class for a value
     * of any type. Simply passes the value straight through to the handler.
     */
    struct InvokerForAny :
        public Invoker
    {
        /**
         * Construct an invoker for the specified handler function.
         *
         * @param handler    Handler being invoked.
         */
        InvokerForAny(
            const boost::function<void (const boost::any&)>& handler
            ) :
            Invoker(),
            dm_handler(handler)
        {
        }
        
        /**
         * Invoke the handler with the specified value.
         *
         * @param value    Value to pass to the handler.
         */
        virtual void operator()(const boost::any& value) const
        {
            dm_handler(value);
        }

        /** Handler being invoked. */
        const boost::function<void (const boost::any&)> dm_handler;

    }; // struct InvokerForAny

} } } // namespace KrellInstitute::CBTF::Impl
