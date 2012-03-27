////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
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

/** @file Declaration and definition of the AtomicCounter class. */

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Counter that is atomically incremented every time its value is accessed.
     *
     * @tparam T    Type of the counter's value.
     */
    template <typename T>
    class AtomicCounter :
        private boost::noncopyable
    {
        
    public:
        
        /**
         * Construct a new counter with an optional initial value.
         *
         * @param initial    Initial value of this counter. Has a default
         *                   value which is the default constructed value
         *                   of the counter's value type.
         */
        AtomicCounter(const T& initial = T()) :
            dm_mutex(),
            dm_value(initial)
        {
        }
        
        /**
         * Get the current value of this counter. 
         *
         * @return    Current value of this counter.
         */
        operator T()
        {
            boost::mutex::scoped_lock guard_this(dm_mutex);
            return dm_value++;
        }
        
    private:

        /** Mutual exclusion lock for this counter. */
        boost::mutex dm_mutex;
        
        /** Current value of this counter. */
        T dm_value;
        
    }; // class AtomicCounter

} } } // namespace KrellInstitute::CBTF::Impl
