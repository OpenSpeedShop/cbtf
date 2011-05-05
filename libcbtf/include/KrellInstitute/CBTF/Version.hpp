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

/** @file Declaration of the Version class. */

#pragma once

#include <boost/operators.hpp>
#include <string>

namespace KrellInstitute { namespace CBTF {

    namespace Impl {
        class VersionImpl;
    }

    /**
     * Class representing a software version. Used to identify the version
     * of a component being used or requested.
     *
     * @sa http://en.wikipedia.org/wiki/Software_version
     */
    class Version :
        private boost::totally_ordered<Version>
    {
        
    public:

        /**
         * Construct a version from major, minor, and maintenance numbers.
         *
         * @param major_number          Major number of this version.
         * @param minor_number          Minor number of this version.
         * @param maintenance_number    Maintenance number of this version.
         */
        Version(const unsigned int& major_number,
                const unsigned int& minor_number,
                const unsigned int& maintenance_number);

        /**
         * Construct a version from the string name of that version. I.e.
         * the provided string should be of the form "x.y.z".
         *
         * @param name    String name of this version.
         *
         * @throw std::invalid_argument    The specified string
         *                                 doesn't name a version.
         */
        Version(const std::string& name);
        
        /**
         * Construct a version from an existing version.
         *
         * @param other    Version to be copied.
         */
        Version(const Version& other);
        
        /** Destructor. */
        virtual ~Version();
        
        /**
         * Replace this version with a copy of another one.
         *
         * @param other    Version to be copied.
         * @return         Resulting (this) version.
         */
        Version& operator=(const Version& other);

        /**
         * Is this version less than another one?
         *
         * @param other    Version to be compared.
         * @return         Boolean "true" if this version is less than the
         *                 version to be compared, or "false" otherwise.
         */
        bool operator<(const Version& other) const;
        
        /**
         * Is this version equal to another one?
         *
         * @param other    Version to be compared.
         * @return         Boolean "true" if the versions are equal,
         *                 or "false" otherwise.
         */
        bool operator==(const Version& other) const;

        /**
         * Type conversion to a string.
         *
         * @return    String name of this version.
         */
        operator std::string() const;

        /**
         * Get this version's major number. This number, initially zero, is
         * incremented every time a change is made to the software's interface.
         *
         * @return    Major number of this version.
         *
         * @note    Adding, removing, or changing the name/type of a
         *          component's inputs/outputs are examples of changes
         *          that require its major version number be incremented.
         */
        unsigned int getMajorNumber() const;

        /**
         * Get this version's minor number. This number, initially zero, is
         * incremented every time a change is made to the software's semantics
         * without changing its interface.
         *
         * @return    Minor number of this version.
         *
         * @note    Changing a component's internal queueing behavior
         *          is an example of a change that requires its minor
         *          version number be incremented.
         */
        unsigned int getMinorNumber() const;

        /**
         * Get this version's maintenance number. This number, initially zero,
         * is incremented every time a change is made to the software without
         * changing its interface or semantics.
         *
         * @return    Maintenance number of this version.
         *
         * @note    Fixing an incorrect calculation is an example of a
         *          change that requires the maintenance version number
         *          be incremented.
         */
        unsigned int getMaintenanceNumber() const;
        
    private:

        /**
         * Opaque pointer to this object's internal implementation details.
         * Provides information hiding, improves binary compatibility, and
         * reduces compile times.
         *
         * @sa http://en.wikipedia.org/wiki/Opaque_pointer
         */
        Impl::VersionImpl* dm_impl;

    }; // class Version

    /**
     * Redirection to an output stream.
     *
     * @param stream     Destination output stream.
     * @param version    Version to be redirected.
     * @return           Destination output stream.
     */
    std::ostream& operator<<(std::ostream& stream, const Version& version);
                
} } // namespace KrellInstitute::CBTF
