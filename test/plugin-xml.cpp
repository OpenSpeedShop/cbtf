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

/** @file Plugin used by unit tests for the CBTF XML library. */

#include <boost/bind.hpp>
#include <typeinfo>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;



/**
 * Component that doubles an integer value.
 */
class __attribute__ ((visibility ("hidden"))) Doubler :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new Doubler())
            );
    }

private:

    /** Default constructor. */
    Doubler() :
        Component(Type(typeid(Doubler)), Version(0, 0, 1))
    {
        declareInput<int>(
            "in", boost::bind(&Doubler::inHandler, this, _1)
            );
        declareOutput<int>("out");
    }

    /** Handler for the "in" input.*/
    void inHandler(const int& in)
    {
        emitOutput<int>("out", in * 2);
    }
    
}; // class Doubler

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(Doubler)



/**
 * Component that increments an integer value.
 */
class __attribute__ ((visibility ("hidden"))) Incrementer :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new Incrementer())
            );
    }

private:

    /** Default constructor. */
    Incrementer() :
        Component(Type(typeid(Incrementer)), Version(0, 0, 1))
    {
        declareInput<int>(
            "in", boost::bind(&Incrementer::inHandler, this, _1)
            );
        declareOutput<int>("out");
    }

    /** Handler for the "in" input.*/
    void inHandler(const int& in)
    {
        emitOutput<int>("out", in + 1);
    }
    
}; // class Incrementer

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(Incrementer)
