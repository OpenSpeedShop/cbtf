////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010,2011 Krell Institute. All Rights Reserved.
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

/** @file Unit tests for the CBTF MRNet library. */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE libcbtf-mrnet

#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/ValueSink.hpp>
#include <KrellInstitute/CBTF/ValueSource.hpp>
#include <KrellInstitute/CBTF/XDR.hpp>
#include <KrellInstitute/CBTF/XML.hpp>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "messages.h"

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
 * Unit test for XML-defined distributed (via MRNet) component networks.
 */
BOOST_AUTO_TEST_CASE(TestMRNet)
{
    // Test registration of distributed component network types from XML
    std::set<Type> available_types = Component::getAvailableTypes();
    BOOST_CHECK_EQUAL(available_types.find(Type("TestNetwork")),
                      available_types.end());
    BOOST_CHECK_NO_THROW(
        registerXML(boost::filesystem::path(BUILDDIR) / "test.xml")
        );
    available_types = Component::getAvailableTypes();
    BOOST_CHECK_NE(available_types.find(Type("TestNetwork")),
                   available_types.end());
    
    // Test distributed component network instantiation and metadata
    Component::Instance network;
    BOOST_CHECK_NO_THROW(
        network = Component::instantiate(Type("TestNetwork"))
        );
    std::map<std::string, Type> inputs = network->getInputs();
    BOOST_CHECK_NE(inputs.find("in"), inputs.end());
    std::map<std::string, Type> outputs = network->getOutputs();
    BOOST_CHECK_NE(outputs.find("out"), outputs.end());

    // Test instantiation of the basic launcher component

    BOOST_REQUIRE_NO_THROW(Component::registerPlugin(
        boost::filesystem::path(TOPDIR) / "launchers" / "BasicMRNetLaunchers"
        ));
    Component::Instance launcher;
    BOOST_CHECK_NO_THROW(
        launcher = Component::instantiate(Type("SimpleMRNetLauncher"))
        );
    
    // Test distributed component network intercommunication

    boost::shared_ptr<ValueSource<boost::filesystem::path> > topology_value =
        ValueSource<boost::filesystem::path>::instantiate();
    boost::shared_ptr<ValueSource<int> > input_value = 
        ValueSource<int>::instantiate();
    boost::shared_ptr<ValueSink<int> > output_value = 
        ValueSink<int>::instantiate();

    Component::Instance topology_value_component = 
        boost::reinterpret_pointer_cast<Component>(topology_value);
    Component::Instance input_value_component = 
        boost::reinterpret_pointer_cast<Component>(input_value);
    Component::Instance output_value_component = 
        boost::reinterpret_pointer_cast<Component>(output_value);

    Component::connect(
        topology_value_component, "value", launcher, "TopologyFile"
        );
    Component::connect(launcher, "Network", network, "Network");
    Component::connect(input_value_component, "value", network, "in");
    Component::connect(network, "out", output_value_component, "value");

    *topology_value = boost::filesystem::path(BUILDDIR) / "test.topology";
    *input_value = 10;
    BOOST_CHECK_EQUAL(26, *output_value);
    *input_value = 13;
    BOOST_CHECK_EQUAL(32, *output_value);
}



KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(TestMessage)



/**
 * Unit test for XDR/MRNet conversion components.
 */
BOOST_AUTO_TEST_CASE(TestXDRConverters)
{
    typedef boost::shared_ptr<TestMessage> TestMessagePtr;

    boost::shared_ptr<ValueSource<TestMessagePtr> > input_value = 
        ValueSource<TestMessagePtr>::instantiate();
    Component::Instance input_value_component = 
        boost::reinterpret_pointer_cast<Component>(input_value);
    
    Component::Instance xdr_to_mrnet;
    BOOST_CHECK_NO_THROW(
        xdr_to_mrnet = Component::instantiate(Type(
            "KrellInstitute::CBTF::ConvertXDRToMRNet<TestMessage>"
            ))
        );
    
    Component::Instance mrnet_to_xdr;
    BOOST_CHECK_NO_THROW(
        mrnet_to_xdr = Component::instantiate(Type(
            "KrellInstitute::CBTF::ConvertMRNetToXDR<TestMessage>"
            ))
        );

    boost::shared_ptr<ValueSink<TestMessagePtr> > output_value = 
        ValueSink<TestMessagePtr>::instantiate();
    Component::Instance output_value_component = 
        boost::reinterpret_pointer_cast<Component>(output_value);

    Component::connect(input_value_component, "value", xdr_to_mrnet, "in");
    Component::connect(xdr_to_mrnet, "out", mrnet_to_xdr, "in");
    Component::connect(mrnet_to_xdr, "out", output_value_component, "value");

    TestMessagePtr input(new TestMessage());
    input->x = 42;
    *input_value = input;

    TestMessagePtr output = *output_value;
    BOOST_CHECK_EQUAL(42, output->x);
}
