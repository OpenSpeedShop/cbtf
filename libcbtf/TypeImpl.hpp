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

/** @file Declaration of the TypeImpl class. */

#pragma once

#include <string>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Implementation details of the Type class. Anything that
     * would normally be a private member of Type is instead a
     * member of TypeImpl.
     */
    class TypeImpl
    {

    public:

        /** Construct a type from its name. */
        TypeImpl(const std::string& name);

        /** Construct a type from its C++ run-time type information. */
        TypeImpl(const std::type_info& info);
        
        /** Construct a type from an existing type. */
        TypeImpl(const TypeImpl& other);
        
        /** Destructor. */
        ~TypeImpl();
        
        /** Replace this type with a copy of another one. */
        TypeImpl& operator=(const TypeImpl& other);

        /** Is this type less than another one? */
        bool operator<(const TypeImpl& other) const;

        /** Is this type equal to another one? */
        bool operator==(const TypeImpl& other) const;
        
        /** Type conversion to a string. */
        operator std::string() const;
        
    private:

        /** Name of this type. */
        std::string dm_name;
        
    }; // class TypeImpl
        
} } } // namespace KrellInstitute::CBTF::Impl
