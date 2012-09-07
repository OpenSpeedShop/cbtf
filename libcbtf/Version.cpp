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

/** @file Definition of the Version class. */

#include <KrellInstitute/CBTF/Version.hpp>

#include "VersionImpl.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version::Version(const unsigned int& major_number,
                 const unsigned int& minor_number,
                 const unsigned int& maintenance_number) :
    dm_impl(new VersionImpl(major_number, minor_number, maintenance_number))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version::Version(const std::string& name) :
    dm_impl(new VersionImpl(name))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version::Version(const Version& other) :
    dm_impl(new VersionImpl(*other.dm_impl))
{
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version::~Version()
{
    delete dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version& Version::operator=(const Version& other)
{
    *dm_impl = *other.dm_impl;
    return *this;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
bool Version::operator<(const Version& other) const
{
    return *dm_impl < *other.dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
bool Version::operator==(const Version& other) const
{
    return *dm_impl == *other.dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
Version::operator std::string() const
{
    return *dm_impl;
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
unsigned int Version::getMajorNumber() const
{
    return dm_impl->getMajorNumber();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
unsigned int Version::getMinorNumber() const
{
    return dm_impl->getMinorNumber();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
unsigned int Version::getMaintenanceNumber() const
{
    return dm_impl->getMaintenanceNumber();
}



//------------------------------------------------------------------------------
// Let the implementation do the real work.
//------------------------------------------------------------------------------
std::ostream& KrellInstitute::CBTF::operator<<(std::ostream& stream,
                                               const Version& version)
{
    stream << static_cast<std::string>(version);
    return stream;
}
