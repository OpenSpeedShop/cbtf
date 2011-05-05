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

/** @file Unit tests for the CBTF XML library. */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE libcbtf-xml

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

    /**
     * Redirect a std::map<std:string, Type> const iterator to an output stream.
     * Defined in order to allow the Boost.Test macros to work properly.
     *
     * @param stream      Target output stream.
     * @param iterator    Const iterator to redirect.
     * @return            Target output stream.
     */
    std::ostream& operator<<(
        std::ostream& stream,
        const std::map<std::string, Type>::const_iterator& iterator
        )
    {
        stream << "std::map<std::string, Type>::const_iterator";
        return stream;
    }

    /**
     * Redirect a std::set<Type> const iterator to an output stream. Defined
     * in order to allow the Boost.Test macros to work properly.
     *
     * @param stream      Target output stream.
     * @param iterator    Const iterator to redirect.
     * @return            Target output stream.
     */
    std::ostream& operator<<(std::ostream& stream,
                             const std::set<Type>::const_iterator& iterator)
    {
        stream << "std::set<Type>::const_iterator";
        return stream;
    }
    
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
    BOOST_CHECK_EQUAL(available_types.find(Type("TestNetwork")),
                      available_types.end());
    BOOST_CHECK_NO_THROW(
        registerXML(boost::filesystem::path(BUILDDIR) / "network.xml")
        );
    available_types = Component::getAvailableTypes();
    BOOST_CHECK_NE(available_types.find(Type("TestNetwork")),
                   available_types.end());

    // Test component network instantiation and metadata
    Component::Instance network;
    BOOST_CHECK_NO_THROW(
        network = Component::instantiate(Type("TestNetwork"))
        );
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
    BOOST_CHECK_EQUAL(42, *output_value);
}
