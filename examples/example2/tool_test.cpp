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

/** @file Unit tests for the CBTF XML library. */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE libcbtf-xml

#include <boost/shared_ptr.hpp>
//#include <boost/test/unit_test.hpp>
#include <iostream>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/ValueSink.hpp>
#include <KrellInstitute/CBTF/ValueSource.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <KrellInstitute/CBTF/XML.hpp>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <stdio.h>

using namespace KrellInstitute::CBTF;

/**
 * Unit test for the MonolithicTool class.
 */
main()
{
    registerXML("tool.xml");

    Component::Instance network;
    network = Component::instantiate(Type("ExampleNetwork"));

    // Test component network intercommunication
    boost::shared_ptr<ValueSource<int> > input_value = ValueSource<int>::instantiate();
    boost::shared_ptr<ValueSink<int> > output_value = ValueSink<int>::instantiate();
    Component::Instance input_value_component = boost::reinterpret_pointer_cast<Component>(input_value);
    Component::Instance output_value_component = boost::reinterpret_pointer_cast<Component>(output_value);
    Component::connect(input_value_component, "value", network, "in");
    Component::connect(network, "out", output_value_component, "value");
    *input_value = 10;
    if (*output_value == 8) {
       printf("Example Passes\n");
    } else {
       printf("Example Fails\n");
    }
}

