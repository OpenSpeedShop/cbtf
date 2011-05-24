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

/** @file Declaration and definition of the InvokerFor functor. */

#pragma once

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <KrellInstitute/CBTF/Impl/Invoker.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Concrete implementation of the Invoker abstract base class for a
     * value of the template-specified type. Converts between a boost::any
     * and a typed value that can actually be passed into the handler.
     *
     * @tparam T    Type of the value being passed.
     */
    template <typename T>
    struct InvokerFor :
        public Invoker
    {
        /**
         * Construct an invoker for the specified handler function.
         *
         * @param handler    Handler being invoked.
         */
        InvokerFor(const boost::function<void (const T&)>& handler) :
            Invoker(),
            dm_handler(handler)
        {
        }
        
        /**
         * Invoke the handler with the specified value.
         *
         * @param value    Value to pass to the handler.
         *
         * @note    It would be better to use boost::any_cast below instead of
         *          boost::unsafe_any_cast. Unfortunately boost::any_cast uses
         *          (as of Boost 1.40 anyway) direct equality comparisons of
         *          typeinfo objects. Doing so is not supported across shared
         *          library boundaries by GCC, and packaging components into
         *          shared libraries is a rather important feature... So until
         *          either the Boost or GCC folks fix this, the unsafe cast is
         *          going to be necessary.
         *
         * @sa http://gcc.gnu.org/faq.html#dso
         */
        virtual void operator()(const boost::any& value) const
        {
            // dm_handler(boost::any_cast<T>(value));
            dm_handler(*boost::unsafe_any_cast<T>(&value));
        }

        /** Handler being invoked. */
        const boost::function<void (const T&)> dm_handler;

    }; // struct InvokerFor<T>

} } } // namespace KrellInstitute::CBTF::Impl
