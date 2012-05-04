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

/** @file Definition of the VersionImpl class. */

#include <boost/format.hpp>
#include <boost/spirit/home/classic.hpp>
#include <stdexcept>

#include "Raise.hpp"
#include "VersionImpl.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VersionImpl::VersionImpl(const unsigned int& major_number,
                         const unsigned int& minor_number,
                         const unsigned int& maintenance_number) :
    dm_major_number(major_number),
    dm_minor_number(minor_number),
    dm_maintenance_number(maintenance_number)
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VersionImpl::VersionImpl(const std::string& name) :
    dm_major_number(0),
    dm_minor_number(0),
    dm_maintenance_number(0)
{
    using namespace boost::spirit::classic;

    bool is_full_match = parse(
        name.c_str(),
        uint_p[assign_a(dm_major_number)] >> ch_p('.') >> 
        uint_p[assign_a(dm_minor_number)] >> ch_p('.') >> 
        uint_p[assign_a(dm_maintenance_number)],
        space_p
        ).full;
    
    if (!is_full_match)
    {
        raise<std::invalid_argument>(
            "The specified string (%1%) doesn't name a version.", name
            );
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VersionImpl::VersionImpl(const VersionImpl& other) :
    dm_major_number(other.dm_major_number),
    dm_minor_number(other.dm_minor_number),
    dm_maintenance_number(other.dm_maintenance_number)
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VersionImpl::~VersionImpl()
{
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VersionImpl& VersionImpl::operator=(const VersionImpl& other)
{
    if (this != &other)
    {
        dm_major_number = other.dm_major_number;
        dm_minor_number = other.dm_minor_number;
        dm_maintenance_number = other.dm_maintenance_number;
    }
    return *this;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool VersionImpl::operator<(const VersionImpl& other) const
{
    if (dm_major_number < other.dm_major_number)
    {
        return true;
    }
    else if (dm_major_number > other.dm_major_number)
    {
        return false;
    }
    if (dm_minor_number < other.dm_minor_number)
    {
        return true;
    }
    else if (dm_minor_number > other.dm_minor_number)
    {
        return false;
    }
    return dm_maintenance_number < other.dm_maintenance_number;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool VersionImpl::operator==(const VersionImpl& other) const
{
    return (dm_major_number == other.dm_major_number) &&
        (dm_minor_number == other.dm_minor_number) &&
        (dm_maintenance_number == other.dm_maintenance_number);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
VersionImpl::operator std::string() const
{
    return boost::str(boost::format("%1%.%2%.%3%") % dm_major_number % 
                      dm_minor_number % dm_maintenance_number);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
unsigned int VersionImpl::getMajorNumber() const
{
    return dm_major_number;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
unsigned int VersionImpl::getMinorNumber() const
{
    return dm_minor_number;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
unsigned int VersionImpl::getMaintenanceNumber() const
{
    return dm_maintenance_number;
}
