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

/** @file Unit tests for the CBTF library. */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE libcbtf

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/ValueSink.hpp>
#include <KrellInstitute/CBTF/ValueSource.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>

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
    
    /**
     * Redirect a std::set<Version> const iterator to an output stream. Defined
     * in order to allow the Boost.Test macros to work properly.
     *
     * @param stream      Target output stream.
     * @param iterator    Const iterator to redirect.
     * @return            Target output stream.
     */
    std::ostream& operator<<(std::ostream& stream,
                             const std::set<Version>::const_iterator& iterator)
    {
        stream << "std::set<Version>::const_iterator";
        return stream;
    }
    
} // namespace std



/**
 * Unit test for the Type class.
 */
BOOST_AUTO_TEST_CASE(TestType)
{
    // Test construction of a type from its name
    Type type_of_MyClass("MyClass");

    // Test construction of a type from its C++ run-time type information
    int x, y;
    float z;
    Type type_of_x(typeid(x)), type_of_y(typeid(y)), type_of_z(typeid(z));

    // Test type conversion to a string
    BOOST_CHECK_EQUAL(static_cast<std::string>(type_of_MyClass), "MyClass");
    BOOST_CHECK_EQUAL(static_cast<std::string>(type_of_x), "int");

    // Test type comparison
    BOOST_CHECK_EQUAL(type_of_x, type_of_y);
    BOOST_CHECK_NE(type_of_x, type_of_z);

    // Test type copy construction and assignment
    Type another_type(type_of_x);
    BOOST_CHECK_EQUAL(another_type, type_of_x);
    another_type = type_of_z;
    BOOST_CHECK_NE(another_type, type_of_x);
    BOOST_CHECK_EQUAL(another_type, type_of_z);
}



/**
 * Unit test for the Version class.
 */
BOOST_AUTO_TEST_CASE(TestVersion)
{
    // Test construction from major, minor, and maintenance numbers
    Version v(0, 0, 0), w(0, 0, 1), x(0, 0, 1), y(0, 2, 0), z(4, 0, 0);

    // Test major, minor, and maintenance number accessors
    BOOST_CHECK_EQUAL(z.getMajorNumber(), 4);
    BOOST_CHECK_EQUAL(y.getMinorNumber(), 2);
    BOOST_CHECK_EQUAL(w.getMaintenanceNumber(), 1);

    // Test type conversion to a string
    BOOST_CHECK_EQUAL(static_cast<std::string>(v), "0.0.0");
    BOOST_CHECK_EQUAL(static_cast<std::string>(w), "0.0.1");
    BOOST_CHECK_EQUAL(static_cast<std::string>(x), "0.0.1");
    BOOST_CHECK_EQUAL(static_cast<std::string>(y), "0.2.0");
    BOOST_CHECK_EQUAL(static_cast<std::string>(z), "4.0.0");

    // Test version comparison
    BOOST_CHECK_EQUAL(w, x);
    BOOST_CHECK_NE(v, w);
    BOOST_CHECK_NE(v, y);
    BOOST_CHECK_NE(y, z);
    BOOST_CHECK_NE(x, y);
    BOOST_CHECK_NE(x, z);
    BOOST_CHECK_NE(y, z);
    BOOST_CHECK( v < w );
    BOOST_CHECK( w <= x );
    BOOST_CHECK( x < y );
    BOOST_CHECK( z > y );
    BOOST_CHECK( y >= x );

    // Test version copy construction and assignment
    Version another_version(x);
    BOOST_CHECK_EQUAL(another_version, x);
    another_version = z;
    BOOST_CHECK_NE(another_version, x);
    BOOST_CHECK_EQUAL(another_version, z);

    // Test construction from the string name of a version
    Version u("4.2.10");
    BOOST_CHECK_EQUAL(u.getMajorNumber(), 4);
    BOOST_CHECK_EQUAL(u.getMinorNumber(), 2);
    BOOST_CHECK_EQUAL(u.getMaintenanceNumber(), 10);
    BOOST_CHECK_THROW(Version("4.2"), std::invalid_argument);
    BOOST_CHECK_THROW(Version("4.2,0"), std::invalid_argument);
    BOOST_CHECK_THROW(Version("A Random String"), std::invalid_argument);
}



/**
 * Component type used by the unit test for the Component class.
 */
class __attribute__ ((visibility ("hidden"))) TestComponentA :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new TestComponentA())
            );
    }

private:

    /** Default constructor. */
    TestComponentA() :
        Component(Type(typeid(TestComponentA)), Version(0, 0, 0))
    {
        declareInput<int>(
            "in", boost::bind(&TestComponentA::inHandler, this, _1)
            );
        declareOutput<int>("double");
        declareOutput<int>("triple");
        declareOutput<float>("float");
    }

    /** Handler for the "in" input. */
    void inHandler(const int& in)
    {
        emitOutput<int>("double", 2 * in);
        emitOutput<int>("triple", 3 * in);
        emitOutput<float>("float", static_cast<float>(in));
    }
    
}; // class TestComponentA

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(TestComponentA)



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
        Component(Type(typeid(TestComponentC)), Version(0, 0, 0))
    {
        declareInput<int>(
            "in", boost::bind(&TestComponentC::inHandler, this, _1)
            );
        declareOutput<int>("incremented");
    }

    /** Handler for the "in" input.*/
    void inHandler(const int& in)
    {
        emitOutput<int>("incremented", in - 1 /* Intentional Bug */ );
    }
    
}; // class TestComponentC

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(TestComponentC)



/**
 * Unit test for the Component class.
 */
BOOST_AUTO_TEST_CASE(TestComponent)
{
    // Test registration of directly-linked component types
    std::set<Type> available_types = Component::getAvailableTypes();
    BOOST_CHECK_EQUAL(available_types.size(), 2);
    BOOST_CHECK_NE(available_types.find(Type("TestComponentA")),
                   available_types.end());
    BOOST_CHECK_NE(available_types.find(Type("TestComponentC")),
                   available_types.end());

    // Test registration of component types from plugins
    BOOST_CHECK_THROW(Component::registerPlugin("ThisPluginDoesNotExist"),
                      std::runtime_error);
    BOOST_CHECK_EQUAL(available_types.find(Type("TestComponentB")),
                      available_types.end());
    BOOST_REQUIRE_NO_THROW(
        Component::registerPlugin(boost::filesystem::path(BUILDDIR) / "plugin")
        );
    available_types = Component::getAvailableTypes();
    BOOST_CHECK_EQUAL(available_types.size(), 3);
    BOOST_CHECK_NE(available_types.find(Type("TestComponentB")),
                   available_types.end());
    
    // Test component instantiation
    BOOST_CHECK_THROW(Component::instantiate(Type(typeid(long))),
                      std::runtime_error);
    Component::Instance instance_of_a;
    BOOST_CHECK_NO_THROW(
        instance_of_a = Component::instantiate(Type("TestComponentA"))
        );
    BOOST_REQUIRE(instance_of_a);
    Component::Instance instance_of_b;
    BOOST_CHECK_NO_THROW(
        instance_of_b = Component::instantiate(Type("TestComponentB"))
        );
    BOOST_REQUIRE(instance_of_b);
    
    // Test component instance metadata
    BOOST_CHECK(!instance_of_a->getBuildString().empty());
    std::map<std::string, Type> inputs = instance_of_a->getInputs();
    BOOST_CHECK_EQUAL(inputs.size(), 1);
    BOOST_REQUIRE_NE(inputs.find("in"), inputs.end());
    BOOST_CHECK_EQUAL(inputs.find("in")->second, Type("int"));
    std::map<std::string, Type> outputs = instance_of_a->getOutputs();
    BOOST_CHECK_EQUAL(outputs.size(), 3);
    BOOST_REQUIRE_NE(outputs.find("double"), outputs.end());
    BOOST_CHECK_EQUAL(outputs.find("double")->second, Type("int"));
    BOOST_REQUIRE_NE(outputs.find("triple"), outputs.end());
    BOOST_CHECK_EQUAL(outputs.find("triple")->second, Type("int"));
    BOOST_REQUIRE_NE(outputs.find("float"), outputs.end());
    BOOST_CHECK_EQUAL(outputs.find("float")->second, Type(typeid(float)));
    
    // Test component versioning
    std::set<Version> available_versions =
        Component::getAvailableVersions(Type("TestComponentA"));
    BOOST_CHECK_EQUAL(available_versions.size(), 1);
    available_versions =
        Component::getAvailableVersions(Type("TestComponentC"));
    BOOST_CHECK_EQUAL(available_versions.size(), 2);
    BOOST_CHECK_EQUAL(available_versions.find(Version(0, 0, 2)),
                      available_versions.end());
    BOOST_CHECK_NE(available_versions.find(Version(0, 0, 1)),
                   available_versions.end());
    Component::Instance instance_of_c;
    BOOST_CHECK_THROW(instance_of_c = Component::instantiate(
                          Type("TestComponentC"), Version(0, 0, 2)
                          ),
                      std::runtime_error);
    BOOST_CHECK_NO_THROW(instance_of_c = Component::instantiate(
                             Type("TestComponentC"), Version(0, 0, 0)
                             ));
    BOOST_CHECK_NE(instance_of_c->getVersion(), Version(0, 0, 1));
    BOOST_CHECK_EQUAL(instance_of_c->getVersion(), Version(0, 0, 0));
    BOOST_CHECK_NO_THROW(
        instance_of_c = Component::instantiate(Type("TestComponentC"))
        );
    BOOST_CHECK_NE(instance_of_c->getVersion(), Version(0, 0, 0));
    BOOST_CHECK_EQUAL(instance_of_c->getVersion(), Version(0, 0, 1));
    
    // Test component connection
    BOOST_CHECK_THROW(Component::connect(
                          instance_of_a, "twice", instance_of_a, "in"
                          ),
                      std::runtime_error);
    BOOST_CHECK_THROW(Component::connect(
                          instance_of_a, "double", instance_of_b, "x"
                          ),
                      std::runtime_error);
    BOOST_CHECK_THROW(Component::connect(
                          instance_of_a, "float", instance_of_b, "in"
                          ),
                      std::runtime_error);
    Component::connect(instance_of_a, "double", instance_of_b, "in");
    BOOST_CHECK_THROW(Component::connect(
                          instance_of_a, "double", instance_of_b, "in"
                          ),
                      std::runtime_error);

    // Test component intercommunication
    boost::shared_ptr<ValueSource<int> > input_value = 
        ValueSource<int>::instantiate();
    boost::shared_ptr<ValueSink<int> > output_value = 
        ValueSink<int>::instantiate();
    Component::Instance input_value_component = 
        boost::reinterpret_pointer_cast<Component>(input_value);
    Component::Instance output_value_component = 
        boost::reinterpret_pointer_cast<Component>(output_value);
    Component::connect(input_value_component, "value", instance_of_a, "in");
    Component::connect(instance_of_b, "half", output_value_component, "value");
    *input_value = 10;
    int first_output_value = *output_value;
    BOOST_CHECK_EQUAL(first_output_value, 10);
    *input_value = 42;
    int second_output_value = *output_value;
    BOOST_CHECK_EQUAL(second_output_value, 42);
}
