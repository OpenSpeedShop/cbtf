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

/** @file Declaration of the VersionImpl class. */

#pragma once

#include <string>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Implementation details of the Version class. Anything that
     * would normally be a private member of Version is instead a
     * member of VersionImpl.
     */
    class VersionImpl
    {

    public:

        /** Construct a version from major, minor, and maintenance numbers. */
        VersionImpl(const unsigned int& major_number,
                    const unsigned int& minor_number,
                    const unsigned int& maintenance_number);

        /** Construct a version from the string name of that version. */
        VersionImpl(const std::string& name);
        
        /** Construct a version from an existing version. */
        VersionImpl(const VersionImpl& other);
        
        /** Destructor. */
        ~VersionImpl();
        
        /** Replace this version with a copy of another one. */
        VersionImpl& operator=(const VersionImpl& other);

        /** Is this version less than another one? */
        bool operator<(const VersionImpl& other) const;
        
        /** Is this version equal to another one? */
        bool operator==(const VersionImpl& other) const;

        /** Type conversion to a string. */
        operator std::string() const;

        /** Get this version's major mumber. */
        unsigned int getMajorNumber() const;

        /** Get this version's minor mumber. */
        unsigned int getMinorNumber() const;

        /** Get this version's maintenance mumber. */
        unsigned int getMaintenanceNumber() const;
        
    private:

        /** Major number of this version. */
        unsigned int dm_major_number;

        /** Minor number of this version. */
        unsigned int dm_minor_number;

        /** Maintenance number of this version. */
        unsigned int dm_maintenance_number;
        
    }; // class VersionImpl
        
} } } // namespace KrellInstitute::CBTF::Impl
