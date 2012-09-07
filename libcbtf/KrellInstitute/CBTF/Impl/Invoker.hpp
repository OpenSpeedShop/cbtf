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

/** @file Declaration and definition of the Invoker functor. */

#pragma once

#include <boost/any.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Abstract base class (technically a structure) for a functor that
     * invokes a handler. Allows the templated implementation details of
     * invocation to be hidden behind a non-templated interface.
     */
    struct Invoker
    {
        /**
         * Invoke the handler with the specified value.
         *
         * @param value    Value to pass to the handler.
         */
        virtual void operator()(const boost::any& value) const = 0;
    };

} } } // namespace KrellInstitute::CBTF::Impl
