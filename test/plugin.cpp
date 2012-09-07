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

/** @file Plugin used by unit tests for the CBTF library. */

#include <boost/bind.hpp>
#include <typeinfo>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;



/**
 * Component type used by the unit test for the Component class.
 */
class __attribute__ ((visibility ("hidden"))) TestComponentB :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new TestComponentB())
            );
    }

private:

    /** Default constructor. */
    TestComponentB() :
        Component(Type(typeid(TestComponentB)), Version(0, 0, 1))
    {
        declareInput<int>(
            "in", boost::bind(&TestComponentB::inHandler, this, _1)
            );
        declareOutput<int>("half");
    }

    /** Handler for the "in" input.*/
    void inHandler(const int& in)
    {
        emitOutput<int>("half", in / 2);
    }
    
}; // class TestComponentB

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(TestComponentB)



/**
 * Component type used by the unit test for the Component class.
 */
class __attribute__ ((visibility ("hidden"))) TestComponentC :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new TestComponentC())
            );
    }

private:

    /** Default constructor. */
    TestComponentC() :
        Component(Type(typeid(TestComponentC)), Version(0, 0, 1))
    {
        declareInput<int>(
            "in", boost::bind(&TestComponentC::inHandler, this, _1)
            );
        declareOutput<int>("incremented");
    }

    /** Handler for the "in" input.*/
    void inHandler(const int& in)
    {
        emitOutput<int>("incremented", in + 1 /* Fix for Bug */ );
    }
    
}; // class TestComponentC

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(TestComponentC)
