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

/** @file Definition of the Type class. */

#include <KrellInstitute/CBTF/Type.hpp>

#include "TypeImpl.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type::Type(const std::string& name) :
    dm_impl(new TypeImpl(name))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type::Type(const std::type_info& info) :
    dm_impl(new TypeImpl(info))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type::Type(const Type& other) :
    dm_impl(new TypeImpl(*other.dm_impl))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type::~Type()
{
    delete dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type& Type::operator=(const Type& other)
{
    *dm_impl = *other.dm_impl;
    return *this;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
bool Type::operator<(const Type& other) const
{
    return *dm_impl < *other.dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
bool Type::operator==(const Type& other) const
{
    return *dm_impl == *other.dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Type::operator std::string() const
{
    return *dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::ostream& KrellInstitute::CBTF::operator<<(std::ostream& stream,
                                               const Type& type)
{
    stream << static_cast<std::string>(type);
    return stream;
}
