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

/** @file Declaration of the Type class. */

#pragma once

#include <boost/operators.hpp>
#include <iostream>
#include <string>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF {

    namespace Impl {
        class TypeImpl;
    }

    /**
     * Class representing a data type. Used to encapsulate C++ run-time
     * type information, support dynamically defined types, and provide
     * rudimentary reflection capabilities.
     *
     * @sa http://en.wikipedia.org/wiki/Run-time_type_information
     * @sa http://en.wikipedia.org/wiki/Reflection_(computer_science)     
     */
    class Type :
        private boost::totally_ordered<Type>
    {
        
    public:
        
        /**
         * Construct a type from its name.
         *
         * @param name    Name of this type.
         */
        Type(const std::string& name);
        
        /**
         * Construct a type from its C++ run-time type information.
         *
         * @param info    C++ run-time type information for this type.
         */
        Type(const std::type_info& info);

        /**
         * Construct a type from an existing type.
         *
         * @param other    Type to be copied.
         */
        Type(const Type& other);
        
        /** Destructor. */
        virtual ~Type();
        
        /**
         * Replace this type with a copy of another one.
         *
         * @param other    Type to be copied.
         * @return         Resulting (this) type.
         */
        Type& operator=(const Type& other);

        /**
         * Is this type less than another one?
         *
         * @param other    Type to be compared.
         * @return         Boolean "true" if this type is less than the
         *                 type to be compared, or "false" otherwise.
         */
        bool operator<(const Type& other) const;

        /**
         * Is this type equal to another one?
         *
         * @param other    Type to be compared.
         * @return         Boolean "true" if the types are equal,
         *                 or "false" otherwise.
         */
        bool operator==(const Type& other) const;

        /**
         * Type conversion to a string.
         *
         * @return    Name of this type.
         */
        operator std::string() const;

    private:

        /**
         * Opaque pointer to this object's internal implementation details.
         * Provides information hiding, improves binary compatibility, and
         * reduces compile times.
         *
         * @sa http://en.wikipedia.org/wiki/Opaque_pointer
         */
        Impl::TypeImpl* dm_impl;

    }; // class Type

    /**
     * Redirection to an output stream.
     *
     * @param stream    Destination output stream.
     * @param type      Type to be redirected.
     * @return          Destination output stream.
     */
    std::ostream& operator<<(std::ostream& stream, const Type& type);
        
} } // namespace KrellInstitute::CBTF
