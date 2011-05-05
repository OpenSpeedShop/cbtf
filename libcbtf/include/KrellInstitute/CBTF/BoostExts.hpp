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

/** @file Extensions to the standard Boost libraries. */

#pragma once

#include <boost/bind.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>

namespace boost {

    /**
     * Deleter used by the specialization of reinterpret_pointer_cast<> for
     * shared_ptr(). Doesn't do anything directly - the automatic destruction
     * of the bound second argument does all the work.
     */
    template <class T, class U>
    void _rpc_sp_deleter(T* /* Unused */, shared_ptr<U> /* Unused */)
    {
    }
    
    /**
     * Specialization of reinterpret_pointer_cast<> for shared_ptr<>. It
     * isn't clear why Boost doesn't already provided this since all the
     * other pointer cast functions have specializations for it. Copying
     * the deleter over directly isn't possible, so dirty tricks must be
     * employed to insure the original shared_ptr<>'s deleter is called.
     *
     * @sa http://www.boost.org/doc/libs/release/libs/smart_ptr/pointer_cast.html
     * @sa http://www.boost.org/doc/libs/release/libs/smart_ptr/shared_ptr.htm
     */
    template <class T, class U>
    shared_ptr<T> reinterpret_pointer_cast(const shared_ptr<U>& r)
    {
        return shared_ptr<T>(
            reinterpret_cast<T*>(r.get()), bind(&_rpc_sp_deleter<T, U>, _1, r)
            );
    }
    
} // namespace boost
