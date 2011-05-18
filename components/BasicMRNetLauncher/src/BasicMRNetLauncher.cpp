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

/** @file Plugin providing the basic MRNet launcher. */

#include <boost/bind.hpp>
#include <typeinfo>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;



/**
 * ...
 */
class __attribute__ ((visibility ("hidden"))) BasicMRNetLauncher :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new BasicMRNetLauncher())
            );
    }

private:

    /** Default constructor. */
    BasicMRNetLauncher() :
        Component(Type(typeid(BasicMRNetLauncher)), Version(0, 0, 0))
    {
        // TODO: Declare inputs & outputs
    }

}; // class BasicMRNetLauncher

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(BasicMRNetLauncher)
