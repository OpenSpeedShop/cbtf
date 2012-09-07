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

/** @file Declaration and definition of the ValueSink class. */

#pragma once

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <deque>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF {
    
    /**
     * Sink for a value of template-specified type. Provides the ability to
     * directly get (via assignment) the output of a component. This class
     * is itself a component, and thus its input can be attached to outputs
     * of other components via the usual mechanism for connecting components.
     *
     * @tparam T    Type of the value being sunk.
     *
     * @note    Unlike most component types, this type's factory function
     *          is not registered with the Component class, and thus must
     *          be directly instantiated.
     */
    template <typename T>
    class ValueSink :
        public Component
    {
        
    public:

        /**
         * Factory function for this component type.
         *
         * @return    A new instance of this type.
         */
        static boost::shared_ptr<ValueSink> instantiate()
        {
            return boost::shared_ptr<ValueSink>(new ValueSink());
        }
                
        /**
         * Get the current value of this sink.
         *
         * @return    Current value of this sink.
         */
        operator T()
        {
            boost::unique_lock<boost::mutex> guard_this(dm_mutex);
            while (dm_values.empty())
            {
                dm_cv.wait(guard_this);
            }
            T value = dm_values.front();
            dm_values.pop_front();
            return value;
        }
        
    private:

        /** Default constructor. */
        ValueSink() :
            Component(Type(typeid(ValueSink)), Version(1, 1, 0)),
            dm_values(),
            dm_mutex(),
            dm_cv()
        {
            declareInput<T>(
                "value", boost::bind(&ValueSink::valueHandler, this, _1)
                );
        }
                
        /** Handler for the "value" input. */
        void valueHandler(const T& value)
        {
            boost::unique_lock<boost::mutex> guard_this(dm_mutex);
            dm_values.push_back(value);
            dm_cv.notify_one();
        }
        
        /** Current values of this sink. */
        std::deque<T> dm_values;

        /** Mutual exclusion lock for this sink. */
        boost::mutex dm_mutex;

        /** Condition variable for this sink. */
        boost::condition_variable dm_cv;
                
    }; // class ValueSink<T>

} } // namespace KrellInstitute::CBTF
