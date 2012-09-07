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

#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
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

using namespace KrellInstitute::CBTF;



namespace std {

    extern std::ostream& operator<<(
        std::ostream& stream,
        const std::map<std::string, Type>::const_iterator& iterator
        );
    
    extern std::ostream& operator<<(
        std::ostream& stream,
        const std::set<Type>::const_iterator& iterator
        );

} // namespace std



/**
 * Unit test for XML-defined component networks.
 */
BOOST_AUTO_TEST_CASE(TestXML)
{
    // Test registration of component network types from XML
    BOOST_CHECK_THROW(registerXML("non_existent_file.xml"),
                      std::runtime_error);
    std::set<Type> available_types = Component::getAvailableTypes();
    BOOST_CHECK_EQUAL(available_types.find(Type("TestXML")),
                      available_types.end());
    BOOST_CHECK_NO_THROW(registerXML("test-xml.xml"));
    available_types = Component::getAvailableTypes();
    BOOST_CHECK_NE(available_types.find(Type("TestXML")),
                   available_types.end());

    // Test component network instantiation and metadata
    Component::Instance network;
    BOOST_CHECK_NO_THROW(network = Component::instantiate(Type("TestXML")));
    std::map<std::string, Type> inputs = network->getInputs();
    BOOST_CHECK_NE(inputs.find("in"), inputs.end());
    std::map<std::string, Type> outputs = network->getOutputs();
    BOOST_CHECK_NE(outputs.find("out"), outputs.end());
    
    // Test component network versioning
    BOOST_CHECK_EQUAL(network->getVersion(), Version(1, 2, 3));
    
    // Test component network intercommunication
    boost::shared_ptr<ValueSource<int> > input_value =
        ValueSource<int>::instantiate();
    boost::shared_ptr<ValueSink<int> > output_value = 
        ValueSink<int>::instantiate();
    Component::Instance input_value_component = 
        boost::reinterpret_pointer_cast<Component>(input_value);
    Component::Instance output_value_component = 
        boost::reinterpret_pointer_cast<Component>(output_value);
    Component::connect(input_value_component, "value", network, "in");
    Component::connect(network, "out", output_value_component, "value");
    *input_value = 10;
    int the_output_value = *output_value;
    BOOST_CHECK_EQUAL(the_output_value, 42);
}
