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

/** @file Definition of the TypeImpl class. */

#include <cstdlib>
#include <cxxabi.h>

#include "TypeImpl.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TypeImpl::TypeImpl(const std::string& name) :
    dm_name(name)
{
}



//------------------------------------------------------------------------------
// Demangle the type's name from the provided C++ run-time type information.
//------------------------------------------------------------------------------
TypeImpl::TypeImpl(const std::type_info& info) :
    dm_name()
{
    int status = 0;
    char* raw = abi::__cxa_demangle(info.name(), NULL, NULL, &status);
    dm_name = (status == 0) ? raw : info.name();
    free(raw);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TypeImpl::TypeImpl(const TypeImpl& other) :
    dm_name(other.dm_name)
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TypeImpl::~TypeImpl()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TypeImpl& TypeImpl::operator=(const TypeImpl& other)
{
    if (this != &other)
    {
        dm_name = other.dm_name;
    }
    return *this;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool TypeImpl::operator<(const TypeImpl& other) const
{
    return dm_name < other.dm_name;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool TypeImpl::operator==(const TypeImpl& other) const
{
    return dm_name == other.dm_name;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TypeImpl::operator std::string() const
{
    return dm_name;
}
