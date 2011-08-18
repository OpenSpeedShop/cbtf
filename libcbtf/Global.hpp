////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the KRELL_INSTITUTE_CBTF_IMPL_GLOBAL macro. */

#pragma once

#include <boost/preprocessor/comma.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

/**
 * Macro definition implementing a variation of the "construct on first use"
 * idiom. Each global is implemented as a namespace containing functions for
 * accessing a mutual exclusion lock for the global and its value. Both the
 * lock and value are guaranteed to be initialized upon (individual) first
 * use during static C++ initialization, and always by the end of static C++
 * initialization. This allows the global to be used without worrying about
 * when it will be initialized with respect to uses of it.
 *
 * @param name    Name of of the global's namespace.
 * @param type    Type of the global's value.
 *
 * @sa http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Construct_On_First_Use
 */
#define KRELL_INSTITUTE_CBTF_IMPL_GLOBAL(name, type)                \
    namespace name {                                                \
        typedef type Type;                                          \
        typedef boost::recursive_mutex::scoped_lock GuardType;      \
        boost::recursive_mutex& mutex()                             \
        {                                                           \
            static boost::recursive_mutex the_mutex;                \
            return the_mutex;                                       \
        }                                                           \
        Type& value()                                               \
        {                                                           \
            static Type the_value;                                  \
            return the_value;                                       \
        }                                                           \
        struct Initializer                                          \
        {                                                           \
            Initializer()                                           \
            {                                                       \
                boost::recursive_mutex::scoped_lock guard(mutex()); \
                value() = Type();                                   \
            }                                                       \
            static Initializer instance;                            \
        };                                                          \
        Initializer Initializer::instance;                          \
    }
